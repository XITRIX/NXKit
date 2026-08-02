#include <platforms/SkiaCtx_sdlBase.h>
#include <SDL3/SDL.h>
#include <cmath>
#include <thread>

using namespace NXKit;

SkiaCtx_sdlBase::SkiaCtx_sdlBase()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL initialization failed: %s",
                     SDL_GetError());
        return;
    }
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    flags |= SDL_WINDOW_METAL;
#elif defined(USE_GLES)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    flags |= SDL_WINDOW_OPENGL;
#endif

    window = SDL_CreateWindow("Window", 1280, 720, flags);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL window creation failed: %s",
                     SDL_GetError());
        return;
    }

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    metalView = SDL_Metal_CreateView(window);
#else
    glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
#endif
}

SkiaCtx_sdlBase::~SkiaCtx_sdlBase() {
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    if (metalView) {
        SDL_Metal_DestroyView(metalView);
        metalView = nullptr;
    }
#endif

    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void SkiaCtx_sdlBase::swapBuffers() {
    if (window && glContext) {
        SDL_GL_SwapWindow(window);
    }
}

NXSize SkiaCtx_sdlBase::getSize() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return { (NXFloat)w, (NXFloat)h };
}

float SkiaCtx_sdlBase::getScaleFactor() {
    int w, h, dw, dh;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GetWindowSizeInPixels(window, &dw, &dh);

    if (w <= 0 || h <= 0) {
        return 1;
    }

    return (float)dw / (float)w;
}

int SkiaCtx_sdlBase::screenFrameRate() {
    SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);
    if (displayID == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to get current display: %s",
                     SDL_GetError());
        return 60;
    }

    const SDL_DisplayMode* mode = nullptr;
    if ((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0) {
        mode = SDL_GetWindowFullscreenMode(window);
    }

    // Borderless fullscreen and windowed modes both use the display's current mode.
    if (!mode) {
        mode = SDL_GetCurrentDisplayMode(displayID);
    }
    if (!mode) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not get the current SDL display mode: %s",
                     SDL_GetError());
        return 60;
    }

    // May be zero if undefined
    if (mode->refresh_rate <= 0.0f) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Refresh rate unknown; assuming 60 Hz");
        return 60;
    }

    return static_cast<int>(std::lround(mode->refresh_rate));
}

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
void* SkiaCtx_sdlBase::metalLayer() const {
    if (!metalView) {
        return nullptr;
    }

    return SDL_Metal_GetLayer(metalView);
}
#endif
