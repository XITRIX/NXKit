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
#include "Time.hpp"
#include "CGFloat.hpp"
#include "video.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

//Application* Application::_shared = nullptr;
Application* Application::shared() {
    return Application::_shared;
}

Application::Application() {
    Application::_shared = this;
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
    return videoContext->mainLoopInteraction();
}

bool Application::mainLoop() {
    // Main loop callback
    if (!mainLoopIteration()) return false;

    // Animations
    updateHighlightAnimation();
    Ticking::updateTickings();

    // Render
    render();

    return true;
}

void Application::render() {
    videoContext->beginFrame();
    videoContext->clear(nvgRGB(0, 0, 0));

    nvgBeginFrame(videoContext->getNVGContext(), windowWidth, windowHeight, windowScale);
    keyWindow->internalDraw(videoContext->getNVGContext());
//    nvgResetTransform(videoContext->getNVGContext()); // scale

    nvgEndFrame(videoContext->getNVGContext());
    videoContext->endFrame();
}

void Application::flushContext() {
    nvgEndFrame(this->videoContext->getNVGContext());

    nvgBeginFrame(videoContext->getNVGContext(), windowWidth, windowHeight, windowScale);
//    nvgScale(videoContext->getNVGContext(), windowScale, windowScale);
}

void Application::setVideoContext(VideoContext* videoContext) {
    this->videoContext = videoContext;
}

VideoContext* Application::getVideoContext() {
    return videoContext;
}
