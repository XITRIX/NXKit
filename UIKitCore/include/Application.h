#pragma once

#include <SDL_events.h>
#include "include/gpu/ganesh/GrDirectContext.h"
#include "tools/window/WindowContext.h"
#include "SkiaCtx.h"
#include "UIView.h"

namespace NXKit {

class Application {
public:
    Application();
    ~Application();

private:
    sk_sp<SkTypeface> typeface;
    float fRotationAngle = 0;

    std::shared_ptr<UIView> keyWindow;

    std::unique_ptr<SkiaCtx> skiaCtx;

    static Application* shared;
    static int resizingEventWatcher(void* data, SDL_Event* event);

    bool runLoop();
    void render();
};

}