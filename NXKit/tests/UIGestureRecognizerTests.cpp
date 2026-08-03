#include <UIAction.h>
#include <UIControl.h>
#include <UIEvent.h>
#include <UIScrollView.h>
#include <UITouch.h>
#include <UIWindow.h>

#include <iostream>
#include <memory>
#include <string>

using namespace NXKit;

namespace NXKit {

class UIGestureRecognizerTestHarness {
public:
    static std::shared_ptr<UIEvent> eventWithTouch(
        const std::shared_ptr<UITouch>& touch,
        const std::shared_ptr<UIWindow>& window
    ) {
        auto event = new_shared<UIEvent>();
        event->_allTouches.insert(touch);
        touch->_window = window;
        return event;
    }

    static void updateTouch(
        const std::shared_ptr<UITouch>& touch,
        UITouchPhase phase,
        NXPoint location
    ) {
        touch->updateAbsoluteLocation(location);
        touch->_phase = phase;
        touch->_timestamp = Timer();
    }
};

} // namespace NXKit

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

class RecordingFocusControl final : public UIControl {
public:
    int focusGainCount = 0;
    int focusLossCount = 0;

    void willGainFocus() override {
        ++focusGainCount;
        UIControl::willGainFocus();
    }

    void willLoseFocus() override {
        ++focusLossCount;
        UIControl::willLoseFocus();
    }
};

void sendTouch(
    const std::shared_ptr<UIWindow>& window,
    const std::shared_ptr<UIEvent>& event,
    const std::shared_ptr<UITouch>& touch,
    UITouchPhase phase,
    NXPoint location
) {
    UIGestureRecognizerTestHarness::updateTouch(touch, phase, location);
    window->sendEvent(event);
}

} // namespace

