//
//  Application.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#include <math.h>

#include "Application.hpp"
#include "UIView.hpp"
#include "UIWindow.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

constexpr uint32_t ORIGINAL_WINDOW_WIDTH  = 1280;
constexpr uint32_t ORIGINAL_WINDOW_HEIGHT = 720;

//Application* Application::_shared = nullptr;
Application* Application::shared() {
    return Application::_shared;
}

Application::Application() {
    Application::_shared = this;
    this->videoContext = new GLFWVideoContext("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
}

void Application::setKeyWindow(UIWindow *window) {
    keyWindow = window;
    keyWindow->setSize(Size(windowWidth, windowHeight));
}

void Application::onWindowResized(unsigned width, unsigned height, float scale) {
    windowWidth  = width;
    windowHeight = height;

    // Rescale UI
    windowScale = scale;
    if (keyWindow)
        keyWindow->setSize(Size(width, height));
}

bool Application::mainLoopIteration() {
    bool isActive;
    do {
        isActive = !glfwGetWindowAttrib(this->videoContext->getGLFWWindow(), GLFW_ICONIFIED);

        if (isActive)
            glfwPollEvents();
        else
            glfwWaitEvents();
    } while (!isActive);

    bool platform = true;

#ifdef __SWITCH__
    platform = appletMainLoop();
#endif

    return !glfwWindowShouldClose(this->videoContext->getGLFWWindow()) || !platform;
}

bool Application::mainLoop() {

    // Main loop callback
    if (!mainLoopIteration()) return false;

    videoContext->beginFrame();
    videoContext->clear(nvgRGB(100, 0, 0));

    nvgBeginFrame(videoContext->getNVGContext(), windowWidth, windowHeight, windowScale);
    
    keyWindow->internalDraw(videoContext->getNVGContext());

    nvgResetTransform(videoContext->getNVGContext()); // scale
    nvgEndFrame(videoContext->getNVGContext());
    videoContext->endFrame();

    return true;
}

void Application::flushContext() {
    nvgEndFrame(this->videoContext->getNVGContext());

    nvgBeginFrame(videoContext->getNVGContext(), windowWidth, windowHeight, windowScale);
//    nvgScale(videoContext->getNVGContext(), windowScale, windowScale);
}

GLFWVideoContext* Application::getVideoContext() {
    return videoContext;
}
