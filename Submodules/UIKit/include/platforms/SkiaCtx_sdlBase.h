#pragma once

#include <SkiaCtx.h>
#include <include/gpu/ganesh/GrDirectContext.h>

namespace NXKit {

class SkiaCtx_sdlBase : public SkiaCtx {
public:
    SkiaCtx_sdlBase();

    virtual void swapBuffers() override;

    virtual NXSize getSize() override;

    virtual float getScaleFactor() override;

protected:
    SDL_Window *window = nullptr;
};

}