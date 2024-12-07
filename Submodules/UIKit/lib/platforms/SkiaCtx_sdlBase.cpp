#include <platforms/SkiaCtx_sdlBase.h>

using namespace NXKit;

SkiaCtx_sdlBase::SkiaCtx_sdlBase(SDL_Window *window): window(window)
{ }

void SkiaCtx_sdlBase::swapBuffers() {
    SDL_GL_SwapWindow(window);
}

NXSize SkiaCtx_sdlBase::getSize() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return { (NXFloat)w, (NXFloat)h };
}

float SkiaCtx_sdlBase::getScaleFactor() {
    int w, h, dw, dh;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GL_GetDrawableSize(window, &dw, &dh);
    return dw / w;
}
