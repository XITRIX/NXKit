#include <platforms/SkiaCtx_sdlBase.h>
#include <SDL.h>
#include <thread>

using namespace NXKit;

SkiaCtx_sdlBase::SkiaCtx_sdlBase()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    flags |= SDL_WINDOW_METAL;
#elif defined(__SWITCH__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    flags |= SDL_WINDOW_OPENGL;
#elif defined(USE_GLES)
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

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, flags);
    if (!window) {
        SDL_GetError();
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
        SDL_GL_DeleteContext(glContext);
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
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    SDL_Metal_GetDrawableSize(window, &dw, &dh);
#else
    SDL_GL_GetDrawableSize(window, &dw, &dh);
#endif

    if (w <= 0 || h <= 0) {
        return 1;
    }

    return (float)dw / (float)w;
}

int SkiaCtx_sdlBase::screenFrameRate() {
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    if (displayIndex < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to get current display: %s",
                     SDL_GetError());

        // Assume display 0 if it fails
        displayIndex = 0;
    }

    SDL_DisplayMode mode;
    if ((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN) {
        // Use the window display mode for full-screen exclusive mode
        if (SDL_GetWindowDisplayMode(window, &mode) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "SDL_GetWindowDisplayMode() failed: %s",
                         SDL_GetError());

            // Assume 60 Hz
            return 60;
        }
    }
    else {
        // Use the current display mode for windowed and borderless
        if (SDL_GetCurrentDisplayMode(displayIndex, &mode) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "SDL_GetCurrentDisplayMode() failed: %s",
                         SDL_GetError());

            // Assume 60 Hz
            return 60;
        }
    }

    // May be zero if undefined
    if (mode.refresh_rate == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Refresh rate unknown; assuming 60 Hz");
        mode.refresh_rate = 60;
    }

    return mode.refresh_rate;
}

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
void* SkiaCtx_sdlBase::metalLayer() const {
    if (!metalView) {
        return nullptr;
    }

    return SDL_Metal_GetLayer(metalView);
}
#endif
