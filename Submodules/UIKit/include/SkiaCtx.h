#pragma once

#include "SDL_video.h"
#include <memory>
#include "include/core/SkSurface.h"
#include <functional>
#include "include/core/SkFontMgr.h"
#include <UITraitCollection.h>
#include <Geometry.h>

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
    virtual float getExtraScaleFactor() { return 1; }
    virtual sk_sp<SkFontMgr> getFontMgr() { return fontMgr; }

    virtual UIUserInterfaceStyle getThemeMode() { return UIUserInterfaceStyle::light; }

    void setExtraScaleFactor(NXFloat extraScaleFactor) { _extraScaleFactor = extraScaleFactor; }
    
    static std::shared_ptr<SkiaCtx> main() { return _main; }
    static std::shared_ptr<SkiaCtx> _main;
protected:
    NXSize _size;
    NXFloat _extraScaleFactor = 1;
    sk_sp<SkFontMgr> fontMgr;


private:
    friend class UIApplication;
};

std::unique_ptr<SkiaCtx> MakeSkiaCtx();

}
