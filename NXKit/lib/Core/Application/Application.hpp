//
//  Application.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#pragma once

#include <Platforms/InputManager.hpp>
#include <Core/Driver/Video/video.hpp>
#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/UIAppDelegate/UIAppDelegate.hpp>
#include <Core/Utils/NotificationEvent.hpp>

namespace NXKit {

class UIView;
class UIWindow;

enum class ApplicationInputType {
    GAMEPAD,
    TOUCH
};

class Application {
public:
    static Application* shared();

    Application();
    virtual ~Application();

    UIWindow* getKeyWindow() { return keyWindow; }
    void setKeyWindow(UIWindow* window);
    
    bool mainLoop();
    void flushContext();
    void onWindowResized(unsigned width, unsigned height, float scale);
    NVGcontext* getContext();
    VideoContext* getVideoContext();
    std::string getResourcesPath() { return resourcesPath; }

    void setResourcesPath(std::string resourcesPath) { this->resourcesPath = resourcesPath; }
    void setVideoContext(VideoContext*);

    UIView* getFocus();
    void setFocus(UIView* view);

    ApplicationInputType getInputType() { return inputType; }
    UIAppDelegate* getDelegate() { return delegate; }

    NotificationEvent<UIView*>* getFocusDidChangeEvent() { return &focusDidChangeEvent; };
private:
    inline static Application* _shared;

    VideoContext* videoContext = nullptr;
    UIAppDelegate* delegate = nullptr;
    UIWindow* keyWindow = nullptr;
    UIView* focus = nullptr;

    void setInputType(ApplicationInputType inputType);
    ApplicationInputType inputType = ApplicationInputType::GAMEPAD;
    NotificationEvent<UIView*> focusDidChangeEvent;

    unsigned windowWidth, windowHeight;
    float windowScale;
    std::string resourcesPath;

    int getFps();
    bool mainLoopIteration();
    bool onControllerButtonPressed(ControllerButton button, bool repeating);
    void input();
    void render();
    void renderFps();

};

}
