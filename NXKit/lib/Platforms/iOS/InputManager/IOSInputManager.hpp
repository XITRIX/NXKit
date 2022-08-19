//
//  IOSInputManager.hpp
//  NXKit-iOS
//
//  Created by Даниил Виноградов on 18.07.2022.
//

#pragma once

#include "InputManager.hpp"

namespace NXKit {

class IOSInputManager: public InputManager {
public:
    IOSInputManager();

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

    float getAxis(short controller, ControllerAxis axis) override;

    int touchCount() override;
    UITouch* getTouch(int id) override;
    std::vector<UITouch*> getTouches() override;

    void update() override;
    
    void setKey(BrlsKeyboardScancode key, bool state);
private:
    bool keys[_BRLS_KBD_KEY_LAST];
    bool keysUp[_BRLS_KBD_KEY_LAST];
    bool keysDown[_BRLS_KBD_KEY_LAST];
};

}
