//
//  UIControl.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/UIControl/UIControl.hpp>

namespace NXKit {

UIControl::UIControl() {
    auto gesture = new GestureRecognizer(this);
    gesture->onStateChanged = [this](UIGestureRecognizerState state) {
        if (state == UIGestureRecognizerState::ENDED) {
            getFirstAvailableAction(BUTTON_A).action();
        }
    };
//    actionSub = InputManager::shared()->getInputUpdated()->subscribe([this]() {
//        if (isFocused() && InputManager::shared()->getButtonDown(BUTTON_A)) {
//            getActions()[BUTTON_A].action();
//            clickAlpha = 1;
//            animate(0.2, [this]() {
//                clickAlpha = 0;
//            });
//        }
//    });
    addGestureRecognizer(gesture);
}
UIControl::~UIControl() {
//    InputManager::shared()->getInputUpdated()->unsubscribe(actionSub);
}

bool UIControl::canBecomeFocused() {
    return !isHidden();
}

bool UIControl::isEnabled() {
    return enabled;
}

bool UIControl::isSelected() {
    return selected;
}

bool UIControl::isHighlighted() {
    return highlighted;
}

void UIControl::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void UIControl::setSelected(bool selected) {
    this->selected = selected;
}

void UIControl::setHighlighted(bool highlighted) {
    clickAlpha = highlighted ? 1 : 0;
    this->highlighted = highlighted;
}

bool UIControl::press(ControllerButton button) {
    if (button == BUTTON_A) {
        auto action = getFirstAvailableAction(BUTTON_A);
        if (!action.condition())
            return UIView::press(button);
        
        action.action();
        clickAlpha = 1;
        animate(0.2, [this]() {
            clickAlpha = 0;
        });
        return true;
    }
    return UIView::press(button);
}

}
