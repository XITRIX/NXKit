#include <NXControllerIconResolver.h>

#include "NXControllerIconProvider.h"

#include <CGImage.h>
#include <UIImage.h>
#include <SkiaCtx.h>

#include <SDL3/SDL.h>

#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkImage.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathBuilder.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <string_view>

namespace NXKit {

namespace {

enum class FallbackGlyph {
    label,
    cross,
    circle,
    square,
    triangle,
    plus,
    minus,
    home,
    dpadUp,
    dpadDown,
    dpadLeft,
    dpadRight,
    touchpad,
};

struct FallbackIcon {
    FallbackGlyph glyph = FallbackGlyph::label;
    std::string_view label;
};

NXControllerType controllerTypeForSDLType(SDL_GamepadType type) {
    switch (type) {
        case SDL_GAMEPAD_TYPE_XBOX360:
        case SDL_GAMEPAD_TYPE_XBOXONE:
            return NXControllerType::xbox;
        case SDL_GAMEPAD_TYPE_PS3:
        case SDL_GAMEPAD_TYPE_PS4:
        case SDL_GAMEPAD_TYPE_PS5:
            return NXControllerType::playStation;
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
        case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        case SDL_GAMEPAD_TYPE_GAMECUBE:
            return NXControllerType::nintendoSwitch;
        case SDL_GAMEPAD_TYPE_UNKNOWN:
        case SDL_GAMEPAD_TYPE_STANDARD:
        case SDL_GAMEPAD_TYPE_COUNT:
            return NXControllerType::generic;
    }
    return NXControllerType::generic;
}

FallbackIcon fallbackIconForButton(
    NXActionButton button,
    NXControllerType controllerType
) {
    if (controllerType == NXControllerType::playStation) {
        switch (button) {
            case NXActionButton::a: return {FallbackGlyph::cross, {}};
            case NXActionButton::b: return {FallbackGlyph::circle, {}};
            case NXActionButton::x: return {FallbackGlyph::square, {}};
            case NXActionButton::y: return {FallbackGlyph::triangle, {}};
            default: break;
        }
    }

    // SDL names face buttons by position. On non-Switch platforms a Nintendo
    // controller therefore exposes B/A/Y/X at the A/B/X/Y positions.
    if (controllerType == NXControllerType::nintendoSwitch) {
#if defined(PLATFORM_SWITCH)
        switch (button) {
            case NXActionButton::a: return {FallbackGlyph::label, "A"};
            case NXActionButton::b: return {FallbackGlyph::label, "B"};
            case NXActionButton::x: return {FallbackGlyph::label, "X"};
            case NXActionButton::y: return {FallbackGlyph::label, "Y"};
            default: break;
        }
#else
        switch (button) {
            case NXActionButton::a: return {FallbackGlyph::label, "B"};
            case NXActionButton::b: return {FallbackGlyph::label, "A"};
            case NXActionButton::x: return {FallbackGlyph::label, "Y"};
            case NXActionButton::y: return {FallbackGlyph::label, "X"};
            default: break;
        }
#endif
    }

    switch (button) {
        case NXActionButton::a: return {FallbackGlyph::label, "A"};
        case NXActionButton::b: return {FallbackGlyph::label, "B"};
        case NXActionButton::x: return {FallbackGlyph::label, "X"};
        case NXActionButton::y: return {FallbackGlyph::label, "Y"};
        case NXActionButton::plus: return {FallbackGlyph::plus, {}};
        case NXActionButton::minus: return {FallbackGlyph::minus, {}};
        case NXActionButton::home: return {FallbackGlyph::home, {}};
        case NXActionButton::leftThumbstick:
            return {FallbackGlyph::label,
                    controllerType == NXControllerType::playStation
                        ? "L3" : "LS"};
        case NXActionButton::rightThumbstick:
            return {FallbackGlyph::label,
                    controllerType == NXControllerType::playStation
                        ? "R3" : "RS"};
        case NXActionButton::leftShoulder:
            if (controllerType == NXControllerType::xbox) {
                return {FallbackGlyph::label, "LB"};
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return {FallbackGlyph::label, "L"};
            }
            return {FallbackGlyph::label, "L1"};
        case NXActionButton::rightShoulder:
            if (controllerType == NXControllerType::xbox) {
                return {FallbackGlyph::label, "RB"};
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return {FallbackGlyph::label, "R"};
            }
            return {FallbackGlyph::label, "R1"};
        case NXActionButton::leftTrigger:
            if (controllerType == NXControllerType::xbox) {
                return {FallbackGlyph::label, "LT"};
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return {FallbackGlyph::label, "ZL"};
            }
            return {FallbackGlyph::label, "L2"};
        case NXActionButton::rightTrigger:
            if (controllerType == NXControllerType::xbox) {
                return {FallbackGlyph::label, "RT"};
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return {FallbackGlyph::label, "ZR"};
            }
            return {FallbackGlyph::label, "R2"};
        case NXActionButton::dpadUp: return {FallbackGlyph::dpadUp, {}};
        case NXActionButton::dpadDown: return {FallbackGlyph::dpadDown, {}};
        case NXActionButton::dpadLeft: return {FallbackGlyph::dpadLeft, {}};
        case NXActionButton::dpadRight: return {FallbackGlyph::dpadRight, {}};
        case NXActionButton::misc1: return {FallbackGlyph::label, "M1"};
        case NXActionButton::rightPaddle1:
            return {FallbackGlyph::label, "P1"};
        case NXActionButton::leftPaddle1:
            return {FallbackGlyph::label, "P3"};
        case NXActionButton::rightPaddle2:
            return {FallbackGlyph::label, "P2"};
        case NXActionButton::leftPaddle2:
            return {FallbackGlyph::label, "P4"};
        case NXActionButton::touchpad:
            return {FallbackGlyph::touchpad, {}};
        case NXActionButton::misc2: return {FallbackGlyph::label, "M2"};
        case NXActionButton::misc3: return {FallbackGlyph::label, "M3"};
        case NXActionButton::misc4: return {FallbackGlyph::label, "M4"};
        case NXActionButton::misc5: return {FallbackGlyph::label, "M5"};
        case NXActionButton::misc6: return {FallbackGlyph::label, "M6"};
    }
    return {FallbackGlyph::label, "?"};
}

void drawVectorCharacter(
    SkCanvas& canvas,
    char character,
    NXFloat left,
    NXFloat top,
    NXFloat width,
    NXFloat height,
    const SkPaint& paint
) {
    const auto x = [left, width](NXFloat value) {
        return left + width * value;
    };
    const auto y = [top, height](NXFloat value) {
        return top + height * value;
    };

    SkPathBuilder path;
    switch (character) {
        case 'A':
            path.moveTo(x(0), y(1));
            path.lineTo(x(0.5f), y(0));
            path.lineTo(x(1), y(1));
            path.moveTo(x(0.2f), y(0.62f));
            path.lineTo(x(0.8f), y(0.62f));
            break;
        case 'B':
            path.moveTo(x(0), y(0));
            path.lineTo(x(0), y(1));
            path.moveTo(x(0), y(0));
            path.cubicTo(x(1), y(0), x(1), y(0.5f), x(0), y(0.5f));
            path.moveTo(x(0), y(0.5f));
            path.cubicTo(x(1), y(0.5f), x(1), y(1), x(0), y(1));
            break;
        case 'L':
            path.moveTo(x(0), y(0));
            path.lineTo(x(0), y(1));
            path.lineTo(x(1), y(1));
            break;
        case 'M':
            path.moveTo(x(0), y(1));
            path.lineTo(x(0), y(0));
            path.lineTo(x(0.5f), y(0.55f));
            path.lineTo(x(1), y(0));
            path.lineTo(x(1), y(1));
            break;
        case 'P':
            path.moveTo(x(0), y(1));
            path.lineTo(x(0), y(0));
            path.cubicTo(x(1), y(0), x(1), y(0.55f), x(0), y(0.55f));
            break;
        case 'R':
            path.moveTo(x(0), y(1));
            path.lineTo(x(0), y(0));
            path.cubicTo(x(1), y(0), x(1), y(0.5f), x(0), y(0.5f));
            path.moveTo(x(0.48f), y(0.5f));
            path.lineTo(x(1), y(1));
            break;
        case 'S':
            path.moveTo(x(1), y(0));
            path.lineTo(x(0), y(0));
            path.lineTo(x(0), y(0.5f));
            path.lineTo(x(1), y(0.5f));
            path.lineTo(x(1), y(1));
            path.lineTo(x(0), y(1));
            break;
        case 'T':
            path.moveTo(x(0), y(0));
            path.lineTo(x(1), y(0));
            path.moveTo(x(0.5f), y(0));
            path.lineTo(x(0.5f), y(1));
            break;
        case 'X':
            path.moveTo(x(0), y(0));
            path.lineTo(x(1), y(1));
            path.moveTo(x(1), y(0));
            path.lineTo(x(0), y(1));
            break;
        case 'Y':
            path.moveTo(x(0), y(0));
            path.lineTo(x(0.5f), y(0.5f));
            path.lineTo(x(1), y(0));
            path.moveTo(x(0.5f), y(0.5f));
            path.lineTo(x(0.5f), y(1));
            break;
        case 'Z':
        case '2':
            path.moveTo(x(0), y(0));
            path.lineTo(x(1), y(0));
            path.lineTo(x(0), y(1));
            path.lineTo(x(1), y(1));
            break;
        case '1':
            path.moveTo(x(0.2f), y(0.2f));
            path.lineTo(x(0.55f), y(0));
            path.lineTo(x(0.55f), y(1));
            path.moveTo(x(0.15f), y(1));
            path.lineTo(x(0.95f), y(1));
            break;
        case '3':
            path.moveTo(x(0), y(0));
            path.lineTo(x(1), y(0));
            path.lineTo(x(1), y(1));
            path.lineTo(x(0), y(1));
            path.moveTo(x(0.2f), y(0.5f));
            path.lineTo(x(1), y(0.5f));
            break;
        case '4':
            path.moveTo(x(0), y(0));
            path.lineTo(x(0), y(0.55f));
            path.lineTo(x(1), y(0.55f));
            path.moveTo(x(1), y(0));
            path.lineTo(x(1), y(1));
            break;
        case '5':
            path.moveTo(x(1), y(0));
            path.lineTo(x(0), y(0));
            path.lineTo(x(0), y(0.5f));
            path.lineTo(x(1), y(0.5f));
            path.lineTo(x(1), y(1));
            path.lineTo(x(0), y(1));
            break;
        case '6':
            path.moveTo(x(1), y(0));
            path.lineTo(x(0), y(0));
            path.lineTo(x(0), y(1));
            path.lineTo(x(1), y(1));
            path.lineTo(x(1), y(0.5f));
            path.lineTo(x(0), y(0.5f));
            break;
        default:
            path.moveTo(x(0), y(0));
            path.lineTo(x(1), y(0));
            path.lineTo(x(1), y(1));
            path.lineTo(x(0), y(1));
            path.close();
            break;
    }
    canvas.drawPath(path.detach(), paint);
}

void drawVectorLabel(
    SkCanvas& canvas,
    std::string_view label,
    NXFloat unit,
    const SkPaint& paint
) {
    const auto characterWidth = (label.size() == 1 ? 6.5f : 5.0f) * unit;
    const auto spacing = 1.5f * unit;
    const auto totalWidth = characterWidth * static_cast<NXFloat>(label.size())
        + spacing * static_cast<NXFloat>(label.empty() ? 0 : label.size() - 1);
    auto left = 12.0f * unit - totalWidth / 2.0f;
    for (const auto character : label) {
        drawVectorCharacter(
            canvas,
            character,
            left,
            8.0f * unit,
            characterWidth,
            8.0f * unit,
            paint
        );
        left += characterWidth + spacing;
    }
}

void drawFallbackGlyph(SkCanvas& canvas, NXFloat size, FallbackIcon icon) {
    const auto unit = size / 24.0f;
    const auto scaled = [unit](NXFloat value) { return value * unit; };

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(scaled(1.8f));
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);

