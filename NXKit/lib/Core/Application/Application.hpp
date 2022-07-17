//
//  Application.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#pragma once

#include "video.hpp"
#include "UIView.hpp"
#include "UIWindow.hpp"
#include "UIAppDelegate.hpp"

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

    UIAppDelegate* getDelegate() { return delegate; }
private:
    inline static Application* _shared;

    UIWindow* keyWindow;
    unsigned windowWidth, windowHeight;
    float windowScale;
    std::string resourcesPath;
    UIAppDelegate* delegate = nullptr;

    bool mainLoopIteration();
    void render();
    VideoContext* videoContext = nullptr;

};
