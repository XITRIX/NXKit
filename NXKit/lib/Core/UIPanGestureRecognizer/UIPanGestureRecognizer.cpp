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

void UIPanGestureRecognizer::touchesBegan(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        initialTouchPoint = trackingTouch->locationIn(nullptr);
    }
}

void UIPanGestureRecognizer::touchesMoved(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        if (getState() == UIGestureRecognizerState::POSSIBLE) {
            Point diff = initialTouchPoint - trackingTouch->locationIn(nullptr);
            if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
                setState(UIGestureRecognizerState::CHANGED);
            }
        } else if (getState() == UIGestureRecognizerState::CHANGED) {
            onStateChanged(UIGestureRecognizerState::CHANGED);
        }
    }
}

void UIPanGestureRecognizer::touchesEnded(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        setState(UIGestureRecognizerState::ENDED);
        trackingTouch = nullptr;
    }
}

}
