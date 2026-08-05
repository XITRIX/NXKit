#include <NXControllerIconResolver.h>

#include "NXControllerIconProvider.h"

#include <CGImage.h>
#include <SkiaCtx.h>
#include <UIImage.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace NXKit;

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (condition) {
        return;
    }
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
}

class TestResponder final : public UIResponder {
public:
    std::shared_ptr<UIResponder> next() override { return nullptr; }
};

} // namespace

int main() {
#if defined(PLATFORM_SWITCH)
    SkiaCtx::_main = MakeSkiaCtx();
    const auto switchProvider = NXMakePlatformControllerIconProvider();
    constexpr std::array nativeSwitchButtons {
        NXActionButton::a,
        NXActionButton::b,
        NXActionButton::x,
        NXActionButton::y,
        NXActionButton::plus,
        NXActionButton::minus,
        NXActionButton::leftThumbstick,
        NXActionButton::rightThumbstick,
        NXActionButton::leftShoulder,
        NXActionButton::rightShoulder,
        NXActionButton::leftTrigger,
        NXActionButton::rightTrigger,
        NXActionButton::dpadUp,
        NXActionButton::dpadDown,
        NXActionButton::dpadLeft,
        NXActionButton::dpadRight,
    };
    for (const auto button : nativeSwitchButtons) {
        expect(
            switchProvider->iconForButton(
                button,
                NXControllerType::nintendoSwitch,
                24,
                1
            ) != nullptr,
            "the Switch provider resolves NintendoExt artwork without fallback"
        );
    }
#endif

    auto& resolver = NXControllerIconResolver::shared();
    constexpr std::array controllerTypes {
        NXControllerType::generic,
        NXControllerType::xbox,
        NXControllerType::playStation,
        NXControllerType::nintendoSwitch,
    };
    constexpr std::array buttons {
        NXActionButton::a,
        NXActionButton::b,
        NXActionButton::x,
        NXActionButton::y,
        NXActionButton::plus,
        NXActionButton::minus,
        NXActionButton::home,
        NXActionButton::leftThumbstick,
        NXActionButton::rightThumbstick,
        NXActionButton::leftShoulder,
        NXActionButton::rightShoulder,
        NXActionButton::leftTrigger,
        NXActionButton::rightTrigger,
        NXActionButton::dpadUp,
        NXActionButton::dpadDown,
        NXActionButton::dpadLeft,
        NXActionButton::dpadRight,
        NXActionButton::misc1,
        NXActionButton::rightPaddle1,
        NXActionButton::leftPaddle1,
        NXActionButton::rightPaddle2,
        NXActionButton::leftPaddle2,
        NXActionButton::touchpad,
        NXActionButton::misc2,
        NXActionButton::misc3,
        NXActionButton::misc4,
        NXActionButton::misc5,
        NXActionButton::misc6,
    };

    for (const auto controllerType : controllerTypes) {
        for (const auto button : buttons) {
            const auto icon = resolver.iconForButton(button, controllerType, 24);
            expect(icon != nullptr, "every controller family and button resolves an icon");
            if (!icon) {
                continue;
            }
            expect(
                icon->cgImage() && icon->cgImage()->pointee,
                "resolved controller icons contain raster image data"
            );
            expect(
                std::abs(icon->size().width - 24) < 1
                    && std::abs(icon->size().height - 24) < 1,
                "resolved controller icons preserve the requested logical size"
            );
        }
    }

    auto responder = std::make_shared<TestResponder>();
    for (const auto button : buttons) {
        NXResponderAction {
            .button = button,
            .action = UIAction("Test"),
        }.registerOn(responder);
    }
    const auto collectedActions = NXCollectResponderActions(responder);
    expect(
        collectedActions.size() == buttons.size(),
        "the responder collector includes the complete action-button set"
    );
    for (const auto button : buttons) {
        expect(
            std::any_of(
                collectedActions.begin(),
                collectedActions.end(),
                [button](const NXResponderAction& action) {
                    return action.button == button;
                }
            ),
            "every registered controller button is discoverable"
        );
    }

    bool rejectedZeroSize = false;
    try {
        (void)resolver.iconForButton(
            NXActionButton::a,
            NXControllerType::generic,
            0
        );
    } catch (const std::invalid_argument&) {
        rejectedZeroSize = true;
    }
    expect(rejectedZeroSize, "zero-sized controller icon requests are rejected");

    bool rejectedNonFiniteSize = false;
    try {
        (void)resolver.iconForButton(
            NXActionButton::a,
            NXControllerType::generic,
            std::numeric_limits<NXFloat>::infinity()
        );
    } catch (const std::invalid_argument&) {
        rejectedNonFiniteSize = true;
    }
    expect(
        rejectedNonFiniteSize,
        "non-finite controller icon requests are rejected"
    );

    if (failures == 0) {
        std::cout << "NXControllerIconResolver tests passed\n";
    }
#if defined(PLATFORM_SWITCH)
    SkiaCtx::_main.reset();
#endif
    return failures == 0 ? 0 : 1;
}
