//
//  UIPanGestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 20.08.2022.
//

#include <Core/UIPanGestureRecognizer/UIPanGestureRecognizer.hpp>
#include <Core/UIView/UIView.hpp>

namespace NXKit {

#define THRESHOLD 10

Point UIPanGestureRecognizer::translationInView(UIView* view) {
    if (!trackingTouch) return Point();

    auto positionInTargetView = trackingTouch->locationIn(view);
    auto initialPositionInTargetView = view ? view->convert(initialTouchPoint, trackingTouch->window) : initialTouchPoint;

    return positionInTargetView - initialPositionInTargetView;
}

void UIPanGestureRecognizer::setTranslation(Point translation, UIView* inView) {
    if (!trackingTouch) return;

    auto positionInTargetView = trackingTouch->locationIn(nullptr);
    initialTouchPoint = positionInTargetView - translation;
}

Point UIPanGestureRecognizer::velocityIn(UIView* view, float timeDiffSeconds) {
    if (!trackingTouch || timeDiffSeconds == 0) return Point();

    Point curPos = trackingTouch->locationIn(view);
    Point prevPos = trackingTouch->previousLocationIn(view);

    return (curPos - prevPos) / (float)timeDiffSeconds;
}

Point UIPanGestureRecognizer::velocityIn(UIView* view) {
    float timeDiffSeconds = touchesMovedTimestamp - previousTouchesMovedTimestamp;
    timeDiffSeconds /= 100000.0f;
    return velocityIn(view, timeDiffSeconds);
}

void UIPanGestureRecognizer::touchesBegan(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        initialTouchPoint = trackingTouch->locationIn(nullptr);
        touchesMovedTimestamp = trackingTouch->timestamp;
        previousTouchesMovedTimestamp = trackingTouch->timestamp;
        onStateChanged(UIGestureRecognizerState::POSSIBLE);
    }
}

void UIPanGestureRecognizer::touchesMoved(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        if (getState() == UIGestureRecognizerState::POSSIBLE) {
            Point diff = initialTouchPoint - trackingTouch->locationIn(nullptr);
            if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
                previousTouchesMovedTimestamp = touchesMovedTimestamp;
                touchesMovedTimestamp = trackingTouch->timestamp;
                setState(UIGestureRecognizerState::CHANGED);
            }
        } else if (getState() == UIGestureRecognizerState::CHANGED) {
            previousTouchesMovedTimestamp = touchesMovedTimestamp;
            touchesMovedTimestamp = trackingTouch->timestamp;
            onStateChanged(UIGestureRecognizerState::CHANGED);
        }
    }
}

void UIPanGestureRecognizer::touchesEnded(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        previousTouchesMovedTimestamp = touchesMovedTimestamp;
        touchesMovedTimestamp = trackingTouch->timestamp;
        setState(UIGestureRecognizerState::ENDED);
        trackingTouch = nullptr;
    }
}

}
