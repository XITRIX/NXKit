#pragma once

#include <SkiaCtx.h>
#include <SDL3/SDL_metal.h>
#include "Timer.h"
#include <SDL3/SDL_events.h>
#include <thread>

namespace NXKit {

class SkiaCtx_sdlBase : public SkiaCtx {
public:
    SkiaCtx_sdlBase();
    ~SkiaCtx_sdlBase() override;

    void swapBuffers() override;
    void sleepForNextFrame() override;

    NXSize getSize() override;

    float getScaleFactor() override;

    int screenFrameRate() override;

private:
    static bool SDLCALL watchWindowExpose(void* userdata, SDL_Event* event);
    std::thread::id _eventThread;
    bool _watchesWindowExpose = false;

protected:
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    SDL_MetalView metalView = nullptr;

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    void* metalLayer() const;
#endif
};

}
