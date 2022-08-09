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
    bool getKey(BrlsKeyboardScancode key) override;
    bool getKeyUp(BrlsKeyboardScancode key) override;
    bool getKeyDown(BrlsKeyboardScancode key) override;
    void setKey(BrlsKeyboardScancode key, bool state);
    void update() override;

private:
    bool keys[BRLS_KBD_KEY_LAST];
    bool keysUp[BRLS_KBD_KEY_LAST];
    bool keysDown[BRLS_KBD_KEY_LAST];
};

}
