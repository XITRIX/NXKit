//
//  IOSInputManager.cpp
//  NXKit-iOS
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#include "IOSInputManager.hpp"

#include <Core/Application/Application.hpp>

namespace NXKit {

IOSInputManager::IOSInputManager() {
    //    window = ((GLFWVideoContext*) Application::shared()->getVideoContext())->getGLFWWindow();
    //    glfwSetCursorPosCallback(window, cursorCallback);
}

InputManager* InputManager::shared() {
    if (!_shared) _shared = new IOSInputManager();
    return _shared;
}

Point IOSInputManager::getCoursorPosition() {
    return Point();
}

bool IOSInputManager::getKey(BrlsKeyboardScancode key) {
    return false;
}

bool IOSInputManager::getKeyUp(BrlsKeyboardScancode key) {
    return false;
}

bool IOSInputManager::getKeyDown(BrlsKeyboardScancode key) {
    return false;
}

void IOSInputManager::setKey(BrlsKeyboardScancode key, bool state) {
    if (keys[key] != state) {
        if (state) {
            keysDown[key] = true;
            keysUp[key] = false;
        }
        else {
            keysDown[key] = false;
            keysUp[key] = true;
        }
        keys[key] = state;
    } else {
        keysDown[key] = false;
        keysUp[key] = false;
    }
}

void IOSInputManager::update() {

}

}
