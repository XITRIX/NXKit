#include <UIResponder.h>
#include <UIPress.h>
#include <UITouch.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace NXKit {

void UIResponder::touchesBegan(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesBegan(std::move(touches), std::move(event));
}

void UIResponder::touchesMoved(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesMoved(std::move(touches), std::move(event));
}

void UIResponder::touchesEnded(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesEnded(std::move(touches), std::move(event));
}

void UIResponder::touchesCancelled(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    auto next = this->next();
    if (next) next->touchesCancelled(std::move(touches), std::move(event));
}

void UIResponder::pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesBegan(std::move(pressees), std::move(event));
}

void UIResponder::pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesChanged(std::move(pressees), std::move(event));
}

void UIResponder::pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto unhandledPresses = pressees;
    for (const auto& press : pressees) {
        if (!press || press->isHandled()) {
            unhandledPresses.erase(press);
            continue;
        }

        if (!performRegisteredAction(press)) {
            continue;
        }
        unhandledPresses.erase(press);
    }

    if (unhandledPresses.empty()) {
        return;
    }
    auto next = this->next();
    if (next) next->pressesEnded(std::move(unhandledPresses), std::move(event));
}

bool UIResponder::performRegisteredAction(const std::shared_ptr<UIPress>& press) {
    if (!press || press->isHandled()) {
        return false;
    }
    // Match against a snapshot because matchers are application callbacks and
    // may register or remove actions.
    const auto registeredActions = _registeredActions;
    const auto match = std::find_if(
        registeredActions.begin(),
        registeredActions.end(),
        [&press](const UIResponderAction& candidate) {
            return candidate.matches && candidate.matches(press)
                && (!candidate.canPerform || candidate.canPerform());
        }
    );
    if (match == registeredActions.end()) {
        return false;
    }

    const auto resolvedAction = *match;
    press->_isHandled = true;
    if (resolvedAction.isEnabled && isActionDispatchAllowed()) {
        resolvedAction.action.perform();
    }
    return true;
}

bool UIResponder::isActionDispatchAllowed() {
    std::unordered_set<const UIResponder*> visited;
    UIResponder* responder = this;
    std::shared_ptr<UIResponder> retainedResponder;
    while (responder) {
        if (!visited.insert(responder).second || !responder->allowsActionDispatch()) {
            return false;
        }
        retainedResponder = responder->next();
        responder = retainedResponder.get();
    }
    return true;
}

void UIResponder::pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    auto next = this->next();
    if (next) next->pressesCancelled(std::move(pressees), std::move(event));
}

void UIResponder::registerAction(UIResponderAction action) {
    if (action.identifier.empty()) {
        throw std::invalid_argument("UIResponder action identifier cannot be empty");
    }
    if (!action.matches) {
        throw std::invalid_argument("UIResponder action requires an input matcher");
    }

    const auto existing = std::find_if(
        _registeredActions.begin(),
        _registeredActions.end(),
        [&action](const UIResponderAction& candidate) {
            return candidate.identifier == action.identifier;
        }
    );
    if (existing == _registeredActions.end()) {
        _registeredActions.push_back(std::move(action));
    } else {
        *existing = std::move(action);
    }
    std::stable_sort(
        _registeredActions.begin(),
        _registeredActions.end(),
        [](const UIResponderAction& lhs, const UIResponderAction& rhs) {
            return lhs.priority > rhs.priority;
        }
    );
}

void UIResponder::unregisterAction(const std::string& identifier) {
    std::erase_if(_registeredActions, [&identifier](const UIResponderAction& action) {
        return action.identifier == identifier;
    });
}

std::optional<UIResponderAction> UIResponder::registeredAction(
    const std::string& identifier
) const {
    const auto action = std::find_if(
        _registeredActions.begin(),
        _registeredActions.end(),
        [&identifier](const UIResponderAction& candidate) {
            return candidate.identifier == identifier;
        }
    );
    return action == _registeredActions.end()
        ? std::nullopt
        : std::optional<UIResponderAction>(*action);
}

}
