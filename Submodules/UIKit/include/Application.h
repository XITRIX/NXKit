#pragma once

#include <SDL.h>
#include "include/gpu/ganesh/GrDirectContext.h"
#import "tools/window/WindowContext.h"
#import "SkiaCtx.h"

class Application {
public:
    Application();
    ~Application();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

//    std::unique_ptr<skwindow::WindowContext> skiaWindow;
    std::unique_ptr<SkiaCtx> skiaCtx;

    static Application* shared;
    static int resizingEventWatcher(void* data, SDL_Event* event);

    bool runLoop();
    void render();
};
