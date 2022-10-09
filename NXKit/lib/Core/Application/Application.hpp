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
#include <Core/UITraitCollection/UITraitCollection.hpp>
#include <Core/UIAppDelegate/UIAppDelegate.hpp>
#include <Core/Utils/NotificationEvent.hpp>
#include <Core/Utils/SharedBase/SharedBase.hpp>

namespace NXKit {

class UIView;
class UIWindow;

enum class ApplicationInputType {
    GAMEPAD,
    TOUCH
};

class Application: public enable_shared_from_base<Application> {
public:
    static Application* shared();

    Application();
    virtual ~Application();

    std::shared_ptr<UIWindow> getKeyWindow() { return keyWindow; }
    void setKeyWindow(std::shared_ptr<UIWindow> window);
    
    bool mainLoop();
    void flushContext();
    void onWindowResized(unsigned width, unsigned height, float scale);
    NVGcontext* getContext();
    std::shared_ptr<VideoContext> getVideoContext();
    std::string getResourcesPath() { return resourcesPath; }

    void setResourcesPath(std::string resourcesPath) { this->resourcesPath = resourcesPath; }
    void setVideoContext(std::shared_ptr<VideoContext> videoContext);

    std::weak_ptr<UIView> getFocus();
    void setFocus(std::shared_ptr<UIView> view);

    ApplicationInputType getInputType() { return inputType; }
    std::shared_ptr<UIAppDelegate> getDelegate() { return delegate; }

    NotificationEvent<std::shared_ptr<UIView>>* getFocusDidChangeEvent() { return &focusDidChangeEvent; };
    UIUserInterfaceStyle getUserInterfaceStyle();
    
private:
    inline static Application* _shared;

    std::shared_ptr<VideoContext> videoContext;
    std::shared_ptr<UIAppDelegate> delegate;
    std::shared_ptr<UIWindow> keyWindow;
    std::weak_ptr<UIView> focus;

    void setInputType(ApplicationInputType inputType);
    ApplicationInputType inputType = ApplicationInputType::GAMEPAD;
    NotificationEvent<std::shared_ptr<UIView>> focusDidChangeEvent;

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
