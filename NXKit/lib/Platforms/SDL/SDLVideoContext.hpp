//
//  SDLVideoContext.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.10.2022.
//

#pragma once

#include <Core/Driver/Video/video.hpp>


//#define GLM_FORCE_PURE
//#define GLM_ENABLE_EXPERIMENTAL

//#define GLFW_INCLUDE_NONE
//#include <OpenGL/OpenGL.h>
#include <SDL2/SDL.h>

#include <string>

namespace NXKit {

// GLFW Video Context
class SDLVideoContext : public VideoContext
{
public:
    SDLVideoContext(std::string windowTitle, uint32_t windowWidth, uint32_t windowHeight);
    ~SDLVideoContext();

    NVGcontext* getNVGContext() override;

    void clear(NVGcolor color) override;
    void beginFrame() override;
    void endFrame() override;
    void resetState() override;
    void disableScreenDimming(bool disable) override;
    void getContextPixels(int x, int y, int w, int h, unsigned char** pixels) override;
    float getScaleFactor() override;
    bool mainLoopInteraction() override;
    UIUserInterfaceStyle getUserInterfaceStyle() override;

    SDL_Window* getGLFWWindow();

private:
    SDL_Window* window     = nullptr;
    NVGcontext* nvgContext = nullptr;

#ifdef __SWITCH__
//    GLFWmonitor* monitor = nullptr;
//    int oldWidth, oldHeight;
#endif
};

}
