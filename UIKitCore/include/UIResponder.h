#pragma once

#include <UIAction.h>

#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace NXKit {

class UITouch;
class UIPress;
class UIEvent;
class UIPressesEvent;

// An extensible responder command. UI-specific layers provide stable identifiers
// and input matchers while UIResponder owns ordering and chain dispatch.
struct UIResponderAction {
    std::string identifier;
    bool isEnabled = true;
    UIAction action;
    std::function<bool(const std::shared_ptr<UIPress>&)> matches;
};

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

    void registerAction(UIResponderAction action);
    void unregisterAction(const std::string& identifier);
    [[nodiscard]] std::optional<UIResponderAction> registeredAction(
        const std::string& identifier
    ) const;
    [[nodiscard]] const std::vector<UIResponderAction>& registeredActions() const {
        return _registeredActions;
    }

    // Containers can temporarily gate actions originating anywhere below them
    // in the responder chain without changing the responders' own enabled state.
    virtual bool allowsActionDispatch() const { return true; }
    [[nodiscard]] bool isActionDispatchAllowed();

protected:
    // Returns true for both enabled and disabled matches. A disabled local
    // action still owns the input and prevents parent fallback.
    bool performRegisteredAction(const std::shared_ptr<UIPress>& press);

private:
    std::vector<UIResponderAction> _registeredActions;
};

}