    canvas.drawCircle(scaled(12), scaled(12), scaled(10.2f), paint);

    SkPathBuilder path;
    switch (icon.glyph) {
        case FallbackGlyph::label:
            drawVectorLabel(canvas, icon.label, unit, paint);
            break;
        case FallbackGlyph::cross:
            path.moveTo(scaled(8), scaled(8));
            path.lineTo(scaled(16), scaled(16));
            path.moveTo(scaled(16), scaled(8));
            path.lineTo(scaled(8), scaled(16));
            canvas.drawPath(path.detach(), paint);
            break;
        case FallbackGlyph::circle:
            canvas.drawCircle(scaled(12), scaled(12), scaled(4.3f), paint);
            break;
        case FallbackGlyph::square:
            canvas.drawRect(
                SkRect::MakeLTRB(
                    scaled(8), scaled(8), scaled(16), scaled(16)
                ),
                paint
            );
            break;
        case FallbackGlyph::triangle:
            path.moveTo(scaled(12), scaled(7.5f));
            path.lineTo(scaled(17), scaled(16));
            path.lineTo(scaled(7), scaled(16));
            path.close();
            canvas.drawPath(path.detach(), paint);
            break;
        case FallbackGlyph::plus:
            path.moveTo(scaled(7.5f), scaled(12));
            path.lineTo(scaled(16.5f), scaled(12));
            path.moveTo(scaled(12), scaled(7.5f));
            path.lineTo(scaled(12), scaled(16.5f));
            canvas.drawPath(path.detach(), paint);
            break;
        case FallbackGlyph::minus:
            canvas.drawLine(
                scaled(7.5f),
                scaled(12),
                scaled(16.5f),
                scaled(12),
                paint
            );
            break;
        case FallbackGlyph::home:
            path.moveTo(scaled(7), scaled(12));
            path.lineTo(scaled(12), scaled(7.5f));
            path.lineTo(scaled(17), scaled(12));
            path.moveTo(scaled(8.5f), scaled(10.8f));
            path.lineTo(scaled(8.5f), scaled(16.5f));
            path.lineTo(scaled(15.5f), scaled(16.5f));
            path.lineTo(scaled(15.5f), scaled(10.8f));
            canvas.drawPath(path.detach(), paint);
            break;
        case FallbackGlyph::dpadUp:
        case FallbackGlyph::dpadDown:
        case FallbackGlyph::dpadLeft:
        case FallbackGlyph::dpadRight: {
            const bool vertical = icon.glyph == FallbackGlyph::dpadUp
                || icon.glyph == FallbackGlyph::dpadDown;
            const bool positive = icon.glyph == FallbackGlyph::dpadDown
                || icon.glyph == FallbackGlyph::dpadRight;
            if (vertical) {
                const auto tip = scaled(positive ? 16.5f : 7.5f);
                const auto base = scaled(positive ? 8.5f : 15.5f);
                path.moveTo(scaled(12), tip);
                path.lineTo(scaled(8), base);
                path.lineTo(scaled(16), base);
            } else {
                const auto tip = scaled(positive ? 16.5f : 7.5f);
                const auto base = scaled(positive ? 8.5f : 15.5f);
                path.moveTo(tip, scaled(12));
                path.lineTo(base, scaled(8));
                path.lineTo(base, scaled(16));
            }
            path.close();
            canvas.drawPath(path.detach(), paint);
            break;
        }
        case FallbackGlyph::touchpad:
            canvas.drawRoundRect(
                SkRect::MakeLTRB(
                    scaled(6.8f), scaled(8), scaled(17.2f), scaled(16)
                ),
                scaled(1.5f),
                scaled(1.5f),
                paint
            );
            canvas.drawLine(
                scaled(9), scaled(13.5f), scaled(15), scaled(13.5f), paint
            );
            break;
    }
}

std::shared_ptr<UIImage> makeFallbackIcon(
    NXActionButton button,
    NXControllerType controllerType,
    NXFloat pointSize,
    NXFloat scale
) {
    const auto icon = fallbackIconForButton(button, controllerType);
    return NXMakeControllerIconImage(
        pointSize,
        scale,
        [icon](SkCanvas& canvas, NXFloat size) {
            drawFallbackGlyph(canvas, size, icon);
        }
    );
}

} // namespace

