//
//  SDLInputManager.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.10.2022.
//

#pragma once

#include <Platforms/InputManager.hpp>
#include <SDL2/SDL.h>

namespace NXKit {

class SDLInputManager: public InputManager {
public:
    SDLInputManager();
    virtual ~SDLInputManager();

    Point getCoursorPosition() override;

    bool getMouseButton(BrlsMouseButton button) override;
    bool getMouseButtonUp(BrlsMouseButton button) override;
    bool getMouseButtonDown(BrlsMouseButton button) override;

    bool getKey(BrlsKeyboardScancode key) override;
    bool getKeyUp(BrlsKeyboardScancode key) override;
    bool getKeyDown(BrlsKeyboardScancode key) override;

    short getGamepadsCount() override;

    bool getButton(short controller, ControllerButton button) override;
    bool getButtonUp(short controller, ControllerButton button) override;
    bool getButtonDown(short controller, ControllerButton button) override;

    bool getButton(ControllerButton button) override;
    bool getButtonUp(ControllerButton button) override;
    bool getButtonDown(ControllerButton button) override;

    float getAxis(short controller, ControllerAxis axis) override;

    int touchCount() override;
    std::shared_ptr<UITouch> getTouch(int id) override;
    std::vector<std::shared_ptr<UITouch>> getTouches() override;

    std::string getButtonIcon(ControllerButton button) override;

    void update() override;

protected:
//    GLFWwindow* window = nullptr;

//    short controllersCount = 0;
    Point coursorPosition;

    bool mouseButtons[_BRLS_MOUSE_LAST];
    bool mouseButtonsUp[_BRLS_MOUSE_LAST];
    bool mouseButtonsDown[_BRLS_MOUSE_LAST];
//
//    bool keys[_BRLS_KBD_KEY_LAST];
//    bool keysUp[_BRLS_KBD_KEY_LAST];
//    bool keysDown[_BRLS_KBD_KEY_LAST];

//    bool buttons[GLFW_JOYSTICK_LAST][_BUTTON_MAX];
//    bool buttonsUp[GLFW_JOYSTICK_LAST][_BUTTON_MAX];
//    bool buttonsDown[GLFW_JOYSTICK_LAST][_BUTTON_MAX];

//    bool univButtons[_BUTTON_MAX];
//    bool univButtonsUp[_BUTTON_MAX];
//    bool univButtonsDown[_BUTTON_MAX];

//    float axis[GLFW_JOYSTICK_LAST][_AXES_MAX];

    std::vector<std::shared_ptr<UITouch>> touches;

//    virtual void updateKeyboard();
    virtual void updateMouse();
    virtual void updateTouch();
//    virtual void updateGamepads();
//    virtual void updateUniversalGamepad();
//    virtual void updateButton(short controller, ControllerButton button, bool newValue);
//    static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::vector<std::weak_ptr<UIGestureRecognizer>> getRecognizerHierachyFrom(std::shared_ptr<UIView> view);
};

}
