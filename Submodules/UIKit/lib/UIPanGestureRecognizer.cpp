#include <UIPanGestureRecognizer.h>
#include <UITouch.h>
#include <UIWindow.h>

namespace NXKit {

#define THRESHOLD 10

NXPoint UIPanGestureRecognizer::translationInView(std::shared_ptr<UIView> view) {
    if (!trackingTouch) return NXPoint();

    auto positionInTargetView = trackingTouch->locationIn(view);
    NXPoint initialPositionInTargetView;
    if (trackingTouch->window().expired()) initialPositionInTargetView = initialTouchPoint;
    else { initialPositionInTargetView = view ? trackingTouch->window().lock()->convertToView(initialTouchPoint, view) : initialTouchPoint; }

    return positionInTargetView - initialPositionInTargetView;
}

void UIPanGestureRecognizer::setTranslation(NXPoint translation, std::shared_ptr<UIView> inView) {
    if (!trackingTouch) return;

    auto positionInTargetView = trackingTouch->locationIn(nullptr);
    initialTouchPoint = positionInTargetView - translation;
}

NXPoint UIPanGestureRecognizer::velocityIn(std::shared_ptr<UIView> view, float timeDiffSeconds) {
    if (!trackingTouch || timeDiffSeconds == 0) return NXPoint();

    NXPoint curPos = trackingTouch->locationIn(view);
    NXPoint prevPos = trackingTouch->previousLocationIn(view);

    return (curPos - prevPos) / (float)timeDiffSeconds;
}

NXPoint UIPanGestureRecognizer::velocityIn(std::shared_ptr<UIView> view) {
    float timeDiffSeconds = touchesMovedTimestamp - previousTouchesMovedTimestamp;
    timeDiffSeconds /= 100000.0f;
    return velocityIn(view, timeDiffSeconds);
}

void UIPanGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        initialTouchPoint = trackingTouch->locationIn(nullptr);
        touchesMovedTimestamp = trackingTouch->timestamp();
        previousTouchesMovedTimestamp = trackingTouch->timestamp();
        _state = UIGestureRecognizerState::possible;
        onStateChanged(shared_from_this());
    }
}

void UIPanGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        if (state() == UIGestureRecognizerState::possible) {
            NXPoint diff = initialTouchPoint - trackingTouch->locationIn(nullptr);
//            printf("x %f, y %f\n", diff.x, diff.y);
            if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
                // Reset initial touch point to remove "jiggle" effect after start of recognition
                initialTouchPoint = trackingTouch->locationIn(nullptr);
                touchesMovedTimestamp = trackingTouch->timestamp();
                previousTouchesMovedTimestamp = trackingTouch->timestamp();
                setState(UIGestureRecognizerState::began);
                setState(UIGestureRecognizerState::changed);
            }
        } else if (state() == UIGestureRecognizerState::changed) {
            previousTouchesMovedTimestamp = touchesMovedTimestamp;
            touchesMovedTimestamp = trackingTouch->timestamp();
            _state = UIGestureRecognizerState::changed;
            onStateChanged(shared_from_this());
        }
    }
}

void UIPanGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        previousTouchesMovedTimestamp = touchesMovedTimestamp;
        touchesMovedTimestamp = trackingTouch->timestamp();
        setState(UIGestureRecognizerState::ended);
        trackingTouch = nullptr;
    }
}

}
