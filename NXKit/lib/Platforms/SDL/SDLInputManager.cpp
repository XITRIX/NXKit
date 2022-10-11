//
//  SDLInputManager.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.10.2022.
//

#include "SDLInputManager.hpp"
#include <Core/Application/Application.hpp>

namespace NXKit {

SDLInputManager::SDLInputManager() {
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
}

SDLInputManager::~SDLInputManager() {

}

Point SDLInputManager::getCoursorPosition() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Point(x, y);
}

bool SDLInputManager::getMouseButton(BrlsMouseButton button) { return mouseButtons[button]; }
bool SDLInputManager::getMouseButtonUp(BrlsMouseButton button) { return mouseButtonsUp[button]; }
bool SDLInputManager::getMouseButtonDown(BrlsMouseButton button) { return mouseButtonsDown[button]; }

bool SDLInputManager::getKey(BrlsKeyboardScancode key) { return false; }
bool SDLInputManager::getKeyUp(BrlsKeyboardScancode key) { return false; }
bool SDLInputManager::getKeyDown(BrlsKeyboardScancode key) { return false; }

short SDLInputManager::getGamepadsCount() { return 0; }

bool SDLInputManager::getButton(short controller, ControllerButton button) { return false; }
bool SDLInputManager::getButtonUp(short controller, ControllerButton button) { return false; }
bool SDLInputManager::getButtonDown(short controller, ControllerButton button) { return false; }

bool SDLInputManager::getButton(ControllerButton button) { return false; }
bool SDLInputManager::getButtonUp(ControllerButton button) { return false; }
bool SDLInputManager::getButtonDown(ControllerButton button) { return false; }

float SDLInputManager::getAxis(short controller, ControllerAxis axis) { return 0; }

int SDLInputManager::touchCount() { return (int) touches.size(); }
std::shared_ptr<UITouch> SDLInputManager::getTouch(int id) { return touches[id]; }
std::vector<std::shared_ptr<UITouch>> SDLInputManager::getTouches() { return touches; }

std::string SDLInputManager::getButtonIcon(ControllerButton button) {
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

void SDLInputManager::update() {
    SDL_PumpEvents();

    updateMouse();
    updateTouch();

//    SDL_Event e;
//    while (SDL_PollEvent(&e)) {
//    }
}

void SDLInputManager::updateMouse() {
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    coursorPosition = Point(x, y);

    for (int i = 0; i < _BRLS_MOUSE_LAST; i++) {
        bool button = (buttons & SDL_BUTTON(i+1)) != 0;
        if (mouseButtons[i] != button) {
            if (button) {
                printf("tap %d, %d\n", x, y);
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

void SDLInputManager::updateTouch() {
//    SDL_GetTou
//    SDL_GetTouch
//    if (getMouseButtonDown(BrlsMouseButton::BRLS_MOUSE_LKB)) {
//        auto touch = new UITouch(0, getCoursorPosition(), getCPUTimeUsec());
//        touch->window = Application::shared()->getKeyWindow();
//        touch->view = touch->window->hitTest(touch->absoluteLocation, nullptr);
//        touch->gestureRecognizers = getRecognizerHierachyFrom(touch->view);
//        touch->phase = UITouchPhase::BEGIN;
//        touches.push_back(touch);
//    } else if (getMouseButtonUp(BrlsMouseButton::BRLS_MOUSE_LKB)) {
//        for (int i = 0; i < touchCount(); i++) {
//            auto touch = touches[i];
//            if (touch->touchId == 0) {
//                touch->timestamp = getCPUTimeUsec();
//                touch->phase = UITouchPhase::ENDED;
//            }
//        }
//    } else if (getMouseButton(BrlsMouseButton::BRLS_MOUSE_LKB)) {
//        for (int i = 0; i < touchCount(); i++) {
//            auto touch = touches[i];
//            if (touch->touchId == 0) {
//                touch->timestamp = getCPUTimeUsec();
//                touch->phase = UITouchPhase::MOVED;
//                touch->updateAbsoluteLocation(getCoursorPosition());
//            }
//        }
//    } else {
//        for (int i = 0; i < touchCount(); i++) {
//            auto touch = touches[i];
//            if (touch->touchId == 0) {
//                touches.erase(touches.begin() + i);
//                delete touch;
//            }
//        }
//    }
}

std::vector<std::weak_ptr<UIGestureRecognizer>> SDLInputManager::getRecognizerHierachyFrom(std::shared_ptr<UIView> view) {
    std::vector<std::weak_ptr<UIGestureRecognizer>> recognizers;
    while (view) {
        for (auto recognizer : view->getGestureRecognizers())
            recognizers.push_back(recognizer);
        view = view->getSuperview().lock();
    }

    return recognizers;
}

}
