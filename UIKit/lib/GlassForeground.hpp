#pragma once
#include "GlassVibrancy.hpp"
#include "include/effects/SkRuntimeEffect.h"
#include <stdexcept>
#include <string>

namespace NXKit::Internal {
// Share foreground blending between material children and a resting lens.
inline sk_sp<SkBlender> glassForegroundBlender(bool dark) {
    static const auto runtime=[] {
        const auto code=std::string(glassVibrancySource)+R"sksl(
uniform float dark;
half4 main(half4 source,half4 backdrop) {
    if(source.a<=0)return backdrop;
    half3 color=half3(glassVibrantColor(float3(source.rgb/max(source.a,.0001)),float3(backdrop.rgb/max(backdrop.a,.0001)),dark));
    half3 ink=mix(source.rgb,color*source.a,backdrop.a);
    return half4(ink+backdrop.rgb*(1-source.a),source.a+backdrop.a*(1-source.a));
})sksl";
        auto result=SkRuntimeEffect::MakeForBlender(SkString(code));
        if(!result.effect)throw std::runtime_error(result.errorText.c_str());
        return result.effect;
    }();
    SkRuntimeBlendBuilder builder(runtime);builder.uniform("dark")=dark?1.f:0.f;
    return builder.makeBlender();
}
}
