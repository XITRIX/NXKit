//
//  UITapGestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 20.08.2022.
//

#include <Core/UITapGestureRecognizer/UITapGestureRecognizer.hpp>

namespace NXKit {

#define THRESHOLD 10

void UITapGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        startPoint = trackingTouch->locationIn(this->view.lock());
    }
}

void UITapGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        Point diff = startPoint - trackingTouch->locationIn(this->view.lock());
        if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
            setState(UIGestureRecognizerState::FAILED);
            trackingTouch = nullptr;
        }
    }
}

void UITapGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        setState(UIGestureRecognizerState::ENDED);
        trackingTouch = nullptr;
    }
}

}
