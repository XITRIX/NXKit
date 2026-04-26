#pragma once

#include <SkiaCtx.h>
#include <platforms/SkiaCtx_sdlBase.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/mtl/GrMtlTypes.h>
#include <mutex>
#include <thread>
#import <SDL.h>

namespace NXKit {

class SkiaCtx_macos: public SkiaCtx_sdlBase {
public:
    SkiaCtx_macos();
    ~SkiaCtx_macos() override;

    sk_sp<SkSurface> getBackbufferSurface() override;
//    float getScaleFactor() override;
    void flushAndSubmit(sk_sp<SkSurface> surface) override;
    void swapBuffers() override;
    sk_sp<GrDirectContext> directContext() override { return context; }
    UIUserInterfaceStyle getThemeMode() override;

protected:
    bool platformRunLoop(std::function<bool ()> loop) override;

private:
    sk_sp<GrDirectContext> context;
    sk_sp<SkSurface> surface;
    sk_cfp<GrMTLHandle> device;
    sk_cfp<GrMTLHandle> queue;
    sk_cfp<GrMTLHandle> drawable;
    std::recursive_mutex contextMutex;
    std::thread::id renderThread;
    
    void initContext();
    void destroyContext();
    bool ensureRenderThread();
};

}
