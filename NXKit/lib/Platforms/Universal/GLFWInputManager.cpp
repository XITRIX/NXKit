//
//  InputManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#include "GLFWInputManager.hpp"
#include "GLFWVideoContext.hpp"

#include <Core/Application/Application.hpp>

namespace NXKit {

//void GLFWInputManager::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    GLFWInputManager* self = (GLFWInputManager*)Application::getPlatform()->getInputManager();
//    KeyState state;
//    state.key = (BrlsKeyboardScancode)key;
//    state.mods = mods;
//    state.pressed = action != GLFW_RELEASE;
//    const char* key_name = glfwGetKeyName(key, scancode);
////    if (key_name != NULL)
////        Logger::debug("Key: {} / Code: {}", key_name, key);
////    else
////        Logger::debug("Key: NULL / Code: {}", key);
//    self->getKeyboardKeyStateChanged()->fire(state);
//}

GLFWInputManager::GLFWInputManager() {
    window = ((GLFWVideoContext*) Application::shared()->getVideoContext())->getGLFWWindow();
    //    glfwSetCursorPosCallback(window, cursorCallback);
    //    glfwSetKeyCallback(window, keyboardCallback);
}

InputManager* InputManager::shared() {
    if (!_shared) _shared = new GLFWInputManager();
    return _shared;
}

Point GLFWInputManager::getCoursorPosition() {
    return coursorPosition;
}

bool GLFWInputManager::getMouseButton(BrlsMouseButton button) {
    return mouseButtons[button];
}

bool GLFWInputManager::getMouseButtonUp(BrlsMouseButton button) {
    return mouseButtonsUp[button];
}

bool GLFWInputManager::getMouseButtonDown(BrlsMouseButton button) {
    return mouseButtonsDown[button];
}

bool GLFWInputManager::getKey(BrlsKeyboardScancode key) {
    return keys[key];
}

bool GLFWInputManager::getKeyUp(BrlsKeyboardScancode key) {
    return keysUp[key];
}

bool GLFWInputManager::getKeyDown(BrlsKeyboardScancode key) {
    return keysDown[key];
}

int GLFWInputManager::touchCount() {
    return (int) touches.size();
}

UITouch* GLFWInputManager::getTouch(int id) {
    return touches[id];
}

std::vector<UITouch*> GLFWInputManager::getTouches() {
    return touches;
}

void GLFWInputManager::updateKeyboard() {
    for (int i = 0; i < BRLS_KBD_KEY_LAST; i++) {
        bool key = glfwGetKey(this->window, i);
        if (keys[i] != key) {
            if (key) {
                keysDown[i] = true;
                keysUp[i] = false;
            }
            else {
                keysDown[i] = false;
                keysUp[i] = true;
            }
            keys[i] = key;
        } else {
            keysDown[i] = false;
            keysUp[i] = false;
        }
    }
}

void GLFWInputManager::updateMouse() {
    double x, y;
    glfwGetCursorPos(this->window, &x, &y);
    coursorPosition = Point(x, y);

    for (int i = 0; i < BRLS_MOUSE_LAST; i++) {
        bool button = glfwGetMouseButton(this->window, i);
        if (mouseButtons[i] != button) {
            if (button) {
                mouseButtonsDown[i] = true;
                mouseButtonsUp[i] = false;
            }
            else {
                mouseButtonsDown[i] = false;
                mouseButtonsUp[i] = true;
            }
            mouseButtons[i] = button;
        } else {
            mouseButtonsDown[i] = false;
            mouseButtonsUp[i] = false;
        }
    }
}

std::vector<UIGestureRecognizer*> getRecognizerHierachyFrom(UIView* view) {
    std::vector<UIGestureRecognizer*> recognizers;
    while (view) {
        for (auto recognizer : view->getGestureRecognizers())
            recognizers.push_back(recognizer);
        view = view->getSuperview();
    }

    return recognizers;
}

void GLFWInputManager::updateTouch() {
    if (getMouseButtonDown(BrlsMouseButton::BRLS_MOUSE_LKB)) {
        auto touch = new UITouch(0, getCoursorPosition(), std::time(nullptr));
        touch->window = Application::shared()->getKeyWindow();
        touch->view = touch->window->hitTest(touch->absoluteLocation, nullptr);
        touch->gestureRecognizers = getRecognizerHierachyFrom(touch->view);
        touch->phase = UITouchPhase::BEGIN;
        touches.push_back(touch);
    } else if (getMouseButtonUp(BrlsMouseButton::BRLS_MOUSE_LKB)) {
        for (int i = 0; i < touchCount(); i++) {
            auto touch = touches[i];
            if (touch->touchId == 0) {
                touch->phase = UITouchPhase::ENDED;
            }
        }
    } else if (getMouseButton(BrlsMouseButton::BRLS_MOUSE_LKB)) {
        for (int i = 0; i < touchCount(); i++) {
            auto touch = touches[i];
            if (touch->touchId == 0) {
                touch->timestamp = std::time(nullptr);
                touch->phase = UITouchPhase::MOVED;
                touch->updateAbsoluteLocation(getCoursorPosition());
            }
        }
    } else {
        for (int i = 0; i < touchCount(); i++) {
            auto touch = touches[i];
            if (touch->touchId == 0) {
                touches.erase(touches.begin() + i);
                delete touch;
            }
        }
    }
}

void GLFWInputManager::update() {
    updateKeyboard();
    updateMouse();
    updateTouch();

    getInputUpdated()->fire();
}

}
