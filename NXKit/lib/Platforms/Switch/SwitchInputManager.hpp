//
//  SwitchInputManager.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 21.08.2022.
//

#pragma once

#include <Platforms/Universal/GLFWInputManager.hpp>
#include <map>

namespace NXKit {

class SwitchInputManager: public GLFWInputManager {
public:
    void updateTouch() override;
};
    
}
