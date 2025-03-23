#pragma once

#include <SkiaCtx.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include "Timer.h"

namespace NXKit {

class SkiaCtx_sdlBase : public SkiaCtx {
public:
    SkiaCtx_sdlBase();

    void swapBuffers() override;

    NXSize getSize() override;

    float getScaleFactor() override;

    int screenFrameRate() override;

protected:
    SDL_Window *window = nullptr;
};

}