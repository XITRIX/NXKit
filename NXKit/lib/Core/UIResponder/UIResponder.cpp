//
//  UIResponder.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#include <Core/UIResponder/UIResponder.hpp>

namespace NXKit {

void UIResponder::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) {
    if (!getNext()) return;
    getNext()->touchesBegan(touches, withEvent);
}

void UIResponder::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) {
    if (!getNext()) return;
    getNext()->touchesMoved(touches, withEvent);
}

void UIResponder::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent) {
    if (!getNext()) return;
    getNext()->touchesEnded(touches, withEvent);
}

void UIResponder::touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!getNext()) return;
    getNext()->touchesCancelled(touches, withEvent);
}

bool UIResponder::press(ControllerButton button) {
    if (actions.count(button)) {
        for (auto action: actions[button]) {
            if (action.condition()) {
                action.action();
                return true;
            }
        }
    }
    
    if (getNext()) {
        return getNext()->press(button);
    } else {
        return false;
    }
}

std::map<ControllerButton, std::deque<UIAction>> UIResponder::getActions() {
    return actions;
}

UIAction UIResponder::getFirstAvailableAction(ControllerButton button) {
    for (auto action: actions[button]) {
        if (action.condition())
            return action;
    }
    return UIAction();
}

void UIResponder::addAction(ControllerButton button, UIAction action) {
    actions[button].push_front(action);
}

void UIResponder::popAction(ControllerButton button) {
    if (actions[button].size() > 0)
        actions[button].pop_front();

    if (actions[button].size() == 0)
        actions.erase(button);
}

void UIResponder::removeActions(ControllerButton button) {
    actions.erase(button);
}

}
