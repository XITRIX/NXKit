//
//  UIGestureRecognizer.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.07.2022.
//

#pragma once

#include <Core/UIEvent/UIEvent.hpp>
#include <Core/Utils/NotificationEvent.hpp>
#include <Core/Utils/SharedBase/SharedBase.hpp>
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
    virtual bool gestureRecognizerShouldRecognizeSimultaneouslyWith(std::shared_ptr<UIGestureRecognizer> gestureRecognizer, std::shared_ptr<UIGestureRecognizer> otherGestureRecognizer) { return false; }
};

class UIGestureRecognizer: public enable_shared_from_base<UIGestureRecognizer> {
public:
    std::weak_ptr<UIView> view;
    std::function<void(UIGestureRecognizerState)> onStateChanged = [](auto state){};
    std::shared_ptr<UIGestureRecognizerDelegate> delegate;

    virtual ~UIGestureRecognizer();

    bool isEnabled() { return enabled; }
    void setEnabled(bool enabled);

    UIGestureRecognizerState getState() { return state; }
    void setState(UIGestureRecognizerState state);

    virtual void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

private:
    friend class UIWindow;

    bool enabled = true;
    std::vector<std::shared_ptr<UITouch>> allTouches;
    UIGestureRecognizerState state = UIGestureRecognizerState::POSSIBLE;

    void _touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    bool recognitionCondition();

    void addTouch(std::shared_ptr<UITouch> touch);
    void removeTouch(std::shared_ptr<UITouch> touch);
    void cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
};

}