int main() {
    auto window = new_shared<UIWindow>();
    window->setFrame(NXRect(0, 0, 320, 480));

    auto scrollView = new_shared<UIScrollView>(window->bounds());
    scrollView->setBounceVertically(true);
    window->addSubview(scrollView);

    auto contentView = new_shared<UIView>(NXRect(0, 0, 320, 800));
    scrollView->addSubview(contentView);

    auto control = new_shared<UIControl>();
    control->setFrame(NXRect(0, 0, 320, 80));
    contentView->addSubview(control);

    int primaryActionCount = 0;
    control->primaryAction = UIAction("", [&primaryActionCount]() {
        ++primaryActionCount;
    });

    auto touch = new_shared<UITouch>(1, NXPoint(20, 20), Timer());
    auto event = UIGestureRecognizerTestHarness::eventWithTouch(touch, window);

    sendTouch(window, event, touch, UITouchPhase::began, NXPoint(20, 20));
    expect(control->isHighlighted(), "touch-down highlights the control");

    sendTouch(window, event, touch, UITouchPhase::moved, NXPoint(20, 50));
    expect(
        !control->isHighlighted(),
        "a recognized ancestor pan cancels the control highlight"
    );

    sendTouch(window, event, touch, UITouchPhase::ended, NXPoint(20, 50));
    expect(
        primaryActionCount == 0,
        "lifting after a recognized scroll does not perform the control action"
    );

    auto cancelledTouch = new_shared<UITouch>(2, NXPoint(20, 20), Timer());
    auto cancelledEvent = UIGestureRecognizerTestHarness::eventWithTouch(
        cancelledTouch,
        window
    );
    sendTouch(
        window,
        cancelledEvent,
        cancelledTouch,
        UITouchPhase::began,
        NXPoint(20, 20)
    );
    expect(control->isHighlighted(), "a new touch can track after the scroll ends");
    sendTouch(
        window,
        cancelledEvent,
        cancelledTouch,
        UITouchPhase::cancelled,
        NXPoint(20, 20)
    );
    expect(!control->isHighlighted(), "a cancelled touch clears control tracking");
    expect(primaryActionCount == 0, "a cancelled touch performs no control action");

    auto hierarchyMutationWindow = new_shared<UIWindow>();
    hierarchyMutationWindow->setFrame(NXRect(0, 0, 320, 480));

    auto selfRemovingControl = new_shared<UIControl>();
    selfRemovingControl->setFrame(NXRect(0, 0, 160, 80));
    hierarchyMutationWindow->addSubview(selfRemovingControl);

    int selfRemovingActionCount = 0;
    const std::weak_ptr<UIControl> weakSelfRemovingControl = selfRemovingControl;
    selfRemovingControl->primaryAction = UIAction(
        "",
        [weakSelfRemovingControl, &selfRemovingActionCount]() {
            ++selfRemovingActionCount;
            if (const auto control = weakSelfRemovingControl.lock()) {
                control->removeFromSuperview();
            }
        }
    );

    auto selfRemovingTouch = new_shared<UITouch>(3, NXPoint(20, 20), Timer());
    auto selfRemovingEvent = UIGestureRecognizerTestHarness::eventWithTouch(
        selfRemovingTouch,
        hierarchyMutationWindow
    );
    sendTouch(
        hierarchyMutationWindow,
        selfRemovingEvent,
        selfRemovingTouch,
        UITouchPhase::began,
        NXPoint(20, 20)
    );
    sendTouch(
        hierarchyMutationWindow,
        selfRemovingEvent,
        selfRemovingTouch,
        UITouchPhase::ended,
        NXPoint(20, 20)
    );
    expect(
        selfRemovingActionCount == 1 && selfRemovingControl->superview().expired(),
        "a touch action may remove its own control during recognizer dispatch"
    );
    expect(
        !selfRemovingControl->isFocused(),
        "a detached control reports that it is not focused"
    );

    auto focusWindow = new_shared<UIWindow>();
    focusWindow->setFrame(NXRect(0, 0, 320, 480));

    auto initiallyFocusedControl = new_shared<RecordingFocusControl>();
    initiallyFocusedControl->setFrame(NXRect(0, 0, 100, 80));
    focusWindow->addSubview(initiallyFocusedControl);

    auto requestedFocusControl = new_shared<RecordingFocusControl>();
    requestedFocusControl->setFrame(NXRect(120, 0, 100, 80));
    focusWindow->addSubview(requestedFocusControl);

    focusWindow->updateFocus();
    UIView::completePendingAnimations();
    expect(
        initiallyFocusedControl->isFocused(),
        "the first eligible control starts focused"
    );

    int focusedTouchActionCount = 0;
    bool touchFocusRequestSucceeded = false;
    const std::weak_ptr<UIWindow> weakFocusWindow = focusWindow;
    const std::weak_ptr<RecordingFocusControl> weakRequestedControl =
        requestedFocusControl;
    initiallyFocusedControl->primaryAction = UIAction(
        "",
        [weakFocusWindow, weakRequestedControl, &focusedTouchActionCount,
         &touchFocusRequestSucceeded]() {
            ++focusedTouchActionCount;
            const auto window = weakFocusWindow.lock();
            const auto requestedControl = weakRequestedControl.lock();
            touchFocusRequestSucceeded = window && requestedControl
                && window->focusSystem()->requestFocusUpdate(requestedControl);
        }
    );

    auto focusedTouch = new_shared<UITouch>(4, NXPoint(20, 20), Timer());
    auto focusedTouchEvent = UIGestureRecognizerTestHarness::eventWithTouch(
        focusedTouch,
        focusWindow
    );
    sendTouch(
        focusWindow,
        focusedTouchEvent,
        focusedTouch,
        UITouchPhase::began,
        NXPoint(20, 20)
    );
    expect(
        initiallyFocusedControl->isHighlighted(),
        "touch-down still reaches a control while its focus-loss animation runs"
    );
    expect(
        focusWindow->focusSystem()->focusedItem().expired(),
        "touch-down hides visible controller focus"
    );

    sendTouch(
        focusWindow,
        focusedTouchEvent,
        focusedTouch,
        UITouchPhase::ended,
        NXPoint(20, 20)
    );
    expect(
        focusedTouchActionCount == 1,
        "tapping the previously focused control performs its primary action"
    );
    expect(
        touchFocusRequestSucceeded,
        "a touch-driven action can retarget retained focus"
    );
    expect(
        focusWindow->focusSystem()->focusedItem().expired(),
        "a touch-driven focus request does not reactivate visible focus"
    );
    expect(
        requestedFocusControl->focusGainCount == 0,
        "a touch-driven focus request runs no controller-focus animation"
    );

    if (failures == 0) {
        std::cout << "UIGestureRecognizer tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
