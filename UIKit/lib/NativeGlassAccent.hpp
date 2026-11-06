#pragma once
#include "NativeGlassAccentReferences.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace NXKit::Internal {
namespace NativeGlassAccentDetail {
// HSL conversion follows the native achromatic threshold. Saturation is allowed
// above one by the inverse conversion; native clamps its final RGB components.
inline std::array<double,3> hsl(const NativeGlassColor& color) {
    const double r=color.r,g=color.g,b=color.b;
    const double high=std::max({r,g,b}),low=std::min({r,g,b}),delta=high-low;
    const double lightness=(high+low)*.5;
    if(delta<.0003452669770922512) return {0,0,lightness};
    double hue;
    if(high==r) hue=(g-b)/delta+(g<b?6:0);
    else if(high==g) hue=(b-r)/delta+2;
    else hue=(r-g)/delta+4;
    return {hue/6,delta/(lightness>.5?2-high-low:high+low),lightness};
}
inline std::array<double,3> rgb(double hue,double saturation,double lightness) {
    if(saturation==0) return {lightness,lightness,lightness};
    const double upper=lightness>=.5?lightness+saturation-lightness*saturation:
        lightness*(1+saturation);
    const double lower=2*lightness-upper;
    const auto channel=[&](double h) {
        if(h<0) h+=1;
        if(h>1) h-=1;
        const double value=h<1./6?lower+(upper-lower)*6*h:
            h<.5?upper:h<2./3?lower+(upper-lower)*(2./3-h)*6:lower;
        return std::clamp(value,0.,1.);
    };
    return {channel(hue+1./3),channel(hue),channel(hue-1./3)};
}
}

// Portable translation of iOS27 DesignLibrary's CPU tint-matrix construction.
// Exact native equations and independent 224-matrix verification are recorded
// in docs/liquid-glass-native-settings/color-transfer/native-cpu-accent.json.
// The tiny reference list selects the resolved material recipe; no captured
// color palette or interpolation participates in converting arbitrary RGB.
// Tint alpha is composed separately by the renderer.
inline NativeGlassVibrantColorMatrix nativeGlassAccentMatrix(
    const NativeGlassProfile& profile,NativeGlassColor tint) {
    if(!std::isfinite(tint.r)||!std::isfinite(tint.g)||!std::isfinite(tint.b))
        return profile.accent.vibrant;
    tint.r=std::clamp(tint.r,NXFloat(0),NXFloat(1));
    tint.g=std::clamp(tint.g,NXFloat(0),NXFloat(1));
    tint.b=std::clamp(tint.b,NXFloat(0),NXFloat(1));
    const NativeGlassAccentReference* variant=&nativeGlassAccentReferences.front();
    double nearest=std::numeric_limits<double>::infinity();
    for(const auto& candidate:nativeGlassAccentReferences) {
        double distance=0;
        for(size_t i=0;i<20;++i) {
            const double delta=double(candidate.vibrant.matrix.values[i])
                -profile.accent.vibrant.matrix.values[i];
            distance+=delta*delta;
        }
        if(distance<nearest) {nearest=distance;variant=&candidate;}
    }
    auto result=profile.accent.vibrant;
    result.matrix.values={};
    result.matrix.values[18]=1;
    const auto hsl=NativeGlassAccentDetail::hsl(tint);
    const double hue=hsl[0],saturation=hsl[1],lightness=hsl[2];
    if(saturation==0) {
        // normal(gray,.75), then dodge(.8+.2*gray), then burn(.2*gray).
        const double denominator=(.8+.2*lightness)*(1-.2*lightness);
        for(size_t row=0;row<3;++row) {
            result.matrix.values[row*5+row]=NXFloat(.25/denominator);
            result.matrix.values[row*5+4]=NXFloat((.95*lightness-.2)/denominator);
        }
        return result;
    }
    const bool darkRegular=variant->dark&&!variant->clear;
    const auto adjusted=NativeGlassAccentDetail::rgb(hue,
        saturation*(darkRegular?.95:1.2),lightness*(darkRegular?1.1:.6));
    const std::array<double,3> channels={tint.r,tint.g,tint.b};
    constexpr std::array<double,3> luma={.2126,.7152,.0722};
    for(size_t row=0;row<3;++row) {
        const double slope=darkRegular?(adjusted[row]-channels[row])/.6:
            (channels[row]-adjusted[row])/.85;
        for(size_t column=0;column<3;++column)
            result.matrix.values[row*5+column]=NXFloat(slope*luma[column]);
        result.matrix.values[row*5+4]=NXFloat(channels[row]-(darkRegular?.15:1)*slope);
    }
    return result;
}
}
