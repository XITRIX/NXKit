#pragma once

#include <SDL3/SDL_video.h>
#include <functional>
#include <memory>
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "Timer.h"
#include <UITraitCollection.h>
#include <UIEdgeInsets.h>
#include <Geometry.h>

namespace skgpu::graphite {
class Context;
class Recorder;
}

namespace NXKit {

class SkiaCtx {
public:
    virtual ~SkiaCtx() = default;
    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;
    virtual void flushAndSubmit(sk_sp<SkSurface> surface);
    virtual NXSize getSize() = 0;
    virtual void swapBuffers() = 0;

    virtual float getScaleFactor() { return 1; }
    virtual float getExtraScaleFactor() { return 1; }
    virtual sk_sp<SkFontMgr> getFontMgr() { return fontMgr; }
    SkString getDefaultFontFamilyName() const;
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style = SkFontStyle()) const;

    virtual int screenFrameRate() { return 60; }
    virtual void setTargetFrameRate(int frameRate) { targetFrameRate = frameRate; }

    virtual UIEdgeInsets deviceSafeAreaInsets() { return {}; }

    virtual UIUserInterfaceStyle getThemeMode() { return UIUserInterfaceStyle::light; }

    void setExtraScaleFactor(NXFloat extraScaleFactor) { _extraScaleFactor = extraScaleFactor; }

    virtual bool platformRunLoop(std::function<bool()> loop) = 0;
    virtual void sleepForNextFrame();
    
    static std::shared_ptr<SkiaCtx> main() { return _main; }
    static std::shared_ptr<SkiaCtx> _main;
protected:
    std::unique_ptr<skgpu::graphite::Recorder> createGraphiteRecorder(
            skgpu::graphite::Context* context) const;
    virtual skgpu::graphite::Context* graphiteContext() = 0;
    virtual skgpu::graphite::Recorder* graphiteRecorder() = 0;

    NXSize _size;
    NXFloat _extraScaleFactor = 1;
    // Rendering clients assume a usable manager even when a platform-specific
    // font service or GPU backend fails during startup.
    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefEmpty();
    Timer currentRunLoopStartTimer;

private:
    int targetFrameRate = -1;
    friend class UIApplication;
};

std::unique_ptr<SkiaCtx> MakeSkiaCtx();

}
