//
//  UIGestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#include "UIGestureRecognizer.hpp"

namespace NXKit {

void UIGestureRecognizer::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void UIGestureRecognizer::setState(UIGestureRecognizerState state) {
    if (this->state == state) return;
    auto oldState = this->state;
    this->state = state;
    onStateChanged();
    switch (state) {
        case UIGestureRecognizerState::RECOGNIZED:
        case UIGestureRecognizerState::ENDED:
        case UIGestureRecognizerState::FAILED:
            this->state = UIGestureRecognizerState::POSSIBLE;
            break;
        default: break;
    }
}

void UIGestureRecognizer::touchesBegan(std::vector<UITouch*> touches, UIEvent* event) {}
void UIGestureRecognizer::touchesMoved(std::vector<UITouch*> touches, UIEvent* event) {}
void UIGestureRecognizer::touchesEnded(std::vector<UITouch*> touches, UIEvent* event) {}

}
