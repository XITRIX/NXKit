//
//  UIResponder.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#pragma once

#include <vector>

#include <Core/UITouch/UITouch.hpp>
#include <Core/UIEvent/UIEvent.hpp>
#include <Core/UIAction/UIAction.hpp>
#include <Platforms/InputManager.hpp>

#include <map>

namespace NXKit {

class UIResponder {
public:
    virtual UIResponder* getNext() { return nullptr; }

    virtual void touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent);

    std::map<ControllerButton, UIAction> getActions();
    void addAction(ControllerButton button, UIAction action);
    void removeAction(ControllerButton button);

private:
    std::map<ControllerButton, UIAction> actions;
};

}
