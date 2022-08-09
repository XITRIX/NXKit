//
//  UIGestureRecognizer.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#pragma once

#include "UIEvent.hpp"
#include <functional>

namespace NXKit {

class UITouch;

enum class UIGestureRecognizerState {
    POSSIBLE,
    BEGAN,
    RECOGNIZED,
    CHANGED,
    ENDED,
    CANCELLED,
    FAILED
};

class UIGestureRecognizer {
public:
    bool isEnabled() { return enabled; }
    void setEnabled(bool enabled);

    UIGestureRecognizerState getState() { return state; }
    void setState(UIGestureRecognizerState state);

    virtual void touchesBegan(std::vector<UITouch*> touches, UIEvent* event);
    virtual void touchesMoved(std::vector<UITouch*> touches, UIEvent* event);
    virtual void touchesEnded(std::vector<UITouch*> touches, UIEvent* event);

private:
    bool enabled = true;
    UIGestureRecognizerState state = UIGestureRecognizerState::POSSIBLE;

    std::function<void()> onStateChanged = [](){};
};

}
