#include <NXResponderAction.h>

#include <UIControl.h>
#include <UIKey.h>
#include <UIPress.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace NXKit {

namespace {

std::string actionIdentifier(NXActionButton button) {
    switch (button) {
        case NXActionButton::a: return "NXKit.responderAction.a";
        case NXActionButton::b: return "NXKit.responderAction.b";
        case NXActionButton::x: return "NXKit.responderAction.x";
        case NXActionButton::y: return "NXKit.responderAction.y";
        case NXActionButton::plus: return "NXKit.responderAction.plus";
        case NXActionButton::minus: return "NXKit.responderAction.minus";
    }
    return {};
}

bool matchesGamepadButton(UIGamepadInputType input, NXActionButton button) {
    switch (button) {
        case NXActionButton::a:
        case NXActionButton::b:
            return false;
        case NXActionButton::x:
#if defined(PLATFORM_SWITCH)
            return input == UIGamepadInputType::buttonY;
#else
            return input == UIGamepadInputType::buttonX;
#endif
        case NXActionButton::y:
#if defined(PLATFORM_SWITCH)
            return input == UIGamepadInputType::buttonX;
#else
            return input == UIGamepadInputType::buttonY;
#endif
        case NXActionButton::plus:
            return input == UIGamepadInputType::buttonStart;
        case NXActionButton::minus:
            return input == UIGamepadInputType::buttonOptions;
    }
    return false;
}

bool matchesKeyboardButton(UIKeyboardHIDUsage keyCode, NXActionButton button) {
    switch (button) {
        case NXActionButton::a:
        case NXActionButton::b:
            return false;
        case NXActionButton::x:
            return keyCode == UIKeyboardHIDUsage::keyboardX;
        case NXActionButton::y:
            return keyCode == UIKeyboardHIDUsage::keyboardY;
        case NXActionButton::plus:
            return keyCode == UIKeyboardHIDUsage::keyboardEqualSign;
        case NXActionButton::minus:
            return keyCode == UIKeyboardHIDUsage::keyboardHyphen;
    }
    return false;
}

bool pressMatchesButton(
    const std::shared_ptr<UIPress>& press,
    NXActionButton button
) {
    if (!press) {
        return false;
    }
    if (button == NXActionButton::a && press->type() == UIPressType::select) {
        return true;
    }
    if (button == NXActionButton::b && press->type() == UIPressType::menu) {
        return true;
    }
    if (const auto gamepadKey = press->gamepadKey()) {
        return matchesGamepadButton(gamepadKey->inputType(), button);
    }
    if (const auto key = press->key()) {
        return matchesKeyboardButton(key->keyCode(), button);
    }
    return false;
}

std::optional<NXResponderAction> actionRegisteredDirectlyOn(
    const std::shared_ptr<UIResponder>& responder,
    NXActionButton button
) {
    if (!responder) {
        return std::nullopt;
    }

    const auto registered = responder->registeredAction(actionIdentifier(button));
    if (registered) {
        return NXResponderAction {
            .button = button,
            .isEnabled = registered->isEnabled,
            .action = registered->action,
        };
    }

    // UIControl::primaryAction predates responder commands. Treat it as the
    // control's local A command until UIControl gains a proper target/action API.
    if (button == NXActionButton::a) {
        if (const auto control = std::dynamic_pointer_cast<UIControl>(responder);
            control && control->primaryAction) {
            auto action = *control->primaryAction;
            if (action.title().empty()) {
                action.setTitle("OK");
            }
            return NXResponderAction {
                .button = button,
                .isEnabled = control->isEnabled(),
                .action = std::move(action),
            };
        }
    }
    return std::nullopt;
}

} // namespace

void NXResponderAction::registerOn(const std::shared_ptr<UIResponder>& responder) const {
    if (!responder) {
        throw std::invalid_argument("NXResponderAction requires a non-null responder");
    }
    responder->registerAction(UIResponderAction {
        .identifier = actionIdentifier(button),
        .isEnabled = isEnabled,
        .action = action,
        .matches = [button = button](const std::shared_ptr<UIPress>& press) {
            return pressMatchesButton(press, button);
        },
    });
}

void NXResponderAction::unregisterFrom(
    const std::shared_ptr<UIResponder>& responder
) const {
    if (responder) {
        responder->unregisterAction(actionIdentifier(button));
    }
}

std::optional<NXResponderAction> NXResponderActionForButton(
    const std::shared_ptr<UIResponder>& responder,
    NXActionButton button
) {
    return actionRegisteredDirectlyOn(responder, button);
}

std::vector<NXResponderAction> NXCollectResponderActions(
    const std::shared_ptr<UIResponder>& firstResponder
) {
    constexpr std::array buttonOrder {
        NXActionButton::plus,
        NXActionButton::minus,
        NXActionButton::x,
        NXActionButton::y,
        NXActionButton::b,
        NXActionButton::a,
    };

    std::vector<std::shared_ptr<UIResponder>> responderChain;
    std::unordered_set<const UIResponder*> visited;
    for (auto responder = firstResponder;
         responder && visited.insert(responder.get()).second;
         responder = responder->next()) {
        responderChain.push_back(responder);
    }

    std::vector<NXResponderAction> result;
    for (const auto button : buttonOrder) {
        for (const auto& responder : responderChain) {
            if (const auto action = actionRegisteredDirectlyOn(responder, button)) {
                auto resolvedAction = *action;
                resolvedAction.isEnabled = resolvedAction.isEnabled
                    && responder->isActionDispatchAllowed();
                result.push_back(std::move(resolvedAction));
                break;
            }
        }
    }

    if (std::none_of(result.begin(), result.end(), [](const NXResponderAction& action) {
            return action.button == NXActionButton::a;
        })) {
        result.push_back(NXResponderAction {
            .button = NXActionButton::a,
            .isEnabled = false,
            .action = UIAction("OK"),
        });
    }
    return result;
}

} // namespace NXKit
