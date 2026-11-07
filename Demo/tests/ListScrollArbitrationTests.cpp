#include "../app/Screens/GlassDemoViewController/GlassDemoListView.hpp"
#include "../app/Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include "../app/Screens/GlassDemoViewController/SystemControlsDemoViewController.hpp"
#include "../../UIKit/tests/RasterTestContext.hpp"
#include <UIEvent.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIWindow.h>
#include <UITouch.h>
#include <UISwitch.h>
#include <UISlider.h>
#include <UIApplication.h>

#include <iostream>
#include <string>
#include <vector>

using namespace NXKit;
namespace NXKit {
class UIGestureRecognizerTestHarness {
public:
    static void resumeControllerFocus(const std::shared_ptr<UIWindow>& window) {
        auto press = new_shared<UIPress>(); press->_window = window; press->_responder = window;
        auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press)); window->sendEvent(event);
    }
    static void send(const std::shared_ptr<UIWindow>& window, const std::shared_ptr<UITouch>& touch,
                     UITouchPhase phase, NXKit::NXPoint point, double seconds) {
        touch->_window = window; touch->_phase = phase; touch->updateAbsoluteLocation(point);
        touch->_timestamp.startTime.tv_sec = 1000 + static_cast<long>(seconds);
        touch->_timestamp.startTime.tv_usec = static_cast<long>((seconds - static_cast<long>(seconds)) * 1000000);
        auto event = new_shared<UIEvent>(); event->_allTouches.insert(touch); window->sendEvent(event);
    }
};
}
namespace {
int failures = 0, cases = 0;
void expect(bool result, const std::string& message) { if (!result) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
std::shared_ptr<UIScrollView> enclosingScroll(std::shared_ptr<UIView> view) {
    while (view) { if (const auto scroll = std::dynamic_pointer_cast<UIScrollView>(view)) return scroll; view = view->superview().lock(); }
    return nullptr;
}
struct Fixture {
    std::shared_ptr<UIWindow> window = new_shared<UIWindow>();
    std::shared_ptr<UIViewController> controller;
    std::shared_ptr<UIView> root;
    std::shared_ptr<UIControl> row;
    std::shared_ptr<UIScrollView> scroll;
    std::shared_ptr<UITouch> touch;
    std::string name;
    NXKit::NXPoint point;
    int actions = 0, cancels = 0;
    double clock = 0;
    Fixture(int kind) {
        window->setFrame({0, 0, 400, 600});
        if (kind == 0) {
            name = "fullscreen"; auto screen = new_shared<GlassScreenView>(); root = screen;
            root->setFrame(window->bounds()); row = screen->action("background");
        } else if (kind == 1) {
            name = "navigation"; auto screen = new_shared<GlassNavigationDemoViewController>(); controller = screen;
            root = screen->view(); root->setFrame(window->bounds()); root->layoutIfNeeded(); row = screen->action("next");
        } else if (kind == 5) {
            name = "large-title list";
            auto source = new_shared<GlassListViewController>(0);
            auto navigation = new_shared<UINavigationController>(source);
            source->list()->onLargeTitleDemo();
            controller = navigation;
            root = navigation->topViewController()->view(); root->setFrame(window->bounds()); root->layoutIfNeeded();
            row = std::dynamic_pointer_cast<UIControl>(root->subviews().front());
        } else {
            const auto page = kind == 2 ? SystemControlsDemoViewController::Page::catalog
                : kind == 3 ? SystemControlsDemoViewController::Page::slider : SystemControlsDemoViewController::Page::switchControl;
            name = kind == 2 ? "catalog" : kind == 3 ? "slider settings" : "switch settings";
            auto screen = new_shared<SystemControlsDemoViewController>(page); controller = screen;
            root = screen->view(); root->setFrame(window->bounds()); root->layoutIfNeeded();
            row = screen->action(kind == 2 ? "slider" : kind == 3 ? "value" : "on-tint");
        }
        window->addSubview(root); root->layoutIfNeeded();
        // Some Demo layouts rebuild their row objects once attached to the
        // final viewport. Resolve the actual current row again afterwards.
        if (kind == 0) row = std::static_pointer_cast<GlassScreenView>(root)->action("background");
        if (kind == 1) row = std::static_pointer_cast<GlassNavigationDemoViewController>(controller)->action("next");
        if (kind >= 2 && kind < 5) {
            const auto screen = std::static_pointer_cast<SystemControlsDemoViewController>(controller);
            row = screen->action(kind == 2 ? "slider" : kind == 3 ? "value" : "on-tint");
        }
        if (!row) throw std::runtime_error("Missing selectable row for " + name);
        scroll = enclosingScroll(row->superview().lock());
        if (!scroll) throw std::runtime_error("Missing scroll for " + name);
        row->primaryAction = UIAction("Select", [&] { ++actions; });
        row->addAction(UIAction("Cancel", [&] { ++cancels; }), UIControlEvent::touchCancel);
        // Bring settings rows into the viewport without making assumptions
        // about their arrangement or the size of a particular device.
        auto location = row->convertToView({row->bounds().width() * .5f, row->bounds().height() * .5f}, window);
        if (location.y < 70 || location.y > 500) {
            scroll->setContentOffset(scroll->contentOffset() + NXKit::NXPoint(0, location.y - 250), false);
            location = row->convertToView({row->bounds().width() * .5f, row->bounds().height() * .5f}, window);
        }
        point = location;
    }
    void send(UITouchPhase phase, NXKit::NXPoint delta = {}) {
        if (phase == UITouchPhase::began) touch = new_shared<UITouch>(++nextID, point, Timer());
        UIGestureRecognizerTestHarness::send(window, touch, phase, point + delta, clock); clock += .04;
    }
    void admitHold() {
        // Core's independent timed test verifies the real150ms deadline.
        // Resolve this finite gate deterministically for the list matrix.
        for (auto ancestor = row->superview().lock(); ancestor; ancestor = ancestor->superview().lock())
            if (const auto list = std::dynamic_pointer_cast<UIScrollView>(ancestor))
                for (const auto& recognizer : *list->gestureRecognizers())
                    if (recognizer->delaysTouchesBegan() && recognizer->state() == UIGestureRecognizerState::possible)
                        recognizer->setState(UIGestureRecognizerState::failed);
        clock += .16;
    }
private:
    static unsigned long nextID;
};
unsigned long Fixture::nextID = 100;
void actualRows() {
    for (int kind = 0; kind < 6; ++kind) {
        {
            Fixture f(kind); ++cases; f.send(UITouchPhase::began);
            expect(!f.row->isTracking() && f.actions == 0, f.name + " short contact waits for scroll admission");
            f.send(UITouchPhase::ended);
            expect(f.actions == 1 && !f.row->isHighlighted(), f.name + " tap selects exactly once on release");
            // Touch hides focus appearance. Controller input resumes the
            // retained destination selected by the existing primary action.
            UIGestureRecognizerTestHarness::resumeControllerFocus(f.window);
            expect(f.window->focusSystem()->focusedItem().lock() == f.row, f.name + " selection preserves existing row focus behavior");
        }
        {
            Fixture f(kind); ++cases; f.send(UITouchPhase::began); f.send(UITouchPhase::moved, {0, -20}); f.send(UITouchPhase::moved, {0, -45});
            expect(f.scroll->isDragging() && !f.row->isTracking() && !f.row->isHighlighted() && f.cancels == 0,
                f.name + " fast vertical swipe never admits or highlights row");
            f.send(UITouchPhase::ended, {0, -45}); expect(f.actions == 0, f.name + " fast scrolling never selects row");
        }
        {
            Fixture f(kind); ++cases; f.send(UITouchPhase::began); f.admitHold();
            expect(f.row->isTracking() && f.row->isHighlighted() && f.actions == 0, f.name + " admitted hold highlights without selecting");
            for (const auto& recognizer : *f.row->gestureRecognizers()) expect(recognizer->state() == UIGestureRecognizerState::possible,
                f.name + " row recognizers remain possible while holding");
            expect(f.scroll->panGestureRecognizer()->state() == UIGestureRecognizerState::possible, f.name + " scroll pan remains possible while holding");
            f.send(UITouchPhase::moved, {0, 7});
            expect(f.row->isHighlighted() && !f.scroll->isDragging(), f.name + " sub-threshold movement keeps selection possible");
            f.send(UITouchPhase::moved, {0, -20}); f.send(UITouchPhase::moved, {0, -45});
            expect(f.scroll->isDragging() && !f.row->isTracking() && !f.row->isHighlighted() && f.cancels == 1,
                f.name + " held vertical drag cancels row and gives scrolling ownership");
            f.send(UITouchPhase::ended, {0, -45}); expect(f.actions == 0, f.name + " dragging never selects row on release");
        }
        {
            Fixture f(kind); ++cases; f.send(UITouchPhase::began); f.admitHold(); const auto offset = f.scroll->contentOffset();
            f.send(UITouchPhase::moved, {30, 0}); f.send(UITouchPhase::ended, {30, 0});
            expect(!f.scroll->isDragging() && f.scroll->contentOffset() == offset && f.actions == 0 && !f.row->isHighlighted(),
                f.name + " unsupported horizontal drag clears selection without horizontal scrolling");
        }
        {
            Fixture f(kind); ++cases; f.send(UITouchPhase::began); f.admitHold(); f.send(UITouchPhase::ended);
            expect(f.actions == 1 && f.cancels == 0 && !f.row->isHighlighted(), f.name + " stationary held release selects once");
        }
    }
}
void explicitPoliciesAndAccessories() {
    for (const bool registered : {false, true}) {
        ++cases; auto window = new_shared<UIWindow>(); window->setFrame({0, 0, 400, 600});
        auto list = new_shared<GlassDemoListView>(); list->setFrame(window->bounds()); list->setContentSize({400, 1000}); list->setDelaysContentTouches(false); window->addSubview(list);
        auto row = new_shared<UIControl>(NXRect(20, 70, 360, 52)); list->addSubview(row); if (registered) list->registerSelectableRow(row);
        expect(list->touchesShouldCancelInContentView(row) == registered, "only explicitly registered rows override ordinary control cancellation policy");
        list->setCanCancelContentTouches(false); auto touch = new_shared<UITouch>(1000 + registered, NXKit::NXPoint(70, 90), Timer());
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::began, {70, 90}, 1);
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::moved, {70, 115}, 1.1);
        expect(row->isTracking() && !list->isDragging(), "explicit canCancelContentTouches false preserves admitted content even for selectable row");
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::cancelled, {70, 115}, 1.2);
    }
    for (bool slider : {false, true}) {
        ++cases; auto window = new_shared<UIWindow>(); window->setFrame({0, 0, 400, 600});
        auto list = new_shared<GlassDemoListView>(); list->setFrame(window->bounds()); list->setContentSize({400, 1000}); list->setDelaysContentTouches(false); window->addSubview(list);
        auto row = new_shared<UIControl>(NXRect(20, 70, 360, 80)); list->addSubview(row); list->registerSelectableRow(row);
        const auto control = slider ? std::static_pointer_cast<UIControl>(new_shared<UISlider>(NXRect(30, 20, 260, 44)))
            : std::static_pointer_cast<UIControl>(new_shared<UISwitch>(NXRect(250, 20, 63, 28)));
        row->addSubview(control); int selected = 0; row->primaryAction = UIAction("Select", [&] { ++selected; });
        expect(!list->touchesShouldCancelInContentView(control), "accessory control does not inherit its registered row's cancellation policy");
        NXKit::NXPoint location = control->convertToView({control->bounds().width() * .5f, control->bounds().height() * .5f}, window);
        if (slider) { const auto s = std::static_pointer_cast<UISlider>(control); s->setValue(.5); s->layoutIfNeeded(); const auto thumb = s->thumbRectForBounds(s->bounds(), s->trackRectForBounds(s->bounds()), s->value()); location = s->convertToView({thumb.midX(), thumb.midY()}, window); }
        auto touch = new_shared<UITouch>(1100 + slider, location, Timer());
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::began, location, 1);
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::moved, location + NXKit::NXPoint(0, 30), 1.1);
        expect(!list->isDragging() && selected == 0, "held accessory keeps its exclusive gesture and never selects row");
        UIGestureRecognizerTestHarness::send(window, touch, UITouchPhase::cancelled, location + NXKit::NXPoint(0, 30), 1.2);
    }
}
}
int main() {
    SkiaCtx::_main = std::make_shared<Internal::RasterTestContext>(); UIApplication::shared = new_shared<UIApplication>();
    actualRows(); explicitPoliciesAndAccessories();
    UIApplication::shared.reset(); SkiaCtx::_main.reset();
    std::cout << "Demo list arbitration: " << cases << " cases, " << failures << " failures\n"; return failures ? 1 : 0;
}
