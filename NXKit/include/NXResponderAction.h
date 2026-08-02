#pragma once

#include <UIAction.h>
#include <UIResponder.h>

#include <memory>
#include <optional>
#include <vector>

namespace NXKit {

enum class NXActionButton {
    a,
    b,
    x,
    y,
    plus,
    minus,
};

// A console command registered on a responder. The closest responder action
// for a button wins, including when that action is disabled.
struct NXResponderAction {
    NXActionButton button = NXActionButton::a;
    bool isEnabled = true;
    UIAction action;

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
