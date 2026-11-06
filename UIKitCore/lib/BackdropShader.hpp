#pragma once
#include <BackdropEffect.h>
#include <CALayerCornerCurve.h>
#include "include/core/SkRefCnt.h"
#include <span>
#include "include/core/SkRect.h"

class SkRuntimeEffect;
class SkImageFilter;
class SkCanvas;

namespace NXKit::Internal {
struct BackdropPyramidImage {
    sk_sp<SkImageFilter> image;
    std::array<NXFloat,4> rect{}, info{};
    NXFloat sampleReach = 0;
    // Physical source footprint, distinct from packed-atlas addressing reach.
    NXFloat sourceReach = 0;
};
class BackdropShader {
public:
    static sk_sp<SkRuntimeEffect> compile(const BackdropEffect& effect);
    static void validate(const BackdropEffect& effect, const SkRuntimeEffect& runtimeEffect);
    // Borrow only while the immutable description is retained by the caller.
    static std::span<const NXFloat> uniformValues(const BackdropEffect& effect, const std::string& name);
    // Capture at reduced resolution with an explicit reconstruction grid.
    // Inverse matrix filters alone may fuse away the intended resampling.
    static sk_sp<SkImageFilter> capture(NXFloat scale);
    // Reconstruct one independently sampled lane, including a Gaussian mixture.
    static sk_sp<SkImageFilter> filteredInput(BackdropBlurKernel kernel, NXFloat scaleX,
        NXFloat scaleY, NXFloat captureScale, sk_sp<SkImageFilter> captured);
    // A caller may omit level zero only when its shader never samples it.
    // The prefiltered initial reduction must also be active.
    static BackdropPyramidImage makePyramid(const BackdropEffect& effect, SkRect domainPixels, NXFloat radiusScale=1,
        SkCanvas* canvas=nullptr,sk_sp<SkImageFilter> prefix=nullptr,bool omitBaseTile=false);
    static bool canFreezePyramid(SkCanvas* canvas,NXFloat radiusScale);
    static sk_sp<SkImageFilter> freezePrefix(SkCanvas* canvas,SkRect inputBounds,NXFloat radiusScale);
    static sk_sp<SkImageFilter> makeFilter(
        const BackdropEffect& effect, const sk_sp<SkRuntimeEffect>& runtimeEffect,
        NXSize size, NXFloat radius, CALayerCornerCurve curve, NXFloat scale,
        NXFloat intensity = 0, NXPoint location = {}, SkCanvas* canvas=nullptr,bool omitBaseTile=false
    );
};
}
