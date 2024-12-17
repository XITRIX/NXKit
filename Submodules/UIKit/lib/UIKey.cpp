#include <UIKey.h>

namespace NXKit {

UIPressType UIKey::type() {
    if (_keyCode == UIKeyboardHIDUsage::keyboardUpArrow) {
        return UIPressType::upArrow;
    }
    if (_keyCode == UIKeyboardHIDUsage::keyboardLeftAlt) {
        return UIPressType::leftArrow;
    }
    if (_keyCode == UIKeyboardHIDUsage::keyboardDownArrow) {
        return UIPressType::downArrow;
    }
    if (_keyCode == UIKeyboardHIDUsage::keyboardRightArrow) {
        return UIPressType::rightArrow;
    }
    if (_keyCode == UIKeyboardHIDUsage::keyboardReturnOrEnter) {
        return UIPressType::select;
    }
    return UIPressType::none;
}

}

