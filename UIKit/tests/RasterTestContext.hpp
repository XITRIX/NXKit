#pragma once
#include <SkiaCtx.h>
#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#endif
namespace NXKit::Internal {
// Regression executables do not start SDL's application backend, but text
// measurement still requires a live context and platform font manager.
class RasterTestContext final:public SkiaCtx {
public:
    RasterTestContext() {
#ifdef __APPLE__
        fontMgr=SkFontMgr_New_CoreText(nullptr);
#endif
    }
    NXSize getSize()override{return {800,600};}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}
    void swapBuffers()override{}
    bool platformRunLoop(std::function<bool()>)override{return false;}
protected:
    skgpu::graphite::Context* graphiteContext()override{return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
}
