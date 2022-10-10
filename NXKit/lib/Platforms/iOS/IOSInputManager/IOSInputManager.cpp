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

std::shared_ptr<UITouch> IOSInputManager::getTouch(int id) {
    return nullptr;
}

std::vector<std::shared_ptr<UITouch>> IOSInputManager::getTouches() {
    return touches;
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
    updateKeyboard();
    updateTouch();
}

void IOSInputManager::updateTouch() {
    std::map<unsigned long, std::shared_ptr<UITouch>> oldTouches;
    for (auto touch: touches) {
        oldTouches[touch->touchId] = touch;
    }

    for (auto touchIter: currentTouches)
    {
        auto find = oldTouches.find(touchIter.first);
        if (find == oldTouches.end()) {
            auto touch = make_shared<UITouch>(touchIter.first, Point(touchIter.second.x, touchIter.second.y), getCPUTimeUsec());
            touch->window = Application::shared()->getKeyWindow();
            touch->view = touch->window->hitTest(touch->absoluteLocation, nullptr);
            touch->gestureRecognizers = getRecognizerHierachyFrom(touch->view);
            touch->phase = UITouchPhase::BEGIN;
            touches.push_back(touch);
//             printf("Touch begin at: X - %f, Y - %f\n", touchIter.second.x, touchIter.second.y);
        } else {
            auto touch = find->second;
            touch->timestamp = getCPUTimeUsec();
            touch->phase = UITouchPhase::MOVED;
            touch->updateAbsoluteLocation(Point(touchIter.second.x, touchIter.second.y));
            oldTouches.erase(find);
//             printf("Touch moved at: X - %f, Y - %f\n", touchIter.second.x, touchIter.second.y);
        }
    }

    for (auto touchIter: oldTouches) {
        std::shared_ptr<UITouch> touch = touchIter.second;
        if (touch->phase == UITouchPhase::MOVED) {
            touch->timestamp = getCPUTimeUsec();
            touch->phase = UITouchPhase::ENDED;
//             printf("Touch ended\n");
        } else if (touch->phase == UITouchPhase::ENDED) {
            touches.erase(std::remove(touches.begin(), touches.end(), touch));
//             printf("Touch removed\n");
//            delete touch;
        }
    }
}

void IOSInputManager::updateKeyboard() {
    for (int i = 0; i < _BRLS_KBD_KEY_LAST; i++) {
        bool key = currentKeys[i];
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

std::string IOSInputManager::getButtonIcon(ControllerButton button) {
    switch (button) {
        case BUTTON_A:
            return "\uE0E0";
        case BUTTON_B:
            return "\uE0E1";
        case BUTTON_X:
            return "\uE0E2";
        case BUTTON_Y:
            return "\uE0E3";
        case BUTTON_LSB:
            return "\uE104";
        case BUTTON_RSB:
            return "\uE105";
        case BUTTON_LT:
            return "\uE0E6";
        case BUTTON_RT:
            return "\uE0E7";
        case BUTTON_LB:
            return "\uE0E4";
        case BUTTON_RB:
            return "\uE0E5";
        case BUTTON_START:
            return "\uE0EF";
        case BUTTON_BACK:
            return "\uE0F0";
        case BUTTON_LEFT:
            return "\uE0ED";
        case BUTTON_UP:
            return "\uE0EB";
        case BUTTON_RIGHT:
            return "\uE0EE";
        case BUTTON_DOWN:
            return "\uE0EC";
        default:
            return "\uE152";
    }
}

std::vector<std::weak_ptr<UIGestureRecognizer>> IOSInputManager::getRecognizerHierachyFrom(std::shared_ptr<UIView> view) {
    std::vector<std::weak_ptr<UIGestureRecognizer>> recognizers;
    while (view) {
        for (auto recognizer : view->getGestureRecognizers())
            recognizers.push_back(recognizer);
        view = view->getSuperview().lock();
    }

    return recognizers;
}

}
