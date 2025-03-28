#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/gpu/ganesh/GrDirectContext.h>

#include "include/core/SkFont.h"

namespace NXKit {

class SkiaCtx_ios : public SkiaCtx_sdlBase {
public:
    SkiaCtx_ios();

    sk_sp<SkSurface> getBackbufferSurface() override;

    float getScaleFactor() override;
    NXSize getSize() override;

    sk_sp<GrDirectContext> directContext() override { return context; }
    UIEdgeInsets deviceSafeAreaInsets() override;

    NXKit::UIUserInterfaceStyle getThemeMode() override;
//    void setTargetFrameRate(int frameRate) override;

    bool platformRunLoop(std::function<bool ()> loop) override;
//    void sleepForNextFrame() override {}

private:
    sk_sp<GrDirectContext> context;
    sk_sp<SkSurface> surface;
    
    void initContext();
};

}
