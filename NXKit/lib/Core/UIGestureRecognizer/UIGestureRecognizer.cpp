//
//  UIGestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#include <Core/UIGestureRecognizer/UIGestureRecognizer.hpp>
#include <Core/UITouch/UITouch.hpp>

namespace NXKit {

UIGestureRecognizer::~UIGestureRecognizer() {
    for (auto touch: allTouches) {
        touch->hasBeenCancelledByAGestureRecognizer = true;
    }
}

void UIGestureRecognizer::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void UIGestureRecognizer::setState(UIGestureRecognizerState state) {
    if (this->state == state) return;
    this->state = state;
    onStateChanged(state);
    switch (state) {
        case UIGestureRecognizerState::BEGAN:
            cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
            break;
        case UIGestureRecognizerState::RECOGNIZED:
        case UIGestureRecognizerState::ENDED:
            this->state = UIGestureRecognizerState::POSSIBLE;
            break;
        default: break;
    }
}

void UIGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}


bool UIGestureRecognizer::recognitionCondition() {
    return state != UIGestureRecognizerState::FAILED &&
           state != UIGestureRecognizerState::CANCELLED;
}

void UIGestureRecognizer::_touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    bool firstTouch = allTouches.size() == 0;

    for (auto touch : touches)
        addTouch(touch);

    if (!recognitionCondition()) return;

    touchesBegan(touches, event);

    // Check touche's hasBeenCancelledByAGestureRecognizer in case
    // gesture has been deinited by action in touchBegan
    if (touches.back()->hasBeenCancelledByAGestureRecognizer) return;

    if (firstTouch && state == UIGestureRecognizerState::POSSIBLE)
        onStateChanged(state);
}

void UIGestureRecognizer::_touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    if (!recognitionCondition()) return;
    touchesMoved(touches, event);
}

void UIGestureRecognizer::_touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    bool condition = recognitionCondition();

    for (auto touch : touches)
        removeTouch(touch);

    if (condition)
        touchesEnded(touches, event);

    if (allTouches.size() == 0)
        setState(UIGestureRecognizerState::POSSIBLE);
}

void UIGestureRecognizer::_touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    for (auto touch : touches)
        removeTouch(touch);

//    if (!recognitionCondition()) return;
    
    touchesCancelled(touches, event);
}

void UIGestureRecognizer::addTouch(std::shared_ptr<UITouch> touch) {
    allTouches.push_back(touch);
}

void UIGestureRecognizer::removeTouch(std::shared_ptr<UITouch> touch) {
    allTouches.erase(std::remove(allTouches.begin(), allTouches.end(), touch));
    if (allTouches.size() <= 0 && state == UIGestureRecognizerState::FAILED)
        setState(UIGestureRecognizerState::POSSIBLE);
}

void UIGestureRecognizer::cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously() {
    for (auto touch : allTouches) {
        for (auto rec : touch->gestureRecognizers) {
            if (rec.get() != this && (!rec->delegate || !rec->delegate->gestureRecognizerShouldRecognizeSimultaneouslyWith(rec, shared_from_this()))) {
                rec->setState(UIGestureRecognizerState::CANCELLED);
                rec->touchesCancelled({touch}, nullptr);
            }
        }
    }
}

}
