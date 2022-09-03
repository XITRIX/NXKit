//
//  UIControl+GestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/UIControl/UIControl.hpp>

namespace NXKit {

UIControl::GestureRecognizer::GestureRecognizer(UIControl* control):
    control(control)
{ }

void UIControl::GestureRecognizer::touchesBegan(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!control->isEnabled()) {
        setState(UIGestureRecognizerState::FAILED);
        return;
    }
    if (!trackingTouch) {
        trackingTouch = touches[0];
        control->setHighlighted(true);
        control->onEvent(UIControlTouchEvent::touchDown);
    }
}

void UIControl::GestureRecognizer::touchesMoved(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        Point location = trackingTouch->locationIn(this->view);
        bool touchInside = view->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);

        if (control->isHighlighted() && !touchInside) {
            control->onEvent(UIControlTouchEvent::touchDragOutside);
        } else if (!control->isHighlighted() && touchInside) {
            control->onEvent(UIControlTouchEvent::touchDragInside);
        }

        control->setHighlighted(touchInside);
    }
}

void UIControl::GestureRecognizer::touchesEnded(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        control->setHighlighted(false);
        Point location = trackingTouch->locationIn(this->view);
        bool touchInside = view->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);
        if (touchInside) {
            setState(UIGestureRecognizerState::ENDED);
            control->onEvent(UIControlTouchEvent::touchUpInside);
        }
        else {
            setState(UIGestureRecognizerState::FAILED);
            control->onEvent(UIControlTouchEvent::touchUpOutside);
        }
        trackingTouch = nullptr;
    }
}

void UIControl::GestureRecognizer::touchesCancelled(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesCancelled(touches, event);
    control->setHighlighted(false);
    trackingTouch = nullptr;
}

}
