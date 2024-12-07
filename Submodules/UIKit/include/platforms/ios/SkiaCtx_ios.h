#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/gpu/ganesh/GrDirectContext.h>

class SkiaCtx_ios: public SkiaCtx_sdlBase {
public:
    SkiaCtx_ios(SDL_Window* window);

    sk_sp<SkSurface> getBackbufferSurface() override;
    float getScaleFactor() override;
    sk_sp<GrDirectContext> directContext() override { return context; }
private:
    sk_sp<GrDirectContext> context;

    void initContext();
};
