//
//  Application.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#pragma once

#include <Core/Driver/Video/video.hpp>
#include <Core/UIView/UIView.hpp>
#include <Core/UIWindow/UIWindow.hpp>
#include <Core/UIAppDelegate/UIAppDelegate.hpp>

namespace NXKit {

class UIView;
class UIWindow;

class Application {
public:
    static Application* shared();

    Application();

    UIWindow* getKeyWindow() { return keyWindow; }
    void setKeyWindow(UIWindow* window);
    
    bool mainLoop();
    void flushContext();
    void onWindowResized(unsigned width, unsigned height, float scale);
    VideoContext* getVideoContext();
    std::string getResourcesPath() { return resourcesPath; }

    void setResourcesPath(std::string resourcesPath) { this->resourcesPath = resourcesPath; }
    void setVideoContext(VideoContext*);

    UIView* getFocus();
    void setFocus(UIView* view);

    UIAppDelegate* getDelegate() { return delegate; }
private:
    inline static Application* _shared;

    VideoContext* videoContext = nullptr;
    UIAppDelegate* delegate = nullptr;
    UIWindow* keyWindow = nullptr;
    UIView* focus = nullptr;

    unsigned windowWidth, windowHeight;
    float windowScale;
    std::string resourcesPath;

    bool mainLoopIteration();
    void navigation();
    void render();

};

}
