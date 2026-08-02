#pragma once

#include <SDL3/SDL.h>
#include <SkiaCtx.h>

#include <memory>

namespace NXKit {

class SkiaCtx_switch: public SkiaCtx {
public:
    SkiaCtx_switch();
    ~SkiaCtx_switch() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    void flushAndSubmit(sk_sp<SkSurface> surface) override;

    UIUserInterfaceStyle getThemeMode() override;

    void swapBuffers() override;

    NXSize getSize() override;

    float getScaleFactor() override;

    bool platformRunLoop(std::function<bool ()> loop) override;

protected:
    skgpu::graphite::Context* graphiteContext() override;
    skgpu::graphite::Recorder* graphiteRecorder() override;

private:
    struct GraphiteState;

    SDL_Window *window = nullptr;
    std::unique_ptr<GraphiteState> graphite;
    sk_sp<SkSurface> surface;

    bool initContext();
    bool configureSurface();
};

}
