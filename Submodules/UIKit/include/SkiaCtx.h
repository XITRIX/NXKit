#pragma once

#include <NXSize.h>

#include <SDL.h>
#include <memory>
#include "include/core/SkSurface.h"
#include <functional>
#include "include/core/SkFontMgr.h"

namespace NXKit {

bool platformRunLoop(std::function<bool()> loop);

class SkiaCtx {
public:
    virtual ~SkiaCtx() = default;
    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;
    virtual sk_sp<GrDirectContext> directContext() = 0;
    virtual void flushAndSubmit(sk_sp<SkSurface> surface);
    virtual NXSize getSize() = 0;
    virtual void swapBuffers() = 0;

    virtual float getScaleFactor() { return 1; }
    virtual sk_sp<SkFontMgr> getFontMgr() { return fontMgr; }

protected:
    NXSize _size;
    sk_sp<SkFontMgr> fontMgr;
};

std::unique_ptr<SkiaCtx> MakeSkiaCtx();

}
