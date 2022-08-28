//
//  Application.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#include <math.h>

#include <Platforms/InputManager.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/Utils/Animation/Core/Time.hpp>
#include <Core/Utils/Animation/NXFloat/NXFloat.hpp>
#include <Core/Utils/FontManager/FontManager.hpp>
#include <Core/Driver/Video/video.hpp>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace NXKit {

#define BUTTON_REPEAT_DELAY 15
#define BUTTON_REPEAT_CADENCY 5

Application* Application::shared() {
    return Application::_shared;
}

Application::Application() {
    delegate = new UIAppDelegate();
    Application::_shared = this;
}

Application::~Application() {
    delete delegate;
    delete videoContext;
    delete keyWindow;
    delete FontManager::shared();
    delete InputManager::shared();
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
    input();

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

void Application::input() {
    auto manager = InputManager::shared();
//    if (manager->getGamepadsCount() == 0 || focus == nullptr) return;

    bool anyButtonPressed           = false;
    bool repeating                  = false;
    static Time buttonPressTime     = 0;
    static int repeatingButtonTimer = 0;

//    controllerState.buttons[BUTTON_A]  |= inputManager->getKeyboardKeyState(BRLS_KBD_KEY_ENTER);
//    controllerState.buttons[BUTTON_B]  |= inputManager->getKeyboardKeyState(BRLS_KBD_KEY_ESCAPE);

    for (int i = 0; i < _BUTTON_MAX; i++)
    {
        if (manager->getButton((ControllerButton)i))
        {
            anyButtonPressed = true;
            repeating        = (repeatingButtonTimer > BUTTON_REPEAT_DELAY && repeatingButtonTimer % BUTTON_REPEAT_CADENCY == 0);

            if (manager->getButtonDown((ControllerButton) i) || repeating) {
                onControllerButtonPressed((ControllerButton) i, repeating);
            }
        }

        if (manager->getButtonDown((ControllerButton)i) || manager->getButtonUp((ControllerButton)i))
            buttonPressTime = repeatingButtonTimer = 0;
    }

    if (anyButtonPressed && getCPUTimeUsec() - buttonPressTime > 1000)
    {
        buttonPressTime = getCPUTimeUsec();
        repeatingButtonTimer++; // Increased once every ~1ms
    }
}

void Application::onControllerButtonPressed(ControllerButton button, bool repeating) {
    auto newFocus = focus;
    if (button == BUTTON_NAV_UP) {
        newFocus = focus->getNextFocus(NavigationDirection::UP);
    }
    if (button == BUTTON_NAV_DOWN) {
        newFocus = focus->getNextFocus(NavigationDirection::DOWN);
    }
    if (button == BUTTON_NAV_LEFT) {
        newFocus = focus->getNextFocus(NavigationDirection::LEFT);
    }
    if (button == BUTTON_NAV_RIGHT) {
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
    if (this->focus == view) return;

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
