//
//  UIResponder.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#include "UIResponder.hpp"

void UIResponder::touchesBegan(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!next) return;
    next->touchesBegan(touches, withEvent);
}

void UIResponder::touchesMoved(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!next) return;
    next->touchesMoved(touches, withEvent);
}

void UIResponder::touchesEnded(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!next) return;
    next->touchesEnded(touches, withEvent);
}

void UIResponder::touchesCancelled(std::vector<UITouch*> touches, UIEvent* withEvent) {
    if (!next) return;
    next->touchesCancelled(touches, withEvent);
}
