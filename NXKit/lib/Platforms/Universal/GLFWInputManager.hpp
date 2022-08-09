//
//  GlfwInputManager.h
//  NXKit
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#pragma once

#include "InputManager.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace NXKit {

class GLFWInputManager: public InputManager {
public:
    GLFWInputManager();

    Point getCoursorPosition() override;
    bool getMouseButton(BrlsMouseButton button) override;
    bool getMouseButtonUp(BrlsMouseButton button) override;
    bool getMouseButtonDown(BrlsMouseButton button) override;
    bool getKey(BrlsKeyboardScancode key) override;
    bool getKeyUp(BrlsKeyboardScancode key) override;
    bool getKeyDown(BrlsKeyboardScancode key) override;

    int touchCount() override;
    UITouch* getTouch(int id) override;
    std::vector<UITouch*> getTouches() override;

    void update() override;

private:
    GLFWwindow* window = nullptr;

    Point coursorPosition;
    
    bool mouseButtons[BRLS_MOUSE_LAST];
    bool mouseButtonsUp[BRLS_MOUSE_LAST];
    bool mouseButtonsDown[BRLS_MOUSE_LAST];

    bool keys[BRLS_KBD_KEY_LAST];
    bool keysUp[BRLS_KBD_KEY_LAST];
    bool keysDown[BRLS_KBD_KEY_LAST];

    std::vector<UITouch*> touches;

    void updateKeyboard();
    void updateMouse();
    void updateTouch();
    static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

}
