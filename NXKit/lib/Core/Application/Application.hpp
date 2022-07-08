//
//  Application.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.07.2022.
//

#pragma once

#include "glfw_video.hpp"
#include "UIView.hpp"

class UIView;

class Application {
public:
    static Application* shared();

    Application();

    UIView* getRootView() { return rootView; }
    void setRootView(UIView* view);
    
    bool mainLoop();
    void flushContext();
    void onWindowResized(unsigned width, unsigned height, float scale);
    GLFWVideoContext* getVideoContext();
private:
    inline static Application* _shared;

    UIView* rootView;
    unsigned windowWidth, windowHeight;
    float windowScale;

    bool mainLoopIteration();
    GLFWVideoContext* videoContext = nullptr;

};
