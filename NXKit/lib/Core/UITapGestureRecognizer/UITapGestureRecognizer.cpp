//
//  UITapGestureRecognizer.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 20.08.2022.
//

#include "UITapGestureRecognizer.hpp"

namespace NXKit {

#define THRESHOLD 10

void UITapGestureRecognizer::touchesBegan(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesBegan(touches, event);
    if (!trackingTouch) {
        trackingTouch = touches[0];
        startPoint = trackingTouch->locationIn(this->view);
    }
}

void UITapGestureRecognizer::touchesMoved(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesMoved(touches, event);
    if (trackingTouch == touches[0]) {
        Point diff = startPoint - trackingTouch->locationIn(this->view);
        if (abs(diff.x) >= THRESHOLD || abs(diff.y) >= THRESHOLD) {
            setState(UIGestureRecognizerState::FAILED);
            trackingTouch = nullptr;
        }
    }
}

void UITapGestureRecognizer::touchesEnded(std::vector<UITouch*> touches, UIEvent* event) {
    UIGestureRecognizer::touchesEnded(touches, event);
    if (trackingTouch == touches[0]) {
        setState(UIGestureRecognizerState::ENDED);
        trackingTouch = nullptr;
    }
}

}
