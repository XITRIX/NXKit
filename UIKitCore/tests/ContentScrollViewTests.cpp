#include <UIViewController.h>
#include <UIScrollView.h>
#include "ViewControllerContentScrollAccess.hpp"

#include <iostream>
#include <stdexcept>

using namespace NXKit;
using NXKit::Internal::ViewControllerContentScrollAccess;

namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
std::shared_ptr<UIScrollView> resolve(UIViewController& controller,
                                    NSDirectionalRectEdge edge = NSDirectionalRectEdge::top) {
    return ViewControllerContentScrollAccess::resolvedContentScrollView(controller, edge);
}
class SuppressingController final : public UIViewController {
public:
    std::shared_ptr<UIScrollView> contentScrollViewForEdge(NSDirectionalRectEdge edge) override {
        if (edge == NSDirectionalRectEdge::top) return nullptr;
        return UIViewController::contentScrollViewForEdge(edge);
    }
};
class SelectingController final : public UIViewController {
public:
    std::weak_ptr<UIScrollView> selected;
    std::shared_ptr<UIScrollView> contentScrollViewForEdge(NSDirectionalRectEdge edge) override {
        if (edge == NSDirectionalRectEdge::top) return selected.lock();
        return UIViewController::contentScrollViewForEdge(edge);
    }
};

void discovery() {
    auto controller = new_shared<UIViewController>();
    expect(!controller->contentScrollViewForEdge(NSDirectionalRectEdge::top),
           "public getter reports explicit selection only");
    expect(!resolve(*controller) && !controller->viewIsLoaded(),
           "observation does not load an unloaded controller");
    auto root = new_shared<UIView>();
    auto wrapper = new_shared<UIView>();
    auto inner = new_shared<UIView>();
    auto first = new_shared<UIScrollView>();
    root->addSubview(wrapper);
    wrapper->addSubview(inner);
    inner->addSubview(first);
    auto later = new_shared<UIScrollView>();
    root->addSubview(later);
    controller->setView(root);
    for (auto edge : {NSDirectionalRectEdge::top, NSDirectionalRectEdge::leading,
                      NSDirectionalRectEdge::bottom, NSDirectionalRectEdge::trailing}) {
        expect(resolve(*controller, edge) == first, "automatic discovery follows the first child chain");
        expect(!controller->contentScrollViewForEdge(edge), "discovery does not install an explicit selection");
    }
    first->removeFromSuperview();
    expect(!resolve(*controller), "an empty first branch stops discovery instead of searching siblings");
    wrapper->removeFromSuperview();
    expect(resolve(*controller) == later, "removing the first branch immediately discovers the new first child");
    controller->setView(first);
    expect(resolve(*controller) == first, "a scroll view used as controller root is discovered directly");
    controller->setView(new_shared<UIView>());
    expect(!resolve(*controller), "replacing the root does not retain a stale automatic selection");
}

void parentOnlySubviewRemoval() {
    auto controller = new_shared<UIViewController>();
    auto root = new_shared<UIView>();
    root->addSubview(new_shared<UIView>());
    auto scroll = new_shared<UIScrollView>();
    root->addSubview(scroll);
    controller->setView(root);
    expect(!resolve(*controller), "a first leaf obstructs automatic content-scroll discovery");
    const std::weak_ptr<UIView> removed = root->subviews().front();
    root->subviews().front()->removeFromSuperview();
    expect(removed.expired() && root->subviews().size() == 1 && resolve(*controller) == scroll,
           "a parent-only-owned first subview stays alive through detachment before releasing and rediscovering");
    root->setMask(new_shared<UIView>(NXRect {0,0,20,20}));
    const std::weak_ptr<UIView> removedMask = root->mask();
    root->mask()->removeFromSuperview();
    expect(removedMask.expired() && !root->mask() && !root->layer()->mask(),
           "a parent-only-owned mask stays alive through detachment before releasing");
}

void selections() {
    auto controller = new_shared<UIViewController>();
    auto automatic = new_shared<UIScrollView>();
    controller->setView(automatic);
    auto top = new_shared<UIScrollView>();
    auto bottom = new_shared<UIScrollView>();
    controller->setContentScrollView(top, NSDirectionalRectEdge::top | NSDirectionalRectEdge::leading);
    controller->setContentScrollView(bottom, NSDirectionalRectEdge::bottom);
    expect(resolve(*controller) == top && resolve(*controller, NSDirectionalRectEdge::leading) == top,
           "aggregate setters update each requested edge independently");
    expect(resolve(*controller, NSDirectionalRectEdge::bottom) == bottom
           && resolve(*controller, NSDirectionalRectEdge::trailing) == automatic,
           "unselected edges preserve automatic discovery");
    expect(controller->contentScrollViewForEdge(NSDirectionalRectEdge::top) == top,
           "public getter returns the explicit binding even outside the controller hierarchy");
    auto foreign = new_shared<UIView>();
    foreign->addSubview(top);
    expect(resolve(*controller) == top, "explicit selection survives custom placement and reparenting");
    controller->setContentScrollView(nullptr, NSDirectionalRectEdge::top);
    expect(resolve(*controller) == automatic && resolve(*controller, NSDirectionalRectEdge::leading) == top,
           "nil resets only the specified edge to automatic discovery");
    controller->setView(new_shared<UIView>());
    expect(resolve(*controller, NSDirectionalRectEdge::bottom) == bottom,
           "explicit selection survives controller root replacement");
    controller->setView(automatic);
    std::weak_ptr<UIScrollView> lifetime = bottom;
    bottom.reset();
    expect(lifetime.expired() && resolve(*controller, NSDirectionalRectEdge::bottom) == automatic,
           "bindings are weak and expiration restores discovery");
    controller->setContentScrollView(top);
    controller->setContentScrollView(nullptr, NSDirectionalRectEdge::none);
    expect(resolve(*controller, NSDirectionalRectEdge::trailing) == top, "none setter is a no-op");
    controller->setContentScrollView(nullptr);
    expect(resolve(*controller, NSDirectionalRectEdge::trailing) == automatic, "all reset clears every edge");
    auto unloaded = new_shared<UIViewController>();
    unloaded->setContentScrollView(top);
    expect(!unloaded->viewIsLoaded() && resolve(*unloaded) == top,
           "a manual binding can be installed and queried before view loading");
}

