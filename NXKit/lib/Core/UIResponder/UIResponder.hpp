//
//  UIResponder.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#pragma once

#include <vector>
#include <deque>

#include <Core/UITouch/UITouch.hpp>
#include <Core/UIEvent/UIEvent.hpp>
#include <Core/UIAction/UIAction.hpp>
#include <Platforms/InputManager.hpp>

#include <map>

namespace NXKit {

class UIResponder {
public:
    virtual std::shared_ptr<UIResponder> getNext() { return nullptr; }

    virtual void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent);
    virtual void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent);
    virtual void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> withEvent);
    virtual void touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent);

    virtual bool press(ControllerButton button);

    std::map<ControllerButton, std::deque<UIAction>> getActions();
    UIAction getFirstAvailableAction(ControllerButton button);
    void addAction(ControllerButton button, UIAction action);
    void popAction(ControllerButton button);
    void removeActions(ControllerButton button);

private:
    std::map<ControllerButton, std::deque<UIAction>> actions;
};

}
