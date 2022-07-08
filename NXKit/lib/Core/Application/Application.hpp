//
//  Application.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#pragma once

#include "glfw_video.hpp"
#include "UIView.hpp"
#include "UIWindow.hpp"

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
    GLFWVideoContext* getVideoContext();
private:
    inline static Application* _shared;

    UIWindow* keyWindow;
    unsigned windowWidth, windowHeight;
    float windowScale;

    bool mainLoopIteration();
    GLFWVideoContext* videoContext = nullptr;

};
