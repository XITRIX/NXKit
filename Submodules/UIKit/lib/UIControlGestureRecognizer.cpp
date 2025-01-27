//
//  UIControlGestureRecognizer.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 05.04.2023.
//

#include <UIControlGestureRecognizer.h>
#include <UITouch.h>

namespace NXKit {

UIControlGestureRecognizer::UIControlGestureRecognizer(std::weak_ptr<UIControl> control) :
        control(control)
{ }

void UIControlGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    setState(UIGestureRecognizerState::began);

    if (_touchToTrack == nullptr) {
        _touchToTrack = std::vector<std::shared_ptr<UITouch>>(touches.begin(), touches.end()).front();
        control.lock()->setHighlighted(true);
    }
}

void UIControlGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    std::shared_ptr<UITouch> touch = std::vector<std::shared_ptr<UITouch>>(touches.begin(), touches.end()).front();
    if (_touchToTrack != touch) return;

    auto position = touch->locationIn(control.lock());
    const NXFloat inset = 48;
    auto viewBounds = control.lock()->bounds();
    auto intersects = viewBounds.insetBy({ inset, inset, inset, inset }).contains(position);
    control.lock()->setHighlighted(intersects);
}

void UIControlGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    std::shared_ptr<UITouch> touch = std::vector<std::shared_ptr<UITouch>>(touches.begin(), touches.end()).front();
    if (_touchToTrack != touch) return;
    _touchToTrack = nullptr;

    setState(UIGestureRecognizerState::ended);
    if (control.lock()->isHighlighted()) {
        control.lock()->setHighlighted(false);
        control.lock()->performPrimaryAction();
    }
}

void UIControlGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {

}

void UIControlGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
//    setState(UIGestureRecognizerState::began);
}

void UIControlGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {

}

void UIControlGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
//    setState(UIGestureRecognizerState::ended);
}

void UIControlGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {

}

}