void overridesAndValidation() {
    auto controller = new_shared<SuppressingController>();
    auto automatic = new_shared<UIScrollView>();
    auto explicitSelection = new_shared<UIScrollView>();
    controller->setView(automatic);
    expect(!resolve(*controller) && resolve(*controller, NSDirectionalRectEdge::bottom) == automatic,
           "a direct nil override disables an edge while forwarding preserves automatic discovery");
    controller->setContentScrollView(explicitSelection, NSDirectionalRectEdge::top);
    expect(!resolve(*controller), "override can suppress an explicit binding");
    auto selecting = new_shared<SelectingController>();
    selecting->setView(automatic);
    selecting->selected = explicitSelection;
    expect(resolve(*selecting) == explicitSelection, "container resolution invokes subclass selection");
    explicitSelection.reset();
    // The suppressing controller's binding is weak and must not keep it alive.
    expect(!resolve(*selecting), "nil from a custom selector remains disabled rather than falling back");
    for (auto edge : {NSDirectionalRectEdge::none, NSDirectionalRectEdge::all,
                      NSDirectionalRectEdge::top | NSDirectionalRectEdge::bottom}) {
        bool getterRejected = false;
        try { (void)controller->UIViewController::contentScrollViewForEdge(edge); }
        catch (const std::invalid_argument&) { getterRejected = true; }
        expect(getterRejected, "base getter rejects an empty or aggregate edge");
    }
    bool setterRejected = false;
    try { controller->setContentScrollView(nullptr, static_cast<NSDirectionalRectEdge>(16)); }
    catch (const std::invalid_argument&) { setterRejected = true; }
    expect(setterRejected, "setter rejects unknown directional edge bits");
}

void edgeEffectMetadata() {
    auto scroll = new_shared<UIScrollView>(NXRect(0, 0, 40, 40));
    const std::array edges {scroll->topEdgeEffect(), scroll->leftEdgeEffect(),
                            scroll->bottomEdgeEffect(), scroll->rightEdgeEffect()};
    expect(scroll->topEdgeEffect() == edges[0] && scroll->leftEdgeEffect() == edges[1]
           && scroll->bottomEdgeEffect() == edges[2] && scroll->rightEdgeEffect() == edges[3],
           "each edge effect object has stable identity");
    for (std::size_t index = 0; index < edges.size(); ++index) {
        expect(edges[index] && edges[index]->style() == UIScrollEdgeEffectStyle::automatic
               && !edges[index]->isHidden(), "each edge defaults to automatic and visible");
        for (std::size_t other = index + 1; other < edges.size(); ++other)
            expect(edges[index] != edges[other], "each physical edge owns distinct metadata");
    }
    scroll->drawAndLayoutTreeIfNeeded();
    expect(!scroll->layer()->needsDisplay(), "drawing clears initial scroll layer invalidation");
    edges[0]->setStyle(UIScrollEdgeEffectStyle::soft);
    expect(edges[0]->style() == UIScrollEdgeEffectStyle::soft && scroll->layer()->needsDisplay(),
           "changing an edge style invalidates its owner");
    scroll->drawAndLayoutTreeIfNeeded();
    edges[0]->setStyle(UIScrollEdgeEffectStyle::soft);
    expect(!scroll->layer()->needsDisplay(), "setting the same edge style does not invalidate");
    edges[0]->setHidden(true);
    expect(edges[0]->isHidden() && scroll->layer()->needsDisplay(),
           "hiding an edge invalidates its owner");
    scroll->drawAndLayoutTreeIfNeeded();
    edges[0]->setHidden(true);
    expect(!scroll->layer()->needsDisplay(), "setting the same hidden state does not invalidate");
    bool rejected = false;
    try { edges[0]->setStyle(static_cast<UIScrollEdgeEffectStyle>(-1)); }
    catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected && edges[0]->style() == UIScrollEdgeEffectStyle::soft
           && !scroll->layer()->needsDisplay(), "invalid styles throw without mutating or invalidating");
    edges[1]->setStyle(UIScrollEdgeEffectStyle::hard);
    expect(edges[1]->style() == UIScrollEdgeEffectStyle::hard
           && edges[2]->style() == UIScrollEdgeEffectStyle::automatic && !edges[2]->isHidden(),
           "individual edge mutations are independent");
    std::weak_ptr<UIScrollView> owner = scroll;
    scroll.reset();
    expect(owner.expired(), "retaining edge metadata does not retain its scroll view");
    edges[0]->setHidden(false);
    edges[0]->setStyle(UIScrollEdgeEffectStyle::hard);
    expect(!edges[0]->isHidden() && edges[0]->style() == UIScrollEdgeEffectStyle::hard,
           "metadata remains safely mutable after its owner expires");
}
}

int main() {
    discovery();
    parentOnlySubviewRemoval();
    selections();
    overridesAndValidation();
    edgeEffectMetadata();
    if (failures) return 1;
    std::cout << "Content scroll view discovery, weak selections, overrides, and edge metadata passed\n";
    return 0;
}
