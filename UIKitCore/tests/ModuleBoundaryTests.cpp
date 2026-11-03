#include <UIKitCore.h>
#include <UINib.h>
#include <UIWindow.h>
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <iostream>
#include <stdexcept>

using namespace NXKit;
namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

// A custom kit needs only Core to participate in ancestor lookup and navigation.
class CustomNavigation final : public UIViewController, public UINavigationControllerInterface {
public:
    std::vector<std::shared_ptr<UIViewController>> stack;
    std::vector<std::shared_ptr<UIViewController>> viewControllers() const override { return stack; }
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool) override {
        for (const auto& child : children()) child->removeFromParent();
        stack = std::move(controllers);
        for (const auto& child : stack) { addChild(child); child->didMoveToParent(shared_from_this()); }
    }
    void pushViewController(const std::shared_ptr<UIViewController>& controller, bool) override {
        addChild(controller); stack.push_back(controller); controller->didMoveToParent(shared_from_this());
    }
    std::shared_ptr<UIViewController> popViewController(bool) override {
        if (stack.empty()) return nullptr;
        auto controller = stack.back(); stack.pop_back(); controller->removeFromParent(); return controller;
    }
    std::vector<std::shared_ptr<UIViewController>> popToViewController(
        const std::shared_ptr<UIViewController>& controller, bool animated) override {
        std::vector<std::shared_ptr<UIViewController>> removed;
        while (!stack.empty() && stack.back() != controller) removed.push_back(popViewController(animated));
        return removed;
    }
    std::vector<std::shared_ptr<UIViewController>> popToRootViewController(bool animated) override {
        return stack.empty() ? std::vector<std::shared_ptr<UIViewController>>{}
                             : popToViewController(stack.front(), animated);
    }
    std::shared_ptr<UIViewController> topViewController() const override {
        return stack.empty() ? nullptr : stack.back();
    }
    std::shared_ptr<UIViewController> visibleViewController() const override { return topViewController(); }
};

void discovery() {
    auto outer = new_shared<CustomNavigation>();
    auto inner = new_shared<CustomNavigation>();
    auto wrapper = new_shared<UIViewController>();
    auto leaf = new_shared<UIViewController>();
    expect(!leaf->navigationController() && !leaf->tabBarController(), "uncontained controller has no container");
    outer->pushViewController(inner, false);
    inner->pushViewController(wrapper, false);
    wrapper->addChild(leaf);
    expect(leaf->navigationController() == inner, "nearest custom navigation is found through ordinary wrappers");
    expect(inner->navigationController() == outer, "container lookup excludes self");
    auto next = new_shared<UIViewController>();
    leaf->navigationController()->pushViewController(next, false);
    expect(inner->topViewController() == next, "navigation dispatch uses the shared capability");
    leaf->removeFromParent();
    expect(!leaf->navigationController(), "removal immediately clears lookup");
    outer->addChild(leaf);
    expect(leaf->navigationController() == outer, "reparenting immediately updates lookup");
    std::weak_ptr<CustomNavigation> weakOuter = outer;
    outer.reset();
    expect(weakOuter.expired() && !leaf->navigationController(), "lookup does not retain ancestors");
}

void neutralControl() {
    auto control = new_shared<UIControl>();
    const auto transform = NXAffineTransform::translationBy(3, 7);
    control->setTransform(transform);
    control->setAlpha(.6f);
    control->layer()->setShadowOpacity(.25f);
    control->layer()->setShadowRadius(5);
    control->layer()->setZPosition(4);
    control->willGainFocus();
    control->willChangeFocusHighlight(true);
    control->setHighlighted(true);
    expect(control->isHighlighted(), "neutral control still tracks highlighted state");
    control->setHighlighted(false);
    control->willLoseFocus();
    expect(control->transform() == transform && control->alpha() == .6f,
           "core control feedback preserves caller geometry and opacity");
    expect(control->layer()->shadowOpacity() == .25f && control->layer()->shadowRadius() == 5
           && control->layer()->zPosition() == 4, "core focus does not install a kit's shadow or elevation");
}

void tabMetadata() {
    auto screen=new_shared<UIViewController>(); screen->setTitle("Default");
    const auto item=screen->tabBarItem();
    expect(item && item->title()=="Default" && item->isEnabled() && item->tag()==0
           && !item->image() && !item->selectedImage() && !item->badgeValue() && !item->badgeColor(),
           "Core supplies style-independent tab item defaults without loading a view");
    screen->setTitle("Changed"); expect(item->title()=="Changed","default tab title follows its controller");
    item->setTitle("Explicit"); screen->setTitle("Ignored");
    expect(item->title()=="Explicit","an explicit item title takes precedence");
    item->setEnabled(false); item->setTag(9); item->setBadgeValue("2");
    expect(!item->isEnabled() && item->tag()==9 && item->badgeValue()=="2",
           "tab metadata remains independently mutable in Core");
    screen->setTabBarItem(nullptr);
    expect(screen->tabBarItem()!=item && screen->tabBarItem()->title()=="Ignored" && !screen->viewIsLoaded(),
           "resetting tab metadata recreates the default item without loading content");
}

void genericBackdrop() {
    BackdropEffect effect(R"(
        uniform shader content;
        uniform float gain;
        half4 main(float2 xy) {
            half4 pixel = content.eval(xy);
            return half4(pixel.b * gain, pixel.g, pixel.r, pixel.a);
        }
    )");
    effect.setUniform("gain", 1.0f);
    auto layer = new_shared<CABackdropLayer>(effect);
    layer->setFrame({0, 0, 32, 32});
    auto copy = std::dynamic_pointer_cast<CABackdropLayer>(layer->copy());
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(32,32));
    surface->getCanvas()->clear(SK_ColorBLUE);
    layer->skiaRender(surface->getCanvas());
    SkBitmap pixels; pixels.allocN32Pixels(32,32); surface->readPixels(pixels,0,0);
    expect(pixels.getColor(16,16) == SK_ColorRED, "generic Core shader renders without UIKit");
    effect.setUniform("gain", .5f);
    layer->setEffect(effect);
    expect(copy->effect().uniform("gain")->front() == .5f, "presentation copy receives shader updates");
    bool rejected = false;
    try { layer->setEffect(BackdropEffect("invalid shader")); }
    catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected && layer->effect().uniform("gain")->front() == .5f,
           "invalid shader preserves the previously installed effect");
}
}
int main() {
    discovery();
    neutralControl();
    tabMetadata();
    genericBackdrop();
    if (!failures) std::cout << "Core module boundary tests passed\n";
    return failures ? 1 : 0;
}
