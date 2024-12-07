#include <platforms/SkiaCtx_sdlBase.h>

using namespace NXKit;

SkiaCtx_sdlBase::SkiaCtx_sdlBase()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

#ifdef USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    flags |= SDL_WINDOW_OPENGL;
#endif

    window = SDL_CreateWindow("Window", 12, 12, 1280, 720, flags);
    auto context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
}

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
    return (float)dw / (float)w;
}