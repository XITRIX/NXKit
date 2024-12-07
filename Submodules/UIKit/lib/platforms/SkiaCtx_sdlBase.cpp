#include <platforms/SkiaCtx_sdlBase.h>

SkiaCtx_sdlBase::SkiaCtx_sdlBase(SDL_Window *window): window(window)
{ }

void SkiaCtx_sdlBase::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

NXSize SkiaCtx_sdlBase::getSize() {
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    return { (NXFloat)w, (NXFloat)h };
}