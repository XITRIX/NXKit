//
//  UIGestureRecognizer.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#pragma once

#include <Core/UIEvent/UIEvent.hpp>
#include <Core/Utils/NotificationEvent.hpp>
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

class UIView;
class UIGestureRecognizer;
class UIGestureRecognizerDelegate {
public:
    virtual bool gestureRecognizerShouldRecognizeSimultaneouslyWith(UIGestureRecognizer* gestureRecognizer, UIGestureRecognizer* otherGestureRecognizer) { return false; }
};

class UIGestureRecognizer {
public:
    UIView* view;
    std::function<void(UIGestureRecognizerState)> onStateChanged = [](auto state){};
    UIGestureRecognizerDelegate* delegate = nullptr;

    virtual ~UIGestureRecognizer() {}

    bool isEnabled() { return enabled; }
    void setEnabled(bool enabled);

    UIGestureRecognizerState getState() { return state; }
    void setState(UIGestureRecognizerState state);

    virtual void touchesBegan(std::vector<UITouch*> touches, UIEvent* event);
    virtual void touchesMoved(std::vector<UITouch*> touches, UIEvent* event);
    virtual void touchesEnded(std::vector<UITouch*> touches, UIEvent* event);
    virtual void touchesCancelled(std::vector<UITouch*> touches, UIEvent* event);

private:
    friend class UIWindow;

    bool enabled = true;
    std::vector<UITouch*> allTouches;
    UIGestureRecognizerState state = UIGestureRecognizerState::POSSIBLE;

    void _touchesBegan(std::vector<UITouch*> touches, UIEvent* event);
    void _touchesMoved(std::vector<UITouch*> touches, UIEvent* event);
    void _touchesEnded(std::vector<UITouch*> touches, UIEvent* event);
    void _touchesCancelled(std::vector<UITouch*> touches, UIEvent* event);

    bool recognitionCondition();

    void addTouch(UITouch* touch);
    void removeTouch(UITouch* touch);
    void cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
};

}
