#pragma once

#include <SDL.h>
#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/gpu/ganesh/GrDirectContext.h>

namespace NXKit {

class SkiaCtx_switch: public SkiaCtx {
public:
    SkiaCtx_switch();
    ~SkiaCtx_switch();

    sk_sp<SkSurface> getBackbufferSurface() override;
//    float getScaleFactor() override;
    sk_sp<GrDirectContext> directContext() override { return context; }

    UIUserInterfaceStyle getThemeMode() override { return UIUserInterfaceStyle::light; }

    virtual void swapBuffers() override;

    virtual NXSize getSize() override;

    virtual float getScaleFactor() override;
    
private:
    SDL_Window *window = nullptr;

    sk_sp<GrDirectContext> context;
    sk_sp<SkSurface> surface;
    
    void initContext();
};

}
