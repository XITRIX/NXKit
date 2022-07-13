//
//  UIResponder.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#pragma once

#include <vector>

#include "UITouch.hpp"
#include "UIEvent.hpp"

class UIResponder {
public:
    virtual UIResponder* getNext() { return nullptr; }

    void touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent);
    void touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent);
    void touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent);
    void touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent);
};
