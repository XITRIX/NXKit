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

IOSInputManager::~IOSInputManager() { }

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


bool IOSInputManager::getMouseButton(BrlsMouseButton button) {
    return false;
}

bool IOSInputManager::getMouseButtonUp(BrlsMouseButton button) {
    return false;
}

bool IOSInputManager::getMouseButtonDown(BrlsMouseButton button) {
    return false;
}

short IOSInputManager::getGamepadsCount() {
    return 0;
}

bool IOSInputManager::getButton(short controller, ControllerButton button) {
    return false;
}

bool IOSInputManager::getButtonUp(short controller, ControllerButton button) {
    return false;
}

bool IOSInputManager::getButtonDown(short controller, ControllerButton button) {
    return false;
}

bool IOSInputManager::getButton(ControllerButton button) {
    return false;
}

bool IOSInputManager::getButtonUp(ControllerButton button) {
    return false;
}

bool IOSInputManager::getButtonDown(ControllerButton button) {
    return false;
}

float IOSInputManager::getAxis(short controller, ControllerAxis axis) {
    return 0;
}

int IOSInputManager::touchCount() {
    return 0;
}

UITouch* IOSInputManager::getTouch(int id) {
    return nullptr;
}

std::vector<UITouch*> IOSInputManager::getTouches() {
    return {};
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
