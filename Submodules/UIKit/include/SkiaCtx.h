#pragma once

#include <NXSize.h>

#include <SDL.h>
#include <memory>
#include "include/core/SkSurface.h"
#include <functional>

bool platformRunLoop(std::function<bool()> loop);

class SkiaCtx {
public:
    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;
    virtual sk_sp<GrDirectContext> directContext() = 0;
    virtual void flushAndSubmit(sk_sp<SkSurface> surface);
    virtual NXSize getSize() = 0;
    virtual float getScaleFactor() { return 1; }
    virtual void swapBuffers() = 0;

protected:
    NXSize _size;
};

std::unique_ptr<SkiaCtx> MakeSkiaCtx(SDL_Window* window);
