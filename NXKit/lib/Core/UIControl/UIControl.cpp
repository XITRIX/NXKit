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
//    gesture->onStateChanged = [this](UIGestureRecognizerState state) {
//        if (state == UIGestureRecognizerState::ENDED) {
//            getActions()[BUTTON_A].action();
//        }
//    };
    addGestureRecognizer(gesture);
}

bool UIControl::canBecomeFocused() {
    return true;
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

}
