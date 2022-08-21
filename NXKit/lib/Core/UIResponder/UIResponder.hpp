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

namespace NXKit {

class UIResponder {
public:
    virtual UIResponder* getNext() { return nullptr; }

    virtual void touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent);
    virtual void touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent);
};

}
