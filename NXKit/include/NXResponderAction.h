#pragma once

#include <UIAction.h>
#include <UIResponder.h>

#include <memory>
#include <optional>
#include <vector>

namespace NXKit {

// Face-button names preserve NXKit's existing label/semantic mapping. Plus and
// minus correspond to SDL's start and back positions; the remaining names map
// directly to normalized SDL gamepad controls. Triggers are included because
// they are physical controller buttons even though SDL reports them as axes.
enum class NXActionButton {
    a,
    b,
    x,
    y,
    plus,
    minus,
    home,
    leftThumbstick,
    rightThumbstick,
    leftShoulder,
    rightShoulder,
    leftTrigger,
    rightTrigger,
    dpadUp,
    dpadDown,
    dpadLeft,
    dpadRight,
    misc1,
    rightPaddle1,
    leftPaddle1,
    rightPaddle2,
    leftPaddle2,
    touchpad,
    misc2,
    misc3,
    misc4,
    misc5,
    misc6,
};

// A console command registered on a responder. The closest available action
// for a button wins, including when that action is disabled. Unavailable
// actions fall through to another action or the next responder.
struct NXResponderAction {
    NXActionButton button = NXActionButton::a;
    bool isEnabled = true;
    UIAction action;
    // Empty preserves the historical one-action-per-button replacement
    // behavior. Semantic fallbacks use distinct identifiers and priorities.
    std::string identifier;
    std::function<bool()> canPerform = []() { return true; };
    int priority = 0;

    void registerOn(const std::shared_ptr<UIResponder>& responder) const;
    void unregisterFrom(const std::shared_ptr<UIResponder>& responder) const;

    bool operator==(const NXResponderAction& other) const {
        return button == other.button
            && isEnabled == other.isEnabled
            && action.title() == other.action.title();
    }
};

[[nodiscard]] std::optional<NXResponderAction> NXResponderActionForButton(
    const std::shared_ptr<UIResponder>& responder,
    NXActionButton button
);

[[nodiscard]] std::vector<NXResponderAction> NXCollectResponderActions(
    const std::shared_ptr<UIResponder>& firstResponder
);

} // namespace NXKit
