//
//  InputManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#include <Platforms/Universal/GLFWInputManager.hpp>
#include <Platforms/Universal/GLFWVideoContext.hpp>

#include <Core/Application/Application.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace NXKit {

#define GLFW_GAMEPAD_BUTTON_MAX 15
#define GLFW_GAMEPAD_AXIS_MAX 4

//static short controllersCount = 0;

//static void glfwJoystickCallback(int jid, int event)
//{
//    if (event == GLFW_CONNECTED) { controllersCount++; }
//    else if (event == GLFW_DISCONNECTED) { controllersCount--; }
//}

// LT and RT do not exist here because they are axes
static const size_t GLFW_BUTTONS_MAPPING[GLFW_GAMEPAD_BUTTON_MAX] = {
    BUTTON_A, // GLFW_GAMEPAD_BUTTON_A
    BUTTON_B, // GLFW_GAMEPAD_BUTTON_B
    BUTTON_X, // GLFW_GAMEPAD_BUTTON_X
    BUTTON_Y, // GLFW_GAMEPAD_BUTTON_Y
    BUTTON_LB, // GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
    BUTTON_RB, // GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
    BUTTON_BACK, // GLFW_GAMEPAD_BUTTON_BACK
    BUTTON_START, // GLFW_GAMEPAD_BUTTON_START
    BUTTON_GUIDE, // GLFW_GAMEPAD_BUTTON_GUIDE
    BUTTON_LSB, // GLFW_GAMEPAD_BUTTON_LEFT_THUMB
    BUTTON_RSB, // GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
    BUTTON_UP, // GLFW_GAMEPAD_BUTTON_DPAD_UP
    BUTTON_RIGHT, // GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
    BUTTON_DOWN, // GLFW_GAMEPAD_BUTTON_DPAD_DOWN
    BUTTON_LEFT, // GLFW_GAMEPAD_BUTTON_DPAD_LEFT
};

GLFWInputManager::GLFWInputManager() {
    window = ((GLFWVideoContext*) Application::shared()->getVideoContext())->getGLFWWindow();
//    glfwSetJoystickCallback(glfwJoystickCallback);
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

short GLFWInputManager::getGamepadsCount() {
    return controllersCount;
}

bool GLFWInputManager::getButton(short controller, ControllerButton key) {
    return buttons[controller][key];
}

bool GLFWInputManager::getButtonUp(short controller, ControllerButton key) {
    return buttonsUp[controller][key];
}

bool GLFWInputManager::getButtonDown(short controller, ControllerButton key) {
    return buttonsDown[controller][key];
}

float GLFWInputManager::getAxis(short controller, ControllerAxis axis) {
    return this->axis[controller][axis];
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
    for (int i = 0; i < _BRLS_KBD_KEY_LAST; i++) {
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

    for (int i = 0; i < _BRLS_MOUSE_LAST; i++) {
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

void GLFWInputManager::updateButton(short controller, ControllerButton button, bool newValue) {
    if (buttons[controller][button] != newValue) {
        if (newValue) {
            buttonsDown[controller][button] = true;
            buttonsUp[controller][button] = false;
        }
        else {
            buttonsDown[controller][button] = false;
            buttonsUp[controller][button] = true;
        }
        buttons[controller][button] = newValue;
    } else {
        buttonsDown[controller][button] = false;
        buttonsUp[controller][button] = false;
    }
}

void GLFWInputManager::updateGamepads() {
    controllersCount = 0;
    for (short i = 0; i < GLFW_JOYSTICK_LAST; i++) {
        controllersCount += glfwJoystickPresent(i);
    }

    for (short controller = 0; controller < controllersCount; controller++) {
        GLFWgamepadstate glfwState = {};
        glfwGetGamepadState(controller, &glfwState);

        for (size_t i = 0; i < GLFW_GAMEPAD_BUTTON_MAX; i++) {
            size_t button          = GLFW_BUTTONS_MAPPING[i];
            updateButton(controller, (ControllerButton) button, (bool)glfwState.buttons[i]);
        }

        updateButton(controller, BUTTON_LT, glfwState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1f);
        updateButton(controller, BUTTON_RT, glfwState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1f);

        updateButton(controller, BUTTON_NAV_UP, glfwState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -0.5f || buttons[controller][BUTTON_UP] || keys[BRLS_KBD_KEY_UP]);
        updateButton(controller, BUTTON_NAV_RIGHT, glfwState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] > 0.5f || buttons[controller][BUTTON_RIGHT] || keys[BRLS_KBD_KEY_RIGHT]);
        updateButton(controller, BUTTON_NAV_DOWN, glfwState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > 0.5f || buttons[controller][BUTTON_DOWN] || keys[BRLS_KBD_KEY_DOWN]);
        updateButton(controller, BUTTON_NAV_LEFT, glfwState.axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -0.5f || buttons[controller][BUTTON_LEFT] || keys[BRLS_KBD_KEY_LEFT]);

        for (size_t i = 0; i < GLFW_GAMEPAD_AXIS_MAX; i++) {
            axis[controller][i] = glfwState.axes[i];
        }
    }
}

void GLFWInputManager::update() {
    updateKeyboard();
    updateGamepads();
    updateMouse();
    updateTouch();

    getInputUpdated()->fire();
}

}