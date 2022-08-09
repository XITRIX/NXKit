//
//  UIResponder.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.07.2022.
//

#include "UIResponder.hpp"

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

}
