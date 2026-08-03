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

    if (failures == 0) {
        std::cout << "UIGestureRecognizer tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
