#pragma once

#include <SDL.h>
#include "include/gpu/ganesh/GrDirectContext.h"
#import "tools/window/WindowContext.h"
#import "SkiaCtx.h"

namespace NXKit {

class Application {
public:
    Application();
    ~Application();

private:
    SDL_Window* window;

    sk_sp<SkTypeface> typeface;
    float fRotationAngle = 0;

    std::unique_ptr<SkiaCtx> skiaCtx;

    static Application* shared;
    static int resizingEventWatcher(void* data, SDL_Event* event);

    bool runLoop();
    void render();
};

}