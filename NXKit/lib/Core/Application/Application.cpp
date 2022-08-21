//
//  Application.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#include <math.h>

#include <Core/Application/Application.hpp>
#include <Platforms/InputManager.hpp>
#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/Utils/Animation/Core/Time.hpp>
#include <Core/Utils/Animation/NXFloat/NXFloat.hpp>
#include <Core/Driver/Video/video.hpp>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace NXKit {

Application* Application::shared() {
    return Application::_shared;
}

Application::Application() {
    delegate = new UIAppDelegate();
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

    // Input
    InputManager::shared()->update();
    navigation();

    UIEvent* event = new UIEvent();
    event->allTouches = InputManager::shared()->getTouches();
    keyWindow->sendEvent(event);
    delete event;

    // Animations
    updateHighlightAnimation();
    Ticking::updateTickings();

    // Render
    render();

    return true;
}

void Application::navigation() {
    auto manager = InputManager::shared();
    if (manager->getGamepadsCount() == 0 || focus == nullptr) return;

    auto newFocus = focus;
    if (manager->getButtonDown(0, BUTTON_NAV_UP)) {
        newFocus = focus->getNextFocus(NavigationDirection::UP);
    }
    if (manager->getButtonDown(0, BUTTON_NAV_DOWN)) {
        newFocus = focus->getNextFocus(NavigationDirection::DOWN);
    }
    if (manager->getButtonDown(0, BUTTON_NAV_LEFT)) {
        newFocus = focus->getNextFocus(NavigationDirection::LEFT);
    }
    if (manager->getButtonDown(0, BUTTON_NAV_RIGHT)) {
        newFocus = focus->getNextFocus(NavigationDirection::RIGHT);
    }

    if (newFocus && newFocus != focus) {
        setFocus(newFocus);
    }
}

UIView* Application::getFocus() {
    return this->focus;
}

void Application::setFocus(UIView* view) {
    if (this->focus) {
        this->focus->resignFocused();
    }
    this->focus = view;

    if (this->focus) {
        this->focus->becomeFocused();
        
        if (this->focus->superview)
            this->focus->superview->subviewFocusDidChange(this->focus, this->focus);
    }
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
//    nvgSave(this->videoContext->getNVGContext());
    nvgEndFrame(this->videoContext->getNVGContext());

//    nvgBeginFrame(videoContext->getNVGContext(), windowWidth, windowHeight, windowScale);
    //    nvgScale(videoContext->getNVGContext(), windowScale, windowScale);
}

void Application::setVideoContext(VideoContext* videoContext) {
    this->videoContext = videoContext;
}

VideoContext* Application::getVideoContext() {
    return videoContext;
}

}