class NXControllerIconResolver::Impl {
public:
    Impl() : platformProvider(NXMakePlatformControllerIconProvider()) {}

    ~Impl() {
        std::lock_guard lock(watchMutex);
        if (watchInstalled) {
            SDL_RemoveEventWatch(handleEvent, this);
        }
    }

    NXControllerType currentControllerType() {
#if defined(PLATFORM_SWITCH)
        return NXControllerType::nintendoSwitch;
#else
        ensureEventWatch();
        if (activeGamepadID.load() == 0) {
            selectFirstConnectedGamepad();
        }
        const auto detectedControllerType = activeControllerType.load();
        return platformProvider
            ? platformProvider->resolvedControllerType(detectedControllerType)
            : detectedControllerType;
#endif
    }

    std::unique_ptr<NXControllerIconProvider> platformProvider;

private:
    static bool SDLCALL handleEvent(void* userdata, SDL_Event* event) {
        auto* self = static_cast<Impl*>(userdata);
        if (!self || !event) {
            return true;
        }

        switch (event->type) {
            case SDL_EVENT_GAMEPAD_ADDED:
                if (self->activeGamepadID.load() == 0) {
                    self->selectGamepad(event->gdevice.which);
                }
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                if (self->activeGamepadID.load() == event->gdevice.which) {
                    self->activeGamepadID = 0;
                    self->activeControllerType = NXControllerType::generic;
                    self->selectFirstConnectedGamepad();
                }
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                self->selectGamepad(event->gbutton.which);
                break;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                // Ignore ordinary stick noise so an idle controller cannot
                // steal the legend from the controller the player is using.
                if (std::abs(static_cast<int>(event->gaxis.value)) > 8000) {
                    self->selectGamepad(event->gaxis.which);
                }
                break;
            default:
                break;
        }
        return true;
    }

