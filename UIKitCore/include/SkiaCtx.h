#pragma once

#include <SDL3/SDL_video.h>
#include <functional>
#include <memory>
#include <optional>
#include <cmath>
#include <utility>
#include <vector>
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
    virtual sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style = SkFontStyle(),
                                                NXFloat pointSize = 17) const;

    virtual int screenFrameRate() { return 60; }
    virtual void setTargetFrameRate(int frameRate) { targetFrameRate = frameRate; }

    virtual UIEdgeInsets deviceSafeAreaInsets() { return {}; }

    /** Native UIKit overrides this with window traits. Other platforms use
     logical window points: regular width >= 600, regular height >= 480.
     An empty or invalid axis is unspecified; display scale does not affect it. */
    virtual std::pair<UIUserInterfaceSizeClass, UIUserInterfaceSizeClass> getSizeClasses() {
        const auto size = getSize();
        const auto classify = [](NXFloat value, NXFloat regularMinimum) {
            if (!std::isfinite(value) || value <= 0) return UIUserInterfaceSizeClass::unspecified;
            return value >= regularMinimum ? UIUserInterfaceSizeClass::regular : UIUserInterfaceSizeClass::compact;
        };
        return {classify(size.width, 600), classify(size.height, 480)};
    }

    virtual UIUserInterfaceStyle getThemeMode() { return UIUserInterfaceStyle::light; }

    void setExtraScaleFactor(NXFloat extraScaleFactor) { _extraScaleFactor = extraScaleFactor; }

    virtual bool platformRunLoop(std::function<bool()> loop) = 0;
    virtual void sleepForNextFrame();
    
    static std::shared_ptr<SkiaCtx> main() { return _main; }
    static std::shared_ptr<SkiaCtx> _main;
protected:
    std::unique_ptr<skgpu::graphite::Recorder> createGraphiteRecorder(
            skgpu::graphite::Context* context) const;
    bool finishGraphiteWork(skgpu::graphite::Context* context,
                            skgpu::graphite::Recorder* recorder) const;
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
    friend class CABackdropEffectLayer;
    friend bool applicationRunLoop();
    using BackdropCompletion = std::function<void(std::optional<NXFloat>)>;
    struct BackdropRead {
        sk_sp<SkImage> image;
        BackdropCompletion completion;
    };
    void readBackdrop(sk_sp<SkImage> image, BackdropCompletion completion);
    void submitBackdropReads();
    void pollBackdropReads();
    std::vector<BackdropRead> _backdropReads;
};

std::unique_ptr<SkiaCtx> MakeSkiaCtx();

}
