#include "../app/Screens/GlassDemoViewController/GlassDemoScene.hpp"
#include "CABackdropEffectLayer.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include <UIEvent.h>
#include <UITouch.h>
#include <iostream>
#include <limits>
using namespace NXKit;
namespace NXKit {
// This fixture has no native window or safe-area backend.
class GalleryWindow final : public UIWindow {
public:
    void layoutSubviews() override { UIView::layoutSubviews(); }
};
class UIGestureRecognizerTestHarness {
public:
    static std::shared_ptr<UIEvent> event(const std::shared_ptr<UITouch>& touch,
                                          const std::shared_ptr<UIWindow>& window) {
        auto event = new_shared<UIEvent>();
        event->_allTouches.insert(touch);
        touch->_window = window;
        return event;
    }
    static void send(const std::shared_ptr<UIWindow>& window, const std::shared_ptr<UIEvent>& event,
                     const std::shared_ptr<UITouch>& touch, UITouchPhase phase, NXPoint point) {
        touch->updateAbsoluteLocation(point);
        touch->_phase = phase;
        window->sendEvent(event);
    }
};
class GlassInteractionTestHarness {
public:
    static void advance(const std::shared_ptr<BackdropEffectView>& view, double seconds) {
        view->_advanceInteraction(seconds);
    }
    static auto motion(const std::shared_ptr<BackdropEffectView>& view) {
        return *view->_backdropEffectLayer()->_interaction;
    }
};
} // namespace NXKit
int main() {
    int failures = 0;
    const auto check = [&](bool result, const char* message) {
        if (!result) {
            ++failures;
            std::cerr << message << '\n';
        }
    };
    for (int demo = 0; demo < GlassDemoCount; ++demo) {
        auto scene = new_shared<GlassDemoScene>(static_cast<GlassDemoID>(demo), nullptr);
        const auto initial = scene->glassItems().front()->frame();
        check(scene->glassItems().size() == static_cast<size_t>(GlassDemoElementCount(demo)),
              "Every menu entry constructs its full scene");
        for (auto item : scene->glassItems()) {
            check(item->glassEffect()->isInteractive() == bool(GlassDemos[demo].interactive),
                  "Interaction matches the demo");
            check(item->glassEffect()->adaptsAppearanceToBackdrop() == (demo == GlassDemoContrast26),
                  "Legacy appearance is isolated to its opt-in demo");
            check((item->gestureRecognizers()->size() > 0) ==
                      bool(GlassDemos[demo].movable || GlassDemos[demo].interactive),
                  "Only movable/interactive scenes install touch observers");
        }
        scene->setProgress(1);
        scene->setReversedOrder(true);
        scene->setBackdrop(3);
        scene->reset();
        check(scene->glassItems().front()->frame() == initial && scene->progress() == 0 &&
                  !scene->reversedOrder(),
              "Reset restores geometry and stacking order");
        check(scene->backdrop() == (demo == GlassDemoContrast26 ? 4 : 0),
              "Reset restores the intended backdrop");
        scene->setProgress(std::numeric_limits<float>::quiet_NaN());
        check(scene->progress() == 0, "Nonfinite replay progress is ignored");
        scene->setProgress(2);
        check(scene->progress() == 1, "Replay progress is bounded");
    }
    auto pair = new_shared<GlassDemoScene>(GlassDemoTwoTints, nullptr);
    pair->setProgress(1);
    const NXPoint overlap{335, 220};
    check(pair->hitTest(overlap, nullptr) == pair->glassItems()[1],
          "Coral receives overlapping touches initially");
    pair->setReversedOrder(true);
    check(pair->hitTest(overlap, nullptr) == pair->glassItems()[0],
          "Swapping materials also swaps touch ownership");
    pair->reset();
    check(pair->glassItems()[0]->frame().origin.x < pair->glassItems()[1]->frame().origin.x,
          "Reset separates the pair");
    for (auto demo : {GlassDemoClearButtons, GlassDemoTintedButtons, GlassDemoNavigationButtons,
                      GlassDemoNavigationGrouped}) {
        auto window = new_shared<GalleryWindow>();
        window->setFrame({0, 0, 720, 440});
        auto scene = new_shared<GlassDemoScene>(demo, nullptr);
        window->addSubview(scene);
        check(scene->buttonControls().size() == 3, "Fixed interactive examples expose three actual controls");
        for (float width : {294.0f, 390.0f, 624.0f, 720.0f}) {
            scene->setBounds({0, 0, width, 440 * width / 720});
            scene->layoutIfNeeded();
            for (size_t i = 0; i < 3; ++i) {
                auto item = scene->glassItems()[i];
                const auto frame = item->frame();
                check(frame.width() >= 52 && frame.height() >= 52 && frame.origin.x >= 0 &&
                          frame.maxX() <= width && frame.origin.y >= 0 &&
                          frame.maxY() <= scene->bounds().height(),
                      "Fixed controls retain their touch size and fit narrow and wide previews");
                check(scene->hitTest(NXPoint(frame.midX(), frame.midY()), nullptr) ==
                          scene->buttonControls()[i],
                      "The glyph does not intercept the button's hit target");
                for (auto recognizer : *item->gestureRecognizers())
                    check(!std::dynamic_pointer_cast<UIPanGestureRecognizer>(recognizer),
                          "Fixed controls never install an application drag recognizer");
            }
            if (GlassDemoIsNavigationButtons(demo)) {
                check(scene->glassItems()[0]->frame().origin.x == 20 &&
                          scene->glassItems()[2]->frame().maxX() == width - 20,
                      "Mock navigation buttons stay anchored to the leading and trailing edges");
                check(scene->glassItems()[1]->frame().maxX() + 12 == scene->glassItems()[2]->frame().minX(),
                      "Trailing buttons retain their gap");
            }
        }
        scene->setBounds({0, 0, 720, 440});
        scene->layoutIfNeeded();
        auto item = scene->glassItems()[2];
        const auto original = item->frame();
        if (demo == GlassDemoNavigationGrouped)
            scene->setBackdrop(2);
        auto control = scene->buttonControls()[2];
        auto touch = new_shared<UITouch>(900 + demo, NXPoint(original.midX(), original.midY()), Timer());
        auto event = UIGestureRecognizerTestHarness::event(touch, window);
        UIGestureRecognizerTestHarness::send(window, event, touch, UITouchPhase::began,
                                             NXPoint(original.midX(), original.midY()));
        GlassInteractionTestHarness::advance(item, .15);
        check(GlassInteractionTestHarness::motion(item).glow() > .9f && control->isHighlighted(),
              "Pressing a fixed button lights the glass and tracks its control");
        check(!touch->hasBeenCancelledByAGestureRecognizer() && item->frame() == original,
              "Interactive feedback preserves the button frame and touch delivery");
        if (demo == GlassDemoNavigationGrouped) {
            const auto host = item->superview().lock();
            const auto container = std::dynamic_pointer_cast<GlassContainerView>(host->superview().lock());
            check(container && container->effect().spacing() == 8 &&
                      scene->glassItems()[0]->superview().lock() == host,
                  "The grouped navigation example shares one glass container with a press-sized merge "
                  "distance");
            const auto render = [&] {
                auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1440, 880));
                surface->getCanvas()->scale(2, 2);
                surface->getCanvas()->clear(SK_ColorBLACK);
                scene->layer()->skiaRender(surface->getCanvas());
                SkBitmap pixels;
                pixels.allocN32Pixels(1440, 880);
                surface->readPixels(pixels, 0, 0);
                return pixels;
            };
            auto joined = render();
            auto effect = container->effect();
            effect.setSpacing(0);
            container->setEffect(effect);
            auto separate = render();
            effect.setSpacing(8);
            container->setEffect(effect);
            int neckDifference = 0;
            for (int y = 96; y < 104; ++y)
                for (int x = 1272; x < 1280; ++x)
                    neckDifference =
                        std::max(neckDifference, static_cast<int>(SkColorGetB(joined.getColor(x, y))) -
                                                     static_cast<int>(SkColorGetB(separate.getColor(x, y))));
            check(neckDifference > 20,
                  "Press inflation creates a visible SDF neck between fixed trailing buttons");
        }
        UIGestureRecognizerTestHarness::send(window, event, touch, UITouchPhase::ended,
                                             NXPoint(original.midX(), original.midY()));
        GlassInteractionTestHarness::advance(item, 2);
        check(control->isSelected() && !control->isHighlighted() &&
                  GlassInteractionTestHarness::motion(item).atRest(),
              "Releasing activates the button and settles its glass");
        auto cancelled = new_shared<UITouch>(950 + demo, NXPoint(original.midX(), original.midY()), Timer());
        auto cancelledEvent = UIGestureRecognizerTestHarness::event(cancelled, window);
        UIGestureRecognizerTestHarness::send(window, cancelledEvent, cancelled, UITouchPhase::began,
                                             NXPoint(original.midX(), original.midY()));
        UIGestureRecognizerTestHarness::send(window, cancelledEvent, cancelled, UITouchPhase::moved,
                                             NXPoint(original.midX(), original.midY()) + NXPoint(30, 10));
        GlassInteractionTestHarness::advance(item, .15);
        check(item->frame() == original, "Sliding a finger stretches the glass without moving the item");
        UIGestureRecognizerTestHarness::send(window, cancelledEvent, cancelled, UITouchPhase::cancelled,
                                             NXPoint(original.midX(), original.midY()) + NXPoint(30, 10));
        GlassInteractionTestHarness::advance(item, 2);
        check(control->isSelected() && !control->isHighlighted() &&
                  GlassInteractionTestHarness::motion(item).atRest(),
              "Cancellation settles feedback without firing another action");
        scene->reset();
        check(!control->isSelected() && item->frame() == original,
              "Reset restores button state as well as geometry");
    }
    return failures ? 1 : 0;
}
