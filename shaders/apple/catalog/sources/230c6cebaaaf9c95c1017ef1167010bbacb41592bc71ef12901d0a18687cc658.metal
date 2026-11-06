#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const float SMOOTH_EDGE_PX_1 = 1.5;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float2 resolution_1;
float2 lensCenter_1;
float2 lensSize_1;
float cornerRadius_1;
float cornerCurve_1;
float externalShapeCoverage_1;
float maximumSampleRadius_1;
float refraction_1;
float curve_1;
float dispersion_1;
float saturation_1;
float contrast_1;
float luminosity_1;
float4 tint_1;
float tintMode_1;
float edge_1;
float specStrength_1;
float specFallbackStrength_1;
float specBrightFallbackStrength_1;
float2 specLightDirection_1;
float specDirectionalPower_1;
float specWidthPx_1;
float reflectionSamplePx_1;
float reflectionSpreadPx_1;
float brightBackdropShade_1;
float referenceAppearance_1;
float bodyMode_1;
float backdropLuma_1;
float edgeAbsorption_1;
float adaptiveTint_1;
float bodyCompression_1;
float neutralDetail_1;
float glassTintAmount_1;
float materialFrost_1;
float contentScale_1;
float filterScale_1;
float interactionIntensity_1;
float2 interactionLocation_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
float4 upper2x2_2;
float2 translation_2;
float4 decalBounds_3;
float4 upper2x2_4;
float2 translation_4;
float2 invDimensions_6;
float4 upper2x2_10;
float2 translation_10;
float4 decalBounds_11;
float4 upper2x2_12;
float2 translation_12;
float2 invDimensions_14;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 localCoordsVar [[user(locn1)]];
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct CombinedUniforms {
CombinedUniformData combinedUniformData[1];
};
struct IntrinsicUniforms {
float4 viewport;
float4 dstReadBounds;
};
struct Globals {
    const device CombinedUniforms* _anonInterface0;
    uint uniformSsboIndex;
    constant IntrinsicUniforms* _anonInterface1;
    sampler2D image_7;
    sampler2D image_15;
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 LocalMatrix_2_h4f2(thread Globals& _globals, float2 pos) {
float2 _1_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _2_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _1_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_2_e = saturate(_2_e + 0.5h);
return (((_2_e.x * _2_e.y) * _2_e.z) * _2_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _1_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
}
half4 sk_decal_image_11_h4f2f4(float2 b, float4 c);
float2 boundedBackdropXY_1_f2f2f2(thread Globals& _globals, float2 origin, float2 sampleXY);
half4 sampleBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY);
half4 sampleSharpBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float boxRoundedSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float toBrightness_1_fh3(half3 c);
float3 tintLinear_1_f3f3(float3 color);
float3 tintSRGB_1_f3f3(float3 color);
float3 nativeTintColor_1_f3f3f3(thread Globals& _globals, float3 color, float3 transmitted);
half3 processColor_1_h3h3ffff4(half3 src, float vibrancy, float intensity, float luminosityAdjustment, float4 overlay);
float directionalHighlight_1_ff2(float2 normal);
half3 reflectionColor_1_h3h3f2h3(half3 backdrop, float2 normal, half3 bodyColor);
half3 sampleReflection_1_h3f2f2ff2f2h3(thread Globals& _globals, float2 xy, float2 normal, float sdf, float2 lensCenter, float2 halfDim, half3 fallback);
half4 referenceGlass_1_h4f2f2ff2ff2(float2 xy, float2 halfDim, float r, float2 p, float sdf, float2 normal);
float2 boundedBackdropXY_1_f2f2f2(thread Globals& _globals, float2 origin, float2 sampleXY) {
float2 offset = sampleXY - origin;
float offsetLength = length(offset);
float scale = min(1.0, max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].maximumSampleRadius_1, 0.0) / max(offsetLength, 0.0001));
return origin + offset * scale;
}
half4 sampleBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY) {
return LocalMatrix_2_h4f2(_globals, boundedBackdropXY_1_f2f2f2(_globals, origin, sampleXY) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0));
}
half4 sampleSharpBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY) {
float2 _6_pos = boundedBackdropXY_1_f2f2f2(_globals, origin, sampleXY) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float2 _7_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * _6_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10;
half4 _8_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _7_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_8_e = saturate(_8_e + 0.5h);
return (((_8_e.x * _8_e.y) * _8_e.z) * _8_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _7_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12));
}
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r) {
float2 q = (abs(p) - halfDim) + r;
return (length(max(q, 0.0)) + min(max(q.x, q.y), 0.0)) - r;
}
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle) {
float2 radial = p - circle.xy;
float2 join = se.zw - circle.xy;
if (radial.x * join.y - radial.y * join.x > 0.0) {
float len = length(radial);
return float3(len - circle.z, radial / max(len, 1e-06));
}
float m = max(max(p.x, p.y), 1e-06);
float norm = m * powr(powr(p.x / m, se.y) + powr(p.y / m, se.y), 1.0 / se.y);
float2 gradient = powr(p / max(norm, 1e-06), float2(se.y - 1.0));
float magnitude = max(length(gradient), 1e-06);
return float3((norm - se.x) / magnitude, gradient / magnitude);
}
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim) {
float2 q = abs(p);
float c = halfDim.x - halfDim.y;
float3 result;
if (q.y + c >= q.x) result = continuousOctant_1_f3f2f4f4(float2(q.x, q.y + c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTop_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTopCircle_1); else {
result = continuousOctant_1_f3f2f4f4(float2(q.y, q.x - c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRight_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRightCircle_1);
result.yz = result.zy;
}
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
result.yz = result.yz * direction;
return result;
}
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim).yz;
float2 q = (abs(p) - halfDim) + r;
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
if (max(q.x, q.y) > 0.0) {
float2 n = max(q, 0.0);
return (direction * n) / max(length(n), 1e-06);
}
return q.x > q.y ? float2(direction.x, 0.0) : float2(0.0, direction.y);
}
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _9_q = (abs(p) - halfDim) + r;
return float3((length(max(_9_q, 0.0)) + min(max(_9_q.x, _9_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float toBrightness_1_fh3(half3 c) {
return float(dot(c, half3(0.2126h, 0.7152h, 0.0722h)));
}
float3 nativeTintColor_1_f3f3f3(thread Globals& _globals, float3 color, float3 transmitted) {
float amount = clamp(toBrightness_1_fh3(half3(transmitted)), 0.0, 1.0);
float red = max(color.x - max(color.y, color.z), 0.0);
float green = max(color.y - max(color.x, color.z), 0.0);
float gain = (0.395 + 0.265 * red) - 0.045 * green;
float bias = (-0.014 - 0.025 * red) + 0.022 * green;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 > 1.5) {
float chroma = max(transmitted.x, max(transmitted.y, transmitted.z)) - min(transmitted.x, min(transmitted.y, transmitted.z));
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1 < 0.5) {
float size = smoothstep(52.0, 120.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y));
float luma = clamp(amount - (((0.15 * chroma) * 4.0) * amount) * (1.0 - amount), 0.0, 1.0);
float knee = mix(0.54, 0.24, size);
float response = ((1.0 - exp(-luma / knee)) / (1.0 - exp(-1.0 / knee))) * mix(1.0, 0.61, size);
response *= mix(1.0, 0.46, max(0.0, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].glassTintAmount_1 - 0.5) * 2.0));
float3 base = color - (color * (1.0 - color)) * 0.02;
return base * (1.0 - 0.02 * response) + ((1.0 - base) * (0.12 + 0.09 * red)) * response;
}
amount = powr(clamp(amount - (((0.263 * chroma) * 4.0) * amount) * (1.0 - amount), 0.0, 1.0), 1.14);
float preferenceBase = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].glassTintAmount_1 <= 0.5 ? 0.3 + 0.2 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].glassTintAmount_1 : 0.4 + 0.3 * (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].glassTintAmount_1 - 0.5);
float base = preferenceBase * (1.0 - 0.5875 * red);
amount = base + (1.0 - base) * amount;
gain = (0.395 + 0.5 * red) - 0.045 * green;
bias = (-0.014 - 0.043 * red) + 0.022 * green;
}
float3 linear = mix(color * 0.07739938, powr((color + 0.055) * 0.947867334, float3(2.4)), step(float3(0.04045), color));
float3 shaded = max(linear * gain + bias, float3(0.0));
float3 _10_color = mix(shaded, linear, amount);
_10_color = max(_10_color, float3(0.0));
return clamp(mix(_10_color * 12.92, 1.055 * powr(_10_color, float3(0.416666657)) - 0.055, step(float3(0.0031308), _10_color)), 0.0, 1.0);
}
half3 sampleReflection_1_h3f2f2ff2f2h3(thread Globals& _globals, float2 xy, float2 normal, float sdf, float2 lensCenter, float2 halfDim, half3 fallback) {
float2 boundaryXY = xy - normal * sdf;
float2 frameRemaining = max(halfDim - abs(boundaryXY - lensCenter), float2(0.0));
float2 absNormal = abs(normal);
float toFrameX = absNormal.x > 0.0001 ? frameRemaining.x / absNormal.x : 100000.0;
float toFrameY = absNormal.y > 0.0001 ? frameRemaining.y / absNormal.y : 100000.0;
float toFrame = min(toFrameX, toFrameY);
float2 tangent = float2(-normal.y, normal.x);
float2 reflectionXY = boundaryXY + normal * (toFrame + max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].reflectionSamplePx_1, 0.0));
float spread = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].reflectionSpreadPx_1, 0.0);
half4 center = sampleBackdrop_1_h4f2f2(_globals, xy, reflectionXY);
half4 before = sampleBackdrop_1_h4f2f2(_globals, xy, reflectionXY - tangent * spread);
half4 after = sampleBackdrop_1_h4f2f2(_globals, xy, reflectionXY + tangent * spread);
half4 reflected = center * 0.5h + (before + after) * 0.25h;
half3 _16_backdrop = mix(fallback, reflected.xyz, clamp(reflected.w, 0.0h, 1.0h));
float _17_luminance = toBrightness_1_fh3(_16_backdrop);
half3 _18_saturated = half3(float3(_16_backdrop) / max(_17_luminance, 0.001));
_18_saturated = mix(_16_backdrop, _18_saturated, 0.75h);
float _19_colorfulness = length(float3(_16_backdrop) - float3(_17_luminance));
float _20_colorMix = clamp(_19_colorfulness + 0.35, 0.35, 0.9);
float _21_targetBrightness = clamp(_17_luminance * 1.2, 0.0, 1.0);
half3 _22_reflected = half3(clamp(mix(float3(half3(half(_21_targetBrightness))), float3(_18_saturated), _20_colorMix), 0.0, 1.0));
float _23_directionLength = max(length(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specLightDirection_1), 0.0001);
float2 _24_lightDirection = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specLightDirection_1 / _23_directionLength;
float _25_highlightPower = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specDirectionalPower_1, 1.0);
float _26_key = powr(max(dot(normal, _24_lightDirection), 0.0), _25_highlightPower);
float _27_kick = powr(max(dot(normal, -_24_lightDirection), 0.0), _25_highlightPower);
float _28_directionality = clamp(_26_key + _27_kick, 0.0, 1.0);
float _29_maxChannel = float(max(_16_backdrop.x, max(_16_backdrop.y, _16_backdrop.z)));
float _30_blackFallback = 1.0 - smoothstep(0.0, 0.015, _29_maxChannel);
_22_reflected = half3(mix(float3(_22_reflected), float3(fallback), _30_blackFallback));
_22_reflected = half3(mix(float3(_22_reflected), float3(1.0), (_30_blackFallback * _28_directionality) * clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specFallbackStrength_1, 0.0, 1.0)));
float _31_minChannel = float(min(_16_backdrop.x, min(_16_backdrop.y, _16_backdrop.z)));
float _32_chroma = _29_maxChannel - _31_minChannel;
float _33_brightNeutral = smoothstep(0.85, 0.98, _17_luminance) * (1.0 - smoothstep(0.02, 0.12, _32_chroma));
float _34_brightVisibility = 0.15 + _28_directionality * 0.85;
return half3(mix(float3(_22_reflected), float3(0.0), (_33_brightNeutral * _34_brightVisibility) * clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specBrightFallbackStrength_1, 0.0, 1.0)));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 xy = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float2 halfDim = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1 * 0.5;
float r = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRadius_1, min(halfDim.x, halfDim.y));
float2 p = xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1;
float3 surface = roundedBoxSample_1_f3f2f2f(_globals, p, halfDim, r);
float sdf = surface.x;
if (sdf > SMOOTH_EDGE_PX_1) {
return LocalMatrix_2_h4f2(_globals, pos);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].referenceAppearance_1 > 0.5) {
float _35_pixelWidth = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
float _36_distanceInside = max(-sdf, 0.0);
float2 _37_lightDirection = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specLightDirection_1 / max(length(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specLightDirection_1), 0.0001);
float _38_bevel = min((18.0 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].refraction_1) * 1.11111116, max(r, 0.0));
float _39_slope = _38_bevel > 0.0001 ? clamp(1.0 - _36_distanceInside / _38_bevel, 0.0, 1.0) : 0.0;
float _40_normalZ = sqrt(max(0.0, 1.0 - _39_slope * _39_slope));
float _41_reach = min(2.0 * min(halfDim.x, halfDim.y), max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].maximumSampleRadius_1, 0.0));
float _42_u = _38_bevel > 0.0001 ? clamp(_36_distanceInside / _38_bevel, 0.0, 1.0) : 1.0;
float _43_transfer = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? _39_slope * _39_slope : (1.0 - (_42_u * _42_u) * (3.0 - 2.0 * _42_u)) * (1.0 - _42_u * _42_u);
float _44_displacement = min((float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? 40.0 : 20.0) * (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].curve_1 * 1.81818175)) * _43_transfer, _41_reach);
float2 _45_sampleXY = xy - surface.yz * _44_displacement;
float2 _46_chromaticShift = ((surface.yz * _44_displacement) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dispersion_1) * 0.3;
half4 _47_pixel = sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY);
half3 _48_frostedRGB = _47_pixel.xyz;
half4 _49_sharpPixel = _47_pixel;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dispersion_1 > 0.0 && _39_slope > 0.0) {
half4 _50_red = sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + _46_chromaticShift);
half4 _51_blue = sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - _46_chromaticShift);
_47_pixel = half4(_50_red.x, _47_pixel.y, _51_blue.z, _47_pixel.w);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].neutralDetail_1 > 0.0) {
float _52_smallDetail = 1.0 - smoothstep(64.0, 160.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y));
_49_sharpPixel = sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY);
float _53_detailRadius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].materialFrost_1, 4.5 * smoothstep(80.0, 160.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y)));
half4 _54_softDetail = _49_sharpPixel;
if (_53_detailRadius > 0.0 && _52_smallDetail > 0.0) {
_54_softDetail = _49_sharpPixel * 0.25h;
_54_softDetail += (((sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(_53_detailRadius, 0.0)) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(_53_detailRadius, 0.0))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(0.0, _53_detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(0.0, _53_detailRadius))) * 0.125h;
_54_softDetail += (((sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(_53_detailRadius)) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(_53_detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(_53_detailRadius, -_53_detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(_53_detailRadius, -_53_detailRadius))) * 0.0625h;
}
half3 _55_detail = _54_softDetail.xyz - _48_frostedRGB;
_55_detail = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1 > 0.5 ? max(_55_detail, half3(0.0h)) : min(_55_detail, half3(0.0h));
_47_pixel.xyz = _47_pixel.xyz + half3((float3(_55_detail) * _52_smallDetail) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].neutralDetail_1);
}
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1 > 0.5) && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].materialFrost_1 > 0.0) {
float _56_radius = min(20.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].materialFrost_1 * 8.0);
half4 _57_wider = (((sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(_56_radius, 0.0)) + sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(_56_radius, 0.0))) + sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY + float2(0.0, _56_radius))) + sampleBackdrop_1_h4f2f2(_globals, xy, _45_sampleXY - float2(0.0, _56_radius))) * 0.25h;
half3 _58_centerRGB = _47_pixel.xyz / max(_47_pixel.w, 0.0001h);
half3 _59_widerRGB = _57_wider.xyz / max(_57_wider.w, 0.0001h);
half3 _60_chromaDifference = _59_widerRGB - _58_centerRGB;
_60_chromaDifference -= half3(half(toBrightness_1_fh3(_60_chromaDifference)));
_47_pixel.xyz = _47_pixel.xyz + (_60_chromaDifference * _47_pixel.w) * 0.5h;
}
half4 _61_background = half4(0.0h);
if (_47_pixel.w <= 0.0h || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 <= 0.5) _61_background = sampleSharpBackdrop_1_h4f2f2(_globals, xy, xy);
if (_47_pixel.w <= 0.0h) _47_pixel = _61_background;
_47_pixel.xyz = _47_pixel.xyz / max(_47_pixel.w, 0.0001h);
float _62_transmittedLuma = toBrightness_1_fh3(_47_pixel.xyz);
float _63_vibrancy = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].saturation_1;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].neutralDetail_1 > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].saturation_1 > 1.0) {
half3 _64_sharpRGB = _49_sharpPixel.xyz / max(_49_sharpPixel.w, 0.0001h);
float _65_sharpChroma = float(max(_64_sharpRGB.x, max(_64_sharpRGB.y, _64_sharpRGB.z)) - min(_64_sharpRGB.x, min(_64_sharpRGB.y, _64_sharpRGB.z)));
float _66_bodyChroma = float(max(_47_pixel.x, max(_47_pixel.y, _47_pixel.z)) - min(_47_pixel.x, min(_47_pixel.y, _47_pixel.z)));
_63_vibrancy = 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].saturation_1 - 1.0) * clamp(_66_bodyChroma / max(_65_sharpChroma, 0.0001), 0.0, 1.0);
}
half3 _67_body = half3(mix(float3(half3(half(_62_transmittedLuma))), float3(_47_pixel.xyz), _63_vibrancy));
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5) _67_body = clamp(_67_body, 0.0h, 1.0h);
_67_body = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? half3(clamp((float3(_67_body - 0.5h) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contrast_1 + 0.5) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].luminosity_1, 0.0, 1.0)) : half3(float3(_67_body) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contrast_1 + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].luminosity_1);
float _68_contextLuma = 0.0;
float _69_tintAlpha = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].adaptiveTint_1 > 0.0) {
float2 _70_span = halfDim * 0.6;
float _71_scale = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
_68_contextLuma = toBrightness_1_fh3(LocalMatrix_2_h4f2(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1 * _71_scale).xyz);
_68_contextLuma += toBrightness_1_fh3(LocalMatrix_2_h4f2(_globals, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1 + float2(-_70_span.x, -_70_span.y)) * _71_scale).xyz);
_68_contextLuma += toBrightness_1_fh3(LocalMatrix_2_h4f2(_globals, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1 + float2(_70_span.x, -_70_span.y)) * _71_scale).xyz);
_68_contextLuma += toBrightness_1_fh3(LocalMatrix_2_h4f2(_globals, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1 + float2(-_70_span.x, _70_span.y)) * _71_scale).xyz);
_68_contextLuma += toBrightness_1_fh3(LocalMatrix_2_h4f2(_globals, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1 + _70_span) * _71_scale).xyz);
_68_contextLuma *= 0.2;
_69_tintAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].adaptiveTint_1 * (1.0 - exp(-_68_contextLuma * 9.090909)), 0.0, 1.0);
}
half3 _72_directTint = half3(mix(float3(_67_body), float3(half3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz)), _69_tintAlpha));
half3 _73_lumaTint = half3(clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz + toBrightness_1_fh3(_67_body)) - toBrightness_1_fh3(half3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz)), 0.0, 1.0));
float _74_chroma = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.x, max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.z)) - min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.z));
_67_body = half3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 > 0.5 ? mix(float3(_67_body), float3(half3(nativeTintColor_1_f3f3f3(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz, float3(_47_pixel.xyz)))), _69_tintAlpha) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? float3(_72_directTint) : mix(float3(_72_directTint), mix(float3(_67_body), float3(_73_lumaTint), _69_tintAlpha), clamp(_74_chroma * 8.0, 0.0, 1.0))));
float _75_bodyLuma = clamp(toBrightness_1_fh3(_67_body), 0.0, 1.0);
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1 < 0.5) && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 < 0.5) _67_body *= half(1.0 - (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyCompression_1 * _75_bodyLuma) * _75_bodyLuma);
_67_body = clamp(_67_body, 0.0h, 1.0h);
_75_bodyLuma = clamp(toBrightness_1_fh3(_67_body), 0.0, 1.0);
float _76_along = dot(surface.yz, _37_lightDirection);
float _77_rimThickness = 1.0 - sqrt(max(0.0, 1.0 - (1.0 - _40_normalZ) * (1.0 - _40_normalZ)));
float _78_absorption = (sqrt(max(_77_rimThickness, 0.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edgeAbsorption_1) * mix(1.4, 0.6, _76_along * 0.5 + 0.5);
half3 _79_result = half3(float3(_67_body) * max(0.0, 1.0 - _78_absorption));
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5) {
float _80_width = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1 * 1.25, 0.0001);
float _81_inset = max(0.0, _36_distanceInside / _80_width - 0.55);
float _82_lobe = powr(abs(_76_along), max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specDirectionalPower_1, 1.0));
float _83_rimLight = (((((_82_lobe * powr(max(1.0 - _75_bodyLuma, 0.0), 0.3)) * (0.1539 * exp(-_81_inset * 4.0) + 0.0569 * exp(-_81_inset * 0.4))) * (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specStrength_1 * 1.923077)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1) * mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specFallbackStrength_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specBrightFallbackStrength_1, _75_bodyLuma)) * 1.923077;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1 <= 0.0) _83_rimLight = 0.0;
_83_rimLight += ((((((_82_lobe * smoothstep(0.8, 0.98, _75_bodyLuma)) * smoothstep(0.5, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1)) * 0.03) * exp(-_36_distanceInside * 1.66666663)) * (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specStrength_1 * 1.923077)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1) * float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1 > 0.0);
_79_result += half3(half(_83_rimLight));
} else {
float _84_hairline = clamp((max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1, 0.0) - _36_distanceInside) / _35_pixelWidth + 0.5, 0.0, 1.0);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1 <= 0.0) _84_hairline = 0.0;
_84_hairline *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1;
float _85_power = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specDirectionalPower_1, 1.0);
float _86_lobe = powr(max(_76_along, 0.0), _85_power) + 0.85 * powr(max(-_76_along, 0.0), _85_power);
float _87_inset = max(0.0, _36_distanceInside - 0.75);
float _88_core = exp(-_87_inset * 2.0);
float _89_tail = exp(-_87_inset * 0.375);
float _90_rimLight = ((((_86_lobe * powr(max(1.0 - _75_bodyLuma, 0.0), 0.3)) * (0.175 * _88_core + 0.035 * _89_tail)) * (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specStrength_1 * 1.923077)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1) * (1.0 - _84_hairline);
if (_90_rimLight > 0.0001) {
half3 _91_reflected = sampleReflection_1_h3f2f2ff2f2h3(_globals, xy, surface.yz, sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1, halfDim, _67_body);
_79_result += half3(float3(half3(0.7h) + _91_reflected * 0.3h) * _90_rimLight);
}
float _92_lineStrength = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specFallbackStrength_1 + _68_contextLuma * 0.5, 0.0, 1.0);
_79_result = half3(mix(float3(_79_result), float3(1.0), _84_hairline * _92_lineStrength));
}
_79_result *= half(1.0 - smoothstep(0.85, 0.98, toBrightness_1_fh3(_79_result)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].brightBackdropShade_1);
float _93_glow = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionIntensity_1, 0.0, 1.0);
if (_93_glow > 0.0) {
float _94_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y) * 1.6);
float _95_sheen = 1.0 - smoothstep(0.0, _94_glowRadius, length(xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionLocation_1));
float _96_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1));
half3 _97_restingResult = _79_result;
_79_result = half3(mix(float3(_79_result), float3(1.0), _93_glow * _96_ambientPressOpacity));
_79_result = half3(mix(float3(_79_result), float3(1.0), (_93_glow * _95_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1)));
float _98_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y));
float _99_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y);
float _100_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 < 0.5) * smoothstep(160.0, 280.0, _99_longSide)) * smoothstep(2.5, 4.0, _99_longSide / _98_shortSide);
float _101_peak = float(max(_97_restingResult.x, max(_97_restingResult.y, _97_restingResult.z)));
float _102_chromaticRange = _101_peak - float(min(_97_restingResult.x, min(_97_restingResult.y, _97_restingResult.z)));
float _103_neutralPress = 1.0 - smoothstep(0.01, 0.1, _102_chromaticRange);
float _104_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1);
float _105_pressGain = mix(1.18 - 0.06 * _101_peak, 1.06, _104_lightAppearance);
half3 _106_colorPress = half3(mix(float3(half3(half(toBrightness_1_fh3(_97_restingResult)))), float3(_97_restingResult), mix(1.03, 1.06, _104_lightAppearance)) * _105_pressGain);
_106_colorPress += half3(half(_103_neutralPress * mix(0.02745098, 0.04509804, _104_lightAppearance)));
_79_result = half3(mix(float3(_79_result), mix(float3(_97_restingResult), float3(_106_colorPress), _93_glow), _100_platterPress));
}
float _107_alpha = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 > 0.5 ? 1.0 : clamp(0.5 - sdf / _35_pixelWidth, 0.0, 1.0);
return half4(mix(float4(_61_background), float4(half4(clamp(_79_result, 0.0h, 1.0h) * _47_pixel.w, _47_pixel.w)), _107_alpha));
}
float2 normal = surface.yz;
float2 sampleXY = xy;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].refraction_1 > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].curve_1 > 0.0) {
float minDim = min(halfDim.x, halfDim.y);
float requestedDepth = minDim * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].refraction_1;
float safeDepth = max(min(requestedDepth, max(r, 1.0)), 1.0);
float depth = clamp(-sdf / safeDepth, 0.0, 1.0);
float curvature = 1.0 - depth;
float bend = 1.0 - sqrt(1.0 - curvature * curvature);
sampleXY = xy - ((bend * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].curve_1) * minDim) * normal;
}
half4 pixel;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dispersion_1 > 0.0) {
float2 normP = p / halfDim;
float2 shift = ((((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dispersion_1 * normP) * normP) * normP) * min(halfDim.x, halfDim.y)) * 0.1;
float2 xyR = sampleXY - shift;
float2 xyG = sampleXY;
float2 xyB = sampleXY + shift;
float2 _108_p = xyR - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1;
float sdfR = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0 ? circularBoxSDF_1_ff2f2f(_108_p, halfDim, r) : continuousCornerSample_1_f3f2f2(_globals, _108_p, halfDim).x;
float2 _109_p = xyB - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1;
float sdfB = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0 ? circularBoxSDF_1_ff2f2f(_109_p, halfDim, r) : continuousCornerSample_1_f3f2f2(_globals, _109_p, halfDim).x;
half4 gVal = sampleBackdrop_1_h4f2f2(_globals, xy, xyG);
half4 rVal = sdfR <= 0.0 ? sampleBackdrop_1_h4f2f2(_globals, xy, xyR) : gVal;
half4 bVal = sdfB <= 0.0 ? sampleBackdrop_1_h4f2f2(_globals, xy, xyB) : gVal;
pixel = half4(rVal.x, gVal.y, bVal.z, gVal.w);
} else {
pixel = sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY);
}
if (pixel.w <= 0.0h) {
pixel = sampleBackdrop_1_h4f2f2(_globals, xy, xy);
}
float4 _110_overlay = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1;
float _111_mono = toBrightness_1_fh3(pixel.xyz);
half3 _112_vibrant = half3(clamp(mix(float3(half3(half(_111_mono))), float3(pixel.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].saturation_1), 0.0, 1.0));
half3 _113_adjusted = half3(clamp(float3(_112_vibrant - 0.5h) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contrast_1 + 0.5, 0.0, 1.0));
half3 _114_luminous = half3(clamp(float3(_113_adjusted) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].luminosity_1, 0.0, 1.0));
pixel.xyz = half3(mix(float3(_114_luminous), float3(half3(_110_overlay.xyz)), _110_overlay.w));
float brightBody = smoothstep(0.85, 0.98, toBrightness_1_fh3(pixel.xyz));
pixel.xyz = pixel.xyz * half(1.0 - brightBody * clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].brightBackdropShade_1, 0.0, 0.1));
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1 > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specStrength_1 > 0.0) {
float rimBand = smoothstep(-max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specWidthPx_1, 1.0), 0.0, sdf);
if (rimBand > 0.0) {
float fresnel = rimBand * rimBand;
float reflectance = clamp((fresnel * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].specStrength_1) * clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].edge_1, 0.0, 1.0), 0.0, 0.9);
half3 reflected = sampleReflection_1_h3f2f2ff2f2h3(_globals, xy, normal, sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1, halfDim, pixel.xyz);
pixel.xyz = half3(mix(float3(pixel.xyz), float3(reflected), reflectance));
}
}
float alpha = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 > 0.5 ? 1.0 : 1.0 - smoothstep(-0.75, 0.75, sdf);
if (alpha >= 1.0) return pixel;
half4 background = sampleBackdrop_1_h4f2f2(_globals, xy, xy);
return half4(mix(float4(background), float4(pixel), alpha));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_15_Tex [[texture(1)]], sampler image_15_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_15_Tex, image_15_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
