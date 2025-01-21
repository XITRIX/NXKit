//
//  UIControlGestureRecognizer.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 05.04.2023.
//

#include <UIControlGestureRecognizer.h>

namespace NXKit {

UIControlGestureRecognizer::UIControlGestureRecognizer(UIControl* control) :
        control(control)
{ }

void UIControlGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {

}

void UIControlGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {

}

void UIControlGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {

}

void UIControlGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {

}

void UIControlGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    setState(UIGestureRecognizerState::began);
}

void UIControlGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {

}

void UIControlGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    setState(UIGestureRecognizerState::ended);
}

void UIControlGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {

}

}
