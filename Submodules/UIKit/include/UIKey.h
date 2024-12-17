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

enum class UIPressType {
    none,
    upArrow,
    downArrow,
    leftArrow,
    rightArrow,
    select,
    menu,
};

struct UIKey {
public:
    UIPressType type();
    std::string characters() { return _characters; }
    OptionSet<UIKeyModifierFlags> modifierFlags() { return _modifierFlags; }
    UIKeyboardHIDUsage keyCode() { return _keyCode; }

private:
    std::string _characters;
    OptionSet<UIKeyModifierFlags> _modifierFlags;
    UIKeyboardHIDUsage _keyCode;

    friend class UIApplication;
};

}

