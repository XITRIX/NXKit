#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/gpu/ganesh/GrDirectContext.h>

namespace NXKit {

class SkiaCtx_macos: public SkiaCtx_sdlBase {
public:
    SkiaCtx_macos();

    sk_sp<SkSurface> getBackbufferSurface() override;
//    float getScaleFactor() override;
    sk_sp<GrDirectContext> directContext() override { return context; }
private:
    sk_sp<GrDirectContext> context;
    sk_sp<SkSurface> surface;
    
    void initContext();
};

}