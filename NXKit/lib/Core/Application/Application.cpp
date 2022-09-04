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
    delete keyWindow;
    delete FontManager::shared();
    delete InputManager::shared();
    delete videoContext;
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
    if (event->allTouches.size() > 0) setInputType(ApplicationInputType::TOUCH);
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
    static bool repeatingLocked            = false;
    static Time buttonPressTime     = 0;
    static int repeatingButtonTimer = 0;

    for (int i = 0; i < _BUTTON_MAX; i++)
    {
        if (manager->getButton((ControllerButton)i))
        {
            anyButtonPressed = true;
            repeating        = (repeatingButtonTimer > BUTTON_REPEAT_DELAY && repeatingButtonTimer % BUTTON_REPEAT_CADENCY == 0 && !repeatingLocked);

            if (inputType != ApplicationInputType::GAMEPAD) {
                setInputType(ApplicationInputType::GAMEPAD);
                return;
            }

            if (manager->getButtonDown((ControllerButton) i) || repeating) {
                repeatingLocked = !onControllerButtonPressed((ControllerButton) i, repeating);
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
    if (!anyButtonPressed) {
        repeatingLocked = false;
    }
}

void Application::setInputType(ApplicationInputType inputType) {
    this->inputType = inputType;
//    printf("Input type changed %d\n", inputType);
}

bool Application::onControllerButtonPressed(ControllerButton button, bool repeating) {
//    if (!focus) {
//        setFocus(keyWindow->getDefaultFocus());
//    }
    
    if (!focus) {
        return false;
    }

    if (focus->press(button)) {
        return false;
    }

    auto newFocus = focus;
    NavigationDirection direction = NavigationDirection::UP;
    if (button == BUTTON_NAV_UP) {
        newFocus = focus->getNextFocus(NavigationDirection::UP);
        direction = NavigationDirection::UP;
    }
    if (button == BUTTON_NAV_DOWN) {
        newFocus = focus->getNextFocus(NavigationDirection::DOWN);
        direction = NavigationDirection::DOWN;
    }
    if (button == BUTTON_NAV_LEFT) {
        newFocus = focus->getNextFocus(NavigationDirection::LEFT);
        direction = NavigationDirection::LEFT;
    }
    if (button == BUTTON_NAV_RIGHT) {
        newFocus = focus->getNextFocus(NavigationDirection::RIGHT);
        direction = NavigationDirection::RIGHT;
    }

    if (newFocus && newFocus != focus) {
        setFocus(newFocus);
        return true;
    } else if (!newFocus && focus) {
        focus->shakeHighlight(direction);
        return false;
    }

    return true;
}
NVGcontext* Application::getContext() {
    return videoContext->getNVGContext();
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

int Application::getFps() {
    static double counter = 0;
    static double res = 0;
    static double prevres = 0;
    static double prevt = getCPUTimeUsec();
    double t = getCPUTimeUsec();
    double dt = t - prevt;
    prevt = t;

    res += dt;
    counter++;

    if (counter >= 60) {
        prevres = res;
        res = 0;
        counter = 0;
    }

    int fps = 1 / (prevres / 1000000.0 / 60.0);
    return fps;
}

void Application::render() {
    videoContext->beginFrame();
    videoContext->clear(nvgRGB(0, 0, 0));

    nvgBeginFrame(getContext(), windowWidth, windowHeight, windowScale);
    keyWindow->internalDraw(getContext());

    renderFps();

    nvgEndFrame(getContext());
    videoContext->endFrame();
}

void Application::renderFps() {
    nvgFontSize(getContext(), 21);
    nvgFillColor(getContext(), UIColor::black.raw());
    nvgText(getContext(), 20, 20, ("FPS: " + std::to_string(getFps())).c_str(), nullptr);
}

void Application::flushContext() {
    nvgEndFrame(getContext());
}

void Application::setVideoContext(VideoContext* videoContext) {
    this->videoContext = videoContext;
}

VideoContext* Application::getVideoContext() {
    return videoContext;
}

}
