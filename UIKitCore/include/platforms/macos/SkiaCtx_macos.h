#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/ports/SkCFObject.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mutex>
#include <thread>
#import <SDL3/SDL.h>

namespace NXKit {

class SkiaCtx_macos: public SkiaCtx_sdlBase {
public:
    SkiaCtx_macos();
    ~SkiaCtx_macos() override;

    sk_sp<SkSurface> getBackbufferSurface() override;
//    float getScaleFactor() override;
    void swapBuffers() override;
    UIUserInterfaceStyle getThemeMode() override;

protected:
    bool platformRunLoop(std::function<bool ()> loop) override;

private:
    std::unique_ptr<skgpu::graphite::Context> context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    sk_sp<SkSurface> surface;
    sk_cfp<CFTypeRef> device;
    sk_cfp<CFTypeRef> queue;
    sk_cfp<CFTypeRef> drawable;
    std::recursive_mutex contextMutex;
    std::thread::id renderThread;

    skgpu::graphite::Context* graphiteContext() override { return context.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() override { return recorder.get(); }
    
    void initContext();
    void destroyContext();
    bool ensureRenderThread();
};

}
