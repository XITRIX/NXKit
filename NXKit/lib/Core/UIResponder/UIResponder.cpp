//
//  UIResponder.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#include <Core/UIResponder/UIResponder.hpp>

namespace NXKit {

void UIResponder::touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!getNext()) return;
    getNext()->touchesBegan(touches, withEvent);
}

void UIResponder::touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!getNext()) return;
    getNext()->touchesMoved(touches, withEvent);
}

void UIResponder::touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!getNext()) return;
    getNext()->touchesEnded(touches, withEvent);
}

void UIResponder::touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!getNext()) return;
    getNext()->touchesCancelled(touches, withEvent);
}

bool UIResponder::press(ControllerButton button) {
    if (getActions().count(button)) {
        getActions()[button].action();
        return true;
    } else if (getNext()) {
        return getNext()->press(button);
    } else {
        return false;
    }
}

std::map<ControllerButton, UIAction> UIResponder::getActions() {
    return actions;
}

void UIResponder::addAction(ControllerButton button, UIAction action) {
    actions[button] = action;
}

void UIResponder::removeAction(ControllerButton button) {
    actions.erase(button);
}

}
