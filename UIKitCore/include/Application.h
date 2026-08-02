#pragma once

#include <SDL3/SDL_events.h>
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

    bool runLoop();
    void render();
};

}
