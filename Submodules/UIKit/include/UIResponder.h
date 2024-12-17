#pragma once

#include <memory>
#include <set>

namespace NXKit {

class UITouch;
class UIPress;
class UIEvent;
class UIPressesEvent;
class UIResponder {
public:
    virtual ~UIResponder() = default;
    virtual std::shared_ptr<UIResponder> next() = 0;

    virtual void touchesBegan(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesMoved(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesEnded(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);
    virtual void touchesCancelled(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event);

    virtual void pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event);
    virtual void pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event);
};

}
