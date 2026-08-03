#include <UIAction.h>
#include <UIControl.h>
#include <UIEvent.h>
#include <UILabel.h>
#include <UIScrollView.h>
#include <UITextView.h>
#include <UITouch.h>
#include <UIWindow.h>

#include <iostream>
#include <memory>
#include <stdexcept>
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
        NXPoint location,
        Timer timestamp = Timer()
    ) {
        touch->updateAbsoluteLocation(location);
        touch->_phase = phase;
        touch->_timestamp = timestamp;
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
    NXPoint location,
    Timer timestamp = Timer()
) {
    UIGestureRecognizerTestHarness::updateTouch(
        touch,
        phase,
        location,
        timestamp
    );
    window->sendEvent(event);
}

} // namespace

int main() {
    auto intrinsicContainer = new_shared<UIView>(NXRect(0, 0, 500, 100));
    intrinsicContainer->setAutolayoutEnabled(true);
    intrinsicContainer->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
    });
    auto intrinsicLabel = new_shared<UILabel>();
    intrinsicLabel->setAutolayoutEnabled(true);
    intrinsicLabel->setText("1");
    intrinsicContainer->addSubview(intrinsicLabel);
    intrinsicContainer->layoutSubviews();
    const auto narrowLabelWidth = intrinsicLabel->frame().width();
    intrinsicLabel->setText("88:88:88");
    intrinsicContainer->layoutIfNeeded();
    expect(
        intrinsicLabel->frame().width() > narrowLabelWidth,
        "changing label text invalidates Yoga's cached intrinsic width"
    );

    auto window = new_shared<UIWindow>();
    window->setFrame(NXRect(0, 0, 320, 480));

    auto scrollView = new_shared<UIScrollView>(window->bounds());
    scrollView->setBounceVertically(true);
    window->addSubview(scrollView);

    auto contentView = new_shared<UIView>(NXRect(0, 0, 320, 800));
    scrollView->addSubview(contentView);
    expect(
        scrollView->contentSize() == NXSize(320, 800),
        "a scroll view preserves legacy inferred content size when unset"
    );
    scrollView->setContentSize(NXSize(320, 900));
    expect(
        scrollView->contentSize() == NXSize(320, 900),
        "an explicit scroll-view content size overrides subview inference"
    );

    bool rejectedInvalidContentSize = false;
    try {
        scrollView->setContentSize(NXSize(-1, 900));
    } catch (const std::invalid_argument&) {
        rejectedInvalidContentSize = true;
    }
    expect(
        rejectedInvalidContentSize,
        "a scroll view rejects negative content dimensions"
    );

    auto nonBouncingWindow = new_shared<UIWindow>();
    nonBouncingWindow->setFrame(NXRect(0, 0, 320, 480));
    auto nonBouncingScrollView = new_shared<UIScrollView>(
        nonBouncingWindow->bounds()
    );
    nonBouncingScrollView->setContentSize(NXSize(320, 900));
    nonBouncingWindow->addSubview(nonBouncingScrollView);

    auto momentumTouch = new_shared<UITouch>(6, NXPoint(20, 300), Timer());
    auto momentumEvent = UIGestureRecognizerTestHarness::eventWithTouch(
        momentumTouch,
        nonBouncingWindow
    );
    sendTouch(
        nonBouncingWindow,
        momentumEvent,
        momentumTouch,
        UITouchPhase::began,
        NXPoint(20, 300),
        Timer(0)
    );
    sendTouch(
        nonBouncingWindow,
        momentumEvent,
        momentumTouch,
        UITouchPhase::moved,
        NXPoint(20, 270),
        Timer(16)
    );
    sendTouch(
        nonBouncingWindow,
        momentumEvent,
        momentumTouch,
        UITouchPhase::moved,
        NXPoint(20, 180),
        Timer(32)
    );
    sendTouch(
        nonBouncingWindow,
        momentumEvent,
        momentumTouch,
        UITouchPhase::ended,
        NXPoint(20, 180),
        Timer(48)
    );
    expect(
        !nonBouncingScrollView->bounceVertically()
            && nonBouncingScrollView->isDecelerating(),
        "a scrollable axis decelerates after release when bouncing is disabled"
    );
    nonBouncingScrollView->setScrollEnabled(false);

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

    auto disabledScrollWindow = new_shared<UIWindow>();
    disabledScrollWindow->setFrame(NXRect(0, 0, 320, 480));

    auto disabledScrollView = new_shared<UIScrollView>(
        disabledScrollWindow->bounds()
    );
    disabledScrollView->setBounceVertically(true);
    disabledScrollView->setScrollEnabled(false);
    disabledScrollWindow->addSubview(disabledScrollView);

    auto disabledScrollContent = new_shared<UIView>(NXRect(0, 0, 320, 800));
    disabledScrollView->addSubview(disabledScrollContent);

    auto disabledScrollControl = new_shared<UIControl>();
    disabledScrollControl->setFrame(NXRect(0, 0, 320, 80));
    disabledScrollContent->addSubview(disabledScrollControl);

    int disabledScrollActionCount = 0;
    disabledScrollControl->primaryAction = UIAction(
        "",
        [&disabledScrollActionCount]() {
            ++disabledScrollActionCount;
        }
    );

    expect(
        !disabledScrollView->isScrollEnabled(),
        "a scroll view reports disabled user scrolling"
    );
    disabledScrollView->setContentOffset(NXPoint(0, 100), false);
    expect(
        disabledScrollView->contentOffset() == NXPoint(0, 100),
        "disabling user scrolling preserves programmatic content offsets"
    );
    disabledScrollView->setContentOffset(NXPoint(), false);

    auto disabledScrollTouch = new_shared<UITouch>(5, NXPoint(20, 20), Timer());
    auto disabledScrollEvent = UIGestureRecognizerTestHarness::eventWithTouch(
        disabledScrollTouch,
        disabledScrollWindow
    );
    sendTouch(
        disabledScrollWindow,
        disabledScrollEvent,
        disabledScrollTouch,
        UITouchPhase::began,
        NXPoint(20, 20)
    );
    sendTouch(
        disabledScrollWindow,
        disabledScrollEvent,
        disabledScrollTouch,
        UITouchPhase::moved,
        NXPoint(20, 50)
    );
    expect(
        disabledScrollView->contentOffset() == NXPoint(),
        "a drag does not move a scroll view when user scrolling is disabled"
    );
    expect(
        disabledScrollControl->isHighlighted(),
        "a disabled ancestor scroll recognizer does not cancel control tracking"
    );
    sendTouch(
        disabledScrollWindow,
        disabledScrollEvent,
        disabledScrollTouch,
        UITouchPhase::ended,
        NXPoint(20, 50)
    );
    expect(
        disabledScrollActionCount == 1,
        "a control action completes inside a scroll-disabled view"
    );

    auto textView = new_shared<UITextView>();
    expect(
        textView->textContainerInset() == UIEdgeInsets(8, 0, 8, 0),
        "a text view uses UIKit's default vertical text-container inset"
    );
    expect(textView->isScrollEnabled(), "a text view scrolls by default");
    expect(
        !textView->isHorizontalScrollEnabled(),
        "a text view wraps instead of scrolling horizontally by default"
    );
    textView->setHorizontalScrollEnabled(true);
    expect(
        textView->isHorizontalScrollEnabled(),
        "a text view can opt into unwrapped horizontal scrolling"
    );
    expect(
        textView->bounceHorizontally(),
        "enabling horizontal text scrolling enables its scroll physics"
    );
    textView->setHorizontalScrollEnabled(false);
    expect(
        !textView->bounceHorizontally(),
        "disabling horizontal text scrolling restores wrapped-axis physics"
    );
    textView->setText("A multiline\ntext value");
    expect(
        textView->text() == "A multiline\ntext value",
        "a text view stores multiline UTF-8 text"
    );

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
