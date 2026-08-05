#include <NXResponderAction.h>

#include <UIControl.h>
#include <UIKey.h>
#include <UIPress.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

namespace NXKit {

namespace {

std::string_view actionButtonName(NXActionButton button) {
    switch (button) {
        case NXActionButton::a: return "a";
        case NXActionButton::b: return "b";
        case NXActionButton::x: return "x";
        case NXActionButton::y: return "y";
        case NXActionButton::plus: return "plus";
        case NXActionButton::minus: return "minus";
        case NXActionButton::home: return "home";
        case NXActionButton::leftThumbstick: return "leftThumbstick";
        case NXActionButton::rightThumbstick: return "rightThumbstick";
        case NXActionButton::leftShoulder: return "leftShoulder";
        case NXActionButton::rightShoulder: return "rightShoulder";
        case NXActionButton::leftTrigger: return "leftTrigger";
        case NXActionButton::rightTrigger: return "rightTrigger";
        case NXActionButton::dpadUp: return "dpadUp";
        case NXActionButton::dpadDown: return "dpadDown";
        case NXActionButton::dpadLeft: return "dpadLeft";
        case NXActionButton::dpadRight: return "dpadRight";
        case NXActionButton::misc1: return "misc1";
        case NXActionButton::rightPaddle1: return "rightPaddle1";
        case NXActionButton::leftPaddle1: return "leftPaddle1";
        case NXActionButton::rightPaddle2: return "rightPaddle2";
        case NXActionButton::leftPaddle2: return "leftPaddle2";
        case NXActionButton::touchpad: return "touchpad";
        case NXActionButton::misc2: return "misc2";
        case NXActionButton::misc3: return "misc3";
        case NXActionButton::misc4: return "misc4";
        case NXActionButton::misc5: return "misc5";
        case NXActionButton::misc6: return "misc6";
    }
    return {};
}

std::string actionIdentifier(NXActionButton button) {
    return "NXKit.responderAction." + std::string(actionButtonName(button));
}

std::string actionInputIdentifier(NXActionButton button) {
    if (button == NXActionButton::a) {
        return UIResponderActionInputSelect;
    }
    if (button == NXActionButton::b) {
        return UIResponderActionInputMenu;
    }
    return "NXKit.responderActionInput."
        + std::string(actionButtonName(button));
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
        case NXActionButton::home:
            return input == UIGamepadInputType::buttonGuide;
        case NXActionButton::leftThumbstick:
            return input == UIGamepadInputType::leftThumbstickButton;
        case NXActionButton::rightThumbstick:
            return input == UIGamepadInputType::rightThumbstickButton;
        case NXActionButton::leftShoulder:
            return input == UIGamepadInputType::leftShoulder;
        case NXActionButton::rightShoulder:
            return input == UIGamepadInputType::rightShoulder;
        case NXActionButton::leftTrigger:
            return input == UIGamepadInputType::leftTrigger;
        case NXActionButton::rightTrigger:
            return input == UIGamepadInputType::rightTrigger;
        case NXActionButton::dpadUp:
            return input == UIGamepadInputType::up;
        case NXActionButton::dpadDown:
            return input == UIGamepadInputType::down;
        case NXActionButton::dpadLeft:
            return input == UIGamepadInputType::left;
        case NXActionButton::dpadRight:
            return input == UIGamepadInputType::right;
        case NXActionButton::misc1:
            return input == UIGamepadInputType::misc1;
        case NXActionButton::rightPaddle1:
            return input == UIGamepadInputType::rightPaddle1;
        case NXActionButton::leftPaddle1:
            return input == UIGamepadInputType::leftPaddle1;
        case NXActionButton::rightPaddle2:
            return input == UIGamepadInputType::rightPaddle2;
        case NXActionButton::leftPaddle2:
            return input == UIGamepadInputType::leftPaddle2;
        case NXActionButton::touchpad:
            return input == UIGamepadInputType::touchpad;
        case NXActionButton::misc2:
            return input == UIGamepadInputType::misc2;
        case NXActionButton::misc3:
            return input == UIGamepadInputType::misc3;
        case NXActionButton::misc4:
            return input == UIGamepadInputType::misc4;
        case NXActionButton::misc5:
            return input == UIGamepadInputType::misc5;
        case NXActionButton::misc6:
            return input == UIGamepadInputType::misc6;
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
        case NXActionButton::dpadUp:
            return keyCode == UIKeyboardHIDUsage::keyboardUpArrow;
        case NXActionButton::dpadDown:
            return keyCode == UIKeyboardHIDUsage::keyboardDownArrow;
        case NXActionButton::dpadLeft:
            return keyCode == UIKeyboardHIDUsage::keyboardLeftArrow;
        case NXActionButton::dpadRight:
            return keyCode == UIKeyboardHIDUsage::keyboardRightArrow;
        case NXActionButton::home:
        case NXActionButton::leftThumbstick:
        case NXActionButton::rightThumbstick:
        case NXActionButton::leftShoulder:
        case NXActionButton::rightShoulder:
        case NXActionButton::leftTrigger:
        case NXActionButton::rightTrigger:
        case NXActionButton::misc1:
        case NXActionButton::rightPaddle1:
        case NXActionButton::leftPaddle1:
        case NXActionButton::rightPaddle2:
        case NXActionButton::leftPaddle2:
        case NXActionButton::touchpad:
        case NXActionButton::misc2:
        case NXActionButton::misc3:
        case NXActionButton::misc4:
        case NXActionButton::misc5:
        case NXActionButton::misc6:
            return false;
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

    // Work from a snapshot because availability predicates are application
    // callbacks and may mutate responder registration.
    const auto registeredActions = responder->registeredActions();
    const auto registered = std::find_if(
        registeredActions.begin(),
        registeredActions.end(),
        [inputIdentifier = actionInputIdentifier(button)](
            const UIResponderAction& candidate
        ) {
            return candidate.inputIdentifier == inputIdentifier
                && (!candidate.canPerform || candidate.canPerform());
        }
    );
    if (registered != registeredActions.end()) {
        return NXResponderAction {
            .button = button,
            .isEnabled = registered->isEnabled,
            .action = registered->action,
            .identifier = registered->identifier,
            .canPerform = registered->canPerform,
            .priority = registered->priority,
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
        .identifier = identifier.empty() ? actionIdentifier(button) : identifier,
        .inputIdentifier = actionInputIdentifier(button),
        .isEnabled = isEnabled,
        .action = action,
        .matches = [button = button](const std::shared_ptr<UIPress>& press) {
            return pressMatchesButton(press, button);
        },
        .canPerform = canPerform,
        .priority = priority,
    });
}

void NXResponderAction::unregisterFrom(
    const std::shared_ptr<UIResponder>& responder
) const {
    if (responder) {
        responder->unregisterAction(
            identifier.empty() ? actionIdentifier(button) : identifier
        );
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
        NXActionButton::home,
        NXActionButton::misc1,
        NXActionButton::touchpad,
        NXActionButton::leftShoulder,
        NXActionButton::rightShoulder,
        NXActionButton::leftTrigger,
        NXActionButton::rightTrigger,
        NXActionButton::leftThumbstick,
        NXActionButton::rightThumbstick,
        NXActionButton::dpadUp,
        NXActionButton::dpadDown,
        NXActionButton::dpadLeft,
        NXActionButton::dpadRight,
        NXActionButton::rightPaddle1,
        NXActionButton::leftPaddle1,
        NXActionButton::rightPaddle2,
        NXActionButton::leftPaddle2,
        NXActionButton::misc2,
        NXActionButton::misc3,
        NXActionButton::misc4,
        NXActionButton::misc5,
        NXActionButton::misc6,
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
