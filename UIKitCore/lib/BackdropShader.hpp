#pragma once
#include <BackdropEffect.h>
#include <CALayerCornerCurve.h>
#include "include/core/SkRefCnt.h"

class SkRuntimeEffect;
class SkImageFilter;

namespace NXKit::Internal {
class BackdropShader {
public:
    static sk_sp<SkRuntimeEffect> compile(const BackdropEffect& effect);
    static void validate(const BackdropEffect& effect, const SkRuntimeEffect& runtimeEffect);
    static sk_sp<SkImageFilter> makeFilter(
        const BackdropEffect& effect, const sk_sp<SkRuntimeEffect>& runtimeEffect,
        NXSize size, NXFloat radius, CALayerCornerCurve curve, NXFloat scale,
        NXFloat intensity = 0, NXPoint location = {}
    );
};
}
