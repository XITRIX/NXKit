//
//  SwitchVideoContext.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 21.08.2022.
//

#pragma once

#include <Platforms/Universal/GLFWVideoContext.hpp>

namespace NXKit {

class SwitchVideoContext: public GLFWVideoContext {
public:
    SwitchVideoContext(std::string windowTitle, uint32_t windowWidth, uint32_t windowHeight):
        GLFWVideoContext(windowTitle, windowWidth, windowHeight)
    {}

    UIUserInterfaceStyle getUserInterfaceStyle() override;
};

}