    void ensureEventWatch() {
        std::lock_guard lock(watchMutex);
        if (watchInstalled || SDL_WasInit(SDL_INIT_EVENTS) == 0) {
            return;
        }
        watchInstalled = SDL_AddEventWatch(handleEvent, this);
    }

    void selectGamepad(SDL_JoystickID identifier) {
        activeGamepadID = identifier;
        activeControllerType = controllerTypeForSDLType(
            SDL_GetGamepadTypeForID(identifier)
        );
    }

    void selectFirstConnectedGamepad() {
        if (SDL_WasInit(SDL_INIT_GAMEPAD) == 0) {
            return;
        }

        int count = 0;
        SDL_JoystickID* identifiers = SDL_GetGamepads(&count);
        if (identifiers && count > 0) {
            selectGamepad(identifiers[0]);
        }
        SDL_free(identifiers);
    }

    std::atomic<SDL_JoystickID> activeGamepadID {0};
    std::atomic<NXControllerType> activeControllerType {
        NXControllerType::generic
    };
    std::mutex watchMutex;
    bool watchInstalled = false;
};

std::shared_ptr<UIImage> NXMakeControllerIconImage(
    NXFloat pointSize,
    NXFloat scale,
    const std::function<void(SkCanvas&, NXFloat)>& drawing
) {
    const auto pixelSize = std::max(
        1,
        static_cast<int>(std::ceil(pointSize * scale))
    );
#if defined(PLATFORM_SWITCH)
    const auto imageInfo = SkImageInfo::Make(
        pixelSize,
        pixelSize,
        kRGBA_8888_SkColorType,
        kPremul_SkAlphaType
    );
#else
    const auto imageInfo = SkImageInfo::MakeN32Premul(pixelSize, pixelSize);
#endif

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(imageInfo)) {
        return nullptr;
    }
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bitmap);
    canvas.scale(scale, scale);
    drawing(canvas, pointSize);

    auto image = bitmap.asImage();
    if (!image) {
        return nullptr;
    }
    auto result = std::make_shared<UIImage>(
        std::make_shared<CGImage>(std::move(image)),
        scale
    );
    result->setRenderModeAsTemplate(true);
    return result;
}

