#include <UIPanGestureRecognizer.h>
#include <UITouch.h>
#include <UIWindow.h>

#include <algorithm>
#include <utility>

namespace NXKit {

#define THRESHOLD 10

NXPoint UIPanGestureRecognizer::translationInView(const std::shared_ptr<UIView>& view) {
    if (!trackingTouch) return NXPoint();

    auto positionInTargetView = trackingTouch->locationIn(view);
    NXPoint initialPositionInTargetView;
    if (trackingTouch->window().expired()) initialPositionInTargetView = initialTouchPoint;
    else { initialPositionInTargetView = view ? trackingTouch->window().lock()->convertToView(initialTouchPoint, view) : initialTouchPoint; }

    return positionInTargetView - initialPositionInTargetView;
}

void UIPanGestureRecognizer::setTranslation(NXPoint translation, const std::shared_ptr<UIView>& inView) {
    if (!trackingTouch) return;

    auto positionInTargetView = trackingTouch->locationIn(nullptr);
    initialTouchPoint = positionInTargetView - translation;
}

NXPoint UIPanGestureRecognizer::velocityIn(const std::shared_ptr<UIView>& view, float timeDiffSeconds) {
    if (timeDiffSeconds <= 0) return NXPoint();

    auto clampedTimeDiffSeconds = std::max(timeDiffSeconds, 1.0f / 240.0f);
    auto curPos = convertTrackedPointToView(lastVelocitySampleEndLocation, view);
    auto prevPos = convertTrackedPointToView(lastVelocitySampleStartLocation, view);

    return (curPos - prevPos) / clampedTimeDiffSeconds;
}

NXPoint UIPanGestureRecognizer::velocityIn(const std::shared_ptr<UIView>& view) {
    if (!hasVelocitySample) return NXPoint();
    return velocityIn(std::move(view), lastVelocitySampleDurationSeconds);
}

void UIPanGestureRecognizer::resetVelocityTracking() {
    hasProcessedMovementSample = false;
    hasVelocitySample = false;
    lastVelocitySampleDurationSeconds = 0;
    lastProcessedLocation = NXPoint();
    lastVelocitySampleStartLocation = NXPoint();
    lastVelocitySampleEndLocation = NXPoint();
}

void UIPanGestureRecognizer::beginVelocityTracking(const std::shared_ptr<UITouch>& touch) {
    if (!touch) return;

    trackingWindow = touch->window();
    hasProcessedMovementSample = true;
    hasVelocitySample = false;
    lastProcessedLocation = touch->absoluteLocation();
    lastProcessedTimestamp = touch->timestamp();
    lastVelocitySampleStartLocation = lastProcessedLocation;
    lastVelocitySampleEndLocation = lastProcessedLocation;
    lastVelocitySampleDurationSeconds = 0;
}

void UIPanGestureRecognizer::recordVelocitySample() {
    if (!trackingTouch) return;

    auto currentLocation = trackingTouch->absoluteLocation();
    auto currentTimestamp = trackingTouch->timestamp();

    if (!hasProcessedMovementSample) {
        beginVelocityTracking(trackingTouch);
        return;
    }

    auto elapsedMilliseconds = currentTimestamp - lastProcessedTimestamp;
    if (elapsedMilliseconds <= 0) {
        lastProcessedLocation = currentLocation;
        lastProcessedTimestamp = currentTimestamp;
        return;
    }

    lastVelocitySampleStartLocation = lastProcessedLocation;
    lastVelocitySampleEndLocation = currentLocation;
    lastVelocitySampleDurationSeconds = elapsedMilliseconds / 1000.0f;
    hasVelocitySample = true;

    lastProcessedLocation = currentLocation;
    lastProcessedTimestamp = currentTimestamp;
}

NXPoint UIPanGestureRecognizer::convertTrackedPointToView(NXPoint point, const std::shared_ptr<UIView>& view) const {
    auto window = trackingWindow.lock();
    if (!window || !view) return point;
    return window->convertToView(point, view);
}

void UIPanGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        initialTouchPoint = trackingTouch->locationIn(nullptr);
        trackingWindow = trackingTouch->window();
        resetVelocityTracking();
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
                beginVelocityTracking(trackingTouch);
                setState(UIGestureRecognizerState::began);
                setState(UIGestureRecognizerState::changed);
            }
        } else if (state() == UIGestureRecognizerState::changed) {
            recordVelocitySample();
            _state = UIGestureRecognizerState::changed;
            onStateChanged(shared_from_this());
        }
    }
}

void UIPanGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        setState(UIGestureRecognizerState::ended);
        trackingTouch = nullptr;
        trackingWindow.reset();
    }
}

}
