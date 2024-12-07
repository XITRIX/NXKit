#pragma once

#include <SkiaCtx.h>
#include <include/gpu/ganesh/GrDirectContext.h>


class SkiaCtx_sdlBase: public SkiaCtx {
public:
    SkiaCtx_sdlBase(SDL_Window* window);

    virtual void swapBuffers() override;
    virtual NXSize getSize() override;

protected:
    SDL_Window* window = nullptr;
};