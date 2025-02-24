#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <tools/SharedBase.hpp>

namespace NXKit {

class UITouch;
class UIPress;
class UIPressesEvent;

enum class UIGestureRecognizerState {
    possible,
    began,
    recognized,
    changed,
    ended,
    cancelled,
    failed
};

class UIView;
class UIEvent;
class UIGestureRecognizer;
class UIGestureRecognizerDelegate {
public:
    virtual bool gestureRecognizerShouldRecognizeSimultaneouslyWith(std::shared_ptr<UIGestureRecognizer> gestureRecognizer, std::shared_ptr<UIGestureRecognizer> otherGestureRecognizer) { return false; }
};

class UIGestureRecognizer: public enable_shared_from_this<UIGestureRecognizer> {
public:
    std::weak_ptr<UIGestureRecognizerDelegate> delegate;
    std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged = [](auto self){};

    UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged = [](auto) {});
    virtual ~UIGestureRecognizer();

    bool isEnabled() { return _isEnabled; }
    void setEnabled(bool enabled);

    UIGestureRecognizerState state() { return _state; }
    void setState(UIGestureRecognizerState state);

    std::weak_ptr<UIView> view() { return _view; }

    virtual void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    virtual void pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);

protected:
    UIGestureRecognizerState _state = UIGestureRecognizerState::possible;
    
private:
    bool _isEnabled = true;
    std::weak_ptr<UIView> _view;
    std::vector<std::shared_ptr<UITouch>> _allTouches;

    void _touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    void _touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    void _pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);
    void _pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event);

    bool recognitionCondition();

    void addTouch(std::shared_ptr<UITouch> touch);
    void removeTouch(std::shared_ptr<UITouch> touch);
    void cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();

    friend class UIView;
};

}