NXControllerIconResolver& NXControllerIconResolver::shared() {
    static NXControllerIconResolver resolver;
    return resolver;
}

NXControllerIconResolver::NXControllerIconResolver()
    : _impl(std::make_unique<Impl>()) {}

NXControllerIconResolver::~NXControllerIconResolver() = default;

NXControllerType NXControllerIconResolver::currentControllerType() const {
    return _impl->currentControllerType();
}

std::shared_ptr<UIImage> NXControllerIconResolver::iconForButton(
    NXActionButton button,
    NXControllerType controllerType,
    NXFloat pointSize
) const {
    if (!std::isfinite(pointSize) || pointSize <= 0 || pointSize > 512) {
        throw std::invalid_argument(
            "Controller icon point size must be finite and in (0, 512]"
        );
    }

#if defined(PLATFORM_SWITCH)
    // Switch supports Nintendo controllers only, and NXResponderAction already
    // maps its semantic actions to Nintendo's physical button positions.
    controllerType = NXControllerType::nintendoSwitch;
#else
    if (controllerType == NXControllerType::automatic) {
        controllerType = currentControllerType();
    }
#endif

    auto scale = 1.0f;
    if (const auto skiaContext = SkiaCtx::main()) {
        const auto displayScale = skiaContext->getScaleFactor();
        if (std::isfinite(displayScale) && displayScale > 0) {
            scale = displayScale;
        }
    }

    if (_impl->platformProvider) {
        if (auto platformIcon = _impl->platformProvider->iconForButton(
                button,
                controllerType,
                pointSize,
                scale
            )) {
            return platformIcon;
        }
    }

    return makeFallbackIcon(button, controllerType, pointSize, scale);
}

} // namespace NXKit
