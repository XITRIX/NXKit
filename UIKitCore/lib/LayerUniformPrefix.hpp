#pragma once
#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace NXKit::Internal {
// The pending clear is still in the draw list. Reading its value avoids a
// snapshot that would flush that clear separately from the first content draw.
inline sk_sp<SkShader> uniformLayerPrefix(SkDevice* device) {
    static const bool disabled=[] {
        const auto flag=std::getenv("NXKIT_SNAPSHOT_MASK_PREFIX");
        return flag&&flag[0]=='1';
    }();
    if(disabled||!device||device->imageInfo().alphaType()!=kPremul_SkAlphaType)return nullptr;
    const auto graphite=device->asGraphiteDevice();
    if(!graphite)return nullptr;
    const auto format=skgpu::graphite::TextureInfoPriv::ViewFormat(graphite->target().proxy()->textureInfo());
    using skgpu::graphite::TextureFormat;
    const auto type=device->imageInfo().colorType();
    if(!((format==TextureFormat::kRGBA8||format==TextureFormat::kBGRA8)
          &&(type==kRGBA_8888_SkColorType||type==kBGRA_8888_SkColorType))
       &&!(format==TextureFormat::kRGBA16F&&type==kRGBA_F16_SkColorType))return nullptr;
    std::array<float,4> color;
    if(!graphite->peekPendingClearColor(&color))return nullptr;
    const auto stored=device->imageInfo().makeWH(1,1);
    const auto floats=SkImageInfo::Make(1,1,kRGBA_F32_SkColorType,kPremul_SkAlphaType,
                                      stored.refColorSpace());
    alignas(16) std::array<unsigned char,16> pixel;
    if(!SkConvertPixels(stored,pixel.data(),stored.minRowBytes(),floats,color.data(),sizeof(color))
       ||!SkConvertPixels(floats,color.data(),sizeof(color),stored,pixel.data(),stored.minRowBytes())
       ||!std::all_of(color.begin(),color.end(),[](float v){return std::isfinite(v);}))return nullptr;
    static const auto effect=SkRuntimeEffect::MakeForShader(SkString(
        "uniform float4 storedColor;half4 main(float2 p){return half4(storedColor);}")).effect;
    if(!effect)return nullptr;
    // Raw premultiplied device values: no unpremultiply or color-space round trip.
    SkRuntimeShaderBuilder builder(effect);builder.uniform("storedColor").set(color.data(),color.size());
    return builder.makeShader();
}
}
