#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/ports/SkCFObject.h>
#include <CoreFoundation/CoreFoundation.h>

#include "include/core/SkFont.h"

namespace NXKit {

class SkiaCtx_ios : public SkiaCtx_sdlBase {
public:
    SkiaCtx_ios();
    ~SkiaCtx_ios() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    float getScaleFactor() override;
    NXSize getSize() override;
    void swapBuffers() override;

    UIEdgeInsets deviceSafeAreaInsets() override;

    NXKit::UIUserInterfaceStyle getThemeMode() override;
//    void setTargetFrameRate(int frameRate) override;

    bool platformRunLoop(std::function<bool ()> loop) override;
//    void sleepForNextFrame() override {}

private:
    std::unique_ptr<skgpu::graphite::Context> context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    sk_sp<SkSurface> surface;
    sk_cfp<CFTypeRef> device;
    sk_cfp<CFTypeRef> queue;
    sk_cfp<CFTypeRef> drawable;

    skgpu::graphite::Context* graphiteContext() override { return context.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() override { return recorder.get(); }
    
    void initContext();
    void destroyContext();
};

}
