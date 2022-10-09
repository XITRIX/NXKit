//
//  UIControl+GestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/UIControl/UIControl.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UIControl::GestureRecognizer::GestureRecognizer(UIControl* control):
    control(control)
{ }

void UIControl::GestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
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

void UIControl::GestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        Point location = trackingTouch->locationIn(this->view.lock());
        bool touchInside = view.lock()->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);

        if (control->isHighlighted() && !touchInside) {
            control->onEvent(UIControlTouchEvent::touchDragOutside);
        } else if (!control->isHighlighted() && touchInside) {
            control->onEvent(UIControlTouchEvent::touchDragInside);
        }

        control->setHighlighted(touchInside);
    }
}

void UIControl::GestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        control->setHighlighted(false);
        Point location = trackingTouch->locationIn(this->view.lock());
        bool touchInside = view.lock()->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);
        if (touchInside) {
//            if (control->canBecomeFocused())
//                Application::shared()->setFocus(control);
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

void UIControl::GestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesCancelled(touches, event);
    control->setHighlighted(false);
    trackingTouch = nullptr;
}

}
