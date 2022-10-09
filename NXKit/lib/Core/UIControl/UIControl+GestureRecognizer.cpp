//
//  UIControl+GestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/UIControl/UIControl.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UIControl::GestureRecognizer::GestureRecognizer()
{ }

std::shared_ptr<UIControl> UIControl::GestureRecognizer::getControl() {
    if (view.expired()) return nullptr;
    return std::dynamic_pointer_cast<UIControl>(view.lock());
}

void UIControl::GestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (getControl() && !getControl()->isEnabled()) {
        setState(UIGestureRecognizerState::FAILED);
        return;
    }
    if (getControl() && !trackingTouch) {
        trackingTouch = touches[0];
        getControl()->setHighlighted(true);
        getControl()->onEvent(UIControlTouchEvent::touchDown);
    }
}

void UIControl::GestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        Point location = trackingTouch->locationIn(this->view.lock());
        bool touchInside = view.lock()->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);

        if (getControl() && getControl()->isHighlighted() && !touchInside) {
            getControl()->onEvent(UIControlTouchEvent::touchDragOutside);
        } else if (getControl() && !getControl()->isHighlighted() && touchInside) {
            getControl()->onEvent(UIControlTouchEvent::touchDragInside);
        }

        if (getControl())
            getControl()->setHighlighted(touchInside);
    }
}

void UIControl::GestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        getControl()->setHighlighted(false);
        Point location = trackingTouch->locationIn(this->view.lock());
        bool touchInside = view.lock()->getBounds().insetBy(UIEdgeInsets(extraTouchArea)).contains(location);
        if (touchInside) {
//            if (control->canBecomeFocused())
//                Application::shared()->setFocus(control);
            setState(UIGestureRecognizerState::ENDED);
            getControl()->onEvent(UIControlTouchEvent::touchUpInside);
        }
        else {
            setState(UIGestureRecognizerState::FAILED);
            getControl()->onEvent(UIControlTouchEvent::touchUpOutside);
        }
        trackingTouch = nullptr;
    }
}

void UIControl::GestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesCancelled(touches, event);
    getControl()->setHighlighted(false);
    trackingTouch = nullptr;
}

}
