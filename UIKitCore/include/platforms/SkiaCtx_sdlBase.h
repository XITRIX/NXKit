#pragma once

#include <SkiaCtx.h>
#include <SDL_metal.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include "Timer.h"

namespace NXKit {

class SkiaCtx_sdlBase : public SkiaCtx {
public:
    SkiaCtx_sdlBase();
    ~SkiaCtx_sdlBase() override;

    void swapBuffers() override;

    NXSize getSize() override;

    float getScaleFactor() override;

    int screenFrameRate() override;

protected:
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    SDL_MetalView metalView = nullptr;

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    void* metalLayer() const;
#endif
};

}
