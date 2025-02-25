#pragma once

#include <string>
#include <UIKeyboardHIDUsage.h>
#include <tools/Tools.hpp>

namespace NXKit {

enum class UIKeyModifierFlags {
    alphaShift = 1 << 0,
    shift = 1 << 1,
    control = 1 << 2,
    alternate = 1 << 3,
    command = 1 << 4,
    numericPad = 1 << 5,
};

struct UIKey {
public:
    std::string characters() { return _characters; }
    OptionSet<UIKeyModifierFlags> modifierFlags() { return _modifierFlags; }
    UIKeyboardHIDUsage keyCode() { return _keyCode; }

private:
    friend class UIApplication;

    std::string _characters;
    OptionSet<UIKeyModifierFlags> _modifierFlags;
    UIKeyboardHIDUsage _keyCode;
};

enum class UIGamepadInputType {
    up,
    down,
    left,
    right,
    buttonA,
    buttonB,
    buttonX,
    buttonY,
    buttonStart,
    buttonOptions,
    buttonGuide,
    leftShoulder,
    rightShoulder,
    leftTrigger,
    rightTrigger,
    leftThumbstickButton,
    leftThumbstickAxisLeft,
    leftThumbstickAxisRight,
    leftThumbstickAxisUp,
    leftThumbstickAxisDown,
    rightThumbstickButton,
    rightThumbstickAxisLeft,
    rightThumbstickAxisRight,
    rightThumbstickAxisUp,
    rightThumbstickAxisDown
};

struct UIGamepadKey {
public:
    [[nodiscard]] bool isPressed() const { return _value >= isPressedThreshold; }
    [[nodiscard]] float value() const { return _value; }
    UIGamepadInputType inputType() { return _inputType; }
private:
    friend class UIApplication;

    float _value = 0;
    float isPressedThreshold = 0.5f;
    UIGamepadInputType _inputType;
};

}

