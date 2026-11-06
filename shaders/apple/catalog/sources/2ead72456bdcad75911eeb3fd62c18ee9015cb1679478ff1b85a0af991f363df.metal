#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<float2, 2> material_resolution_1;
array<float2, 2> material_lensCenter_1;
array<float2, 2> material_lensSize_1;
array<float, 2> material_cornerRadius_1;
array<float, 2> material_cornerCurve_1;
float externalShapeCoverage_1;
array<float, 2> material_maximumSampleRadius_1;
array<float, 2> material_refraction_1;
array<float, 2> material_curve_1;
array<float, 2> material_dispersion_1;
array<float, 2> material_saturation_1;
array<float, 2> material_contrast_1;
array<float, 2> material_luminosity_1;
array<float4, 2> material_tint_1;
array<float, 2> material_tintMode_1;
array<float, 2> material_edge_1;
array<float, 2> material_specStrength_1;
array<float, 2> material_specFallbackStrength_1;
array<float, 2> material_specBrightFallbackStrength_1;
array<float2, 2> material_specLightDirection_1;
array<float, 2> material_specDirectionalPower_1;
array<float, 2> material_specWidthPx_1;
array<float, 2> material_reflectionSamplePx_1;
array<float, 2> material_reflectionSpreadPx_1;
array<float, 2> material_brightBackdropShade_1;
array<float, 2> material_referenceAppearance_1;
array<float, 2> material_bodyMode_1;
array<float, 2> material_backdropLuma_1;
array<float, 2> material_edgeAbsorption_1;
array<float, 2> material_adaptiveTint_1;
array<float, 2> material_bodyCompression_1;
array<float, 2> material_neutralDetail_1;
array<float, 2> material_materialFrost_1;
float contentScale_1;
float filterScale_1;
array<float, 2> material_interactionIntensity_1;
array<float2, 2> material_interactionLocation_1;
array<float4, 2> inverseX_1;
array<float4, 2> inverseY_1;
array<float4, 2> forwardX_1;
array<float4, 2> forwardY_1;
array<float4, 2> geometry_1;
array<float, 2> itemAlpha_1;
array<float, 2> itemShadow_1;
float elevationSigma_1;
float elevationOpacity_1;
float2 elevationOffset_1;
float4 elevationBasis_1;
float mergeSpacing_1;
array<float4, 2> material_cornerTop_1;
array<float4, 2> material_cornerTopCircle_1;
array<float4, 2> material_cornerRight_1;
array<float4, 2> material_cornerRightCircle_1;
float4 color_2;
float4 upper2x2_3;
float2 translation_3;
float4 decalBounds_4;
float4 upper2x2_5;
float2 translation_5;
float2 invDimensions_7;
float4 upper2x2_11;
float2 translation_11;
float4 decalBounds_12;
float4 upper2x2_13;
float2 translation_13;
float2 invDimensions_15;
float4 upper2x2_19;
float2 translation_19;
float4 decalBounds_20;
float4 upper2x2_21;
float2 translation_21;
float2 invDimensions_23;
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
    sampler2D image_8;
    sampler2D image_16;
    sampler2D image_24;
    float2 resolution_1;
    float2 lensCenter_1;
    float2 lensSize_1;
    float cornerRadius_1;
    float cornerCurve_1;
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
    float materialFrost_1;
    float interactionIntensity_1;
    float2 interactionLocation_1;
    array<float, 2> weights_1;
    array<float, 2> distances_1;
    float2 mergedNormal_1;
    float2 materialNormal_1;
    bool geometryBlended_1;
    float3 materialForwardX_1;
    float3 materialForwardY_1;
    int activeMaterial_1;
    float4 cornerTop_1;
    float4 cornerTopCircle_1;
    float4 cornerRight_1;
    float4 cornerRightCircle_1;
};
half4 sk_decal_image_4_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_12_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_20_h4f2f4(float2 b, float4 c);
float2 groupXY_1_f2f2(thread Globals& _globals, float2 xy);
half4 groupBackdrop_1_h4f2(thread Globals& _globals, float2 xy);
half4 groupSharp_1_h4f2(float2 xy);
float2 boundedBackdropXY_1_f2f2f2(float2 origin, float2 sampleXY);
half4 sampleBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY);
half4 sampleSharpBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 itemDistance0_1_f3f2(float2 xy);
float3 itemDistance1_1_f3f2(float2 xy);
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record);
void materialOwnership_1_v();
float glassShadowTail_1_ff(float z);
float glassRimDarkening_1_fff2fff2fffff(float rimDistance, float2 normal, float sceneLuma, float dark, float2 light, float power, float width, float scale, float strength, float shortSide);
float toBrightness_1_fh3(half3 c);
float3 tintLinear_1_f3f3(float3 color);
float3 tintSRGB_1_f3f3(float3 color);
float3 nativeTintColor_1_f3f3f3(thread Globals& _globals, float3 color, float3 transmitted);
float directionalHighlight_1_ff2(float2 normal);
half3 reflectionColor_1_h3h3f2h3(half3 backdrop, float2 normal, half3 bodyColor);
half3 sampleReflection_1_h3f2f2ff2f2h3(float2 xy, float2 normal, float sdf, float2 lensCenter, float2 halfDim, half3 fallback);
half4 referenceGlass_1_h4f2f2ff2ff2(thread Globals& _globals, float2 xy, float2 halfDim, float r, float2 p, float sdf, float2 normal);
float2 groupXY_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals.materialForwardX_1, p), dot(_globals.materialForwardY_1, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = groupXY_1_f2f2(_globals, xy);
if (_globals.activeMaterial_1 == 0) {
float2 _9_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11;
half4 _10_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_12 - _9_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_10_e = saturate(_10_e + 0.5h);
return (((_10_e.x * _10_e.y) * _10_e.z) * _10_e.w) * sample(_globals.image_16, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_15 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_13.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_13.zw) * _9_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_13));
}
if (_globals.activeMaterial_1 == 1) {
float2 _11_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_19.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_19.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_19;
half4 _12_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_20 - _11_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_12_e = saturate(_12_e + 0.5h);
return (((_12_e.x * _12_e.y) * _12_e.z) * _12_e.w) * sample(_globals.image_24, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_23 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_21.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_21.zw) * _11_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_21));
}
return half4(0.0h);
}
half4 sampleBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY) {
float2 _16_offset = sampleXY - origin;
float _17_offsetLength = length(_16_offset);
float _18_scale = min(1.0, max(_globals.maximumSampleRadius_1, 0.0) / max(_17_offsetLength, 0.0001));
return groupBackdrop_1_h4f2(_globals, (origin + _16_offset * _18_scale) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0));
}
half4 sampleSharpBackdrop_1_h4f2f2(thread Globals& _globals, float2 origin, float2 sampleXY) {
float2 _19_offset = sampleXY - origin;
float _20_offsetLength = length(_19_offset);
float _21_scale = min(1.0, max(_globals.maximumSampleRadius_1, 0.0) / max(_20_offsetLength, 0.0001));
float2 _22_pos = groupXY_1_f2f2(_globals, (origin + _19_offset * _21_scale) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0));
float2 _23_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * _22_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3;
half4 _24_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_4 - _23_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_24_e = saturate(_24_e + 0.5h);
return (((_24_e.x * _24_e.y) * _24_e.z) * _24_e.w) * sample(_globals.image_8, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_7 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.zw) * _23_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_5));
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
if (q.y + c >= q.x) result = continuousOctant_1_f3f2f4f4(float2(q.x, q.y + c), _globals.cornerTop_1, _globals.cornerTopCircle_1); else {
result = continuousOctant_1_f3f2f4f4(float2(q.y, q.x - c), _globals.cornerRight_1, _globals.cornerRightCircle_1);
result.yz = result.zy;
}
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
result.yz = result.yz * direction;
return result;
}
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals.cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim).yz;
float2 q = (abs(p) - halfDim) + r;
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
if (max(q.x, q.y) > 0.0) {
float2 n = max(q, 0.0);
return (direction * n) / max(length(n), 1e-06);
}
return q.x > q.y ? float2(direction.x, 0.0) : float2(0.0, direction.y);
}
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals.cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _25_q = (abs(p) - halfDim) + r;
return float3((length(max(_25_q, 0.0)) + min(max(_25_q.x, _25_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record) {
float4 _26_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _27_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _26_g.xy * 0.5;
_globals.cornerCurve_1 = _26_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _28_r = min(_26_g.z, min(_26_g.x, _26_g.y) * 0.5);
float3 _29_surface = roundedBoxSample_1_f3f2f2f(_globals, _27_p, _26_g.xy * 0.5, _28_r);
float _30_d = _29_surface.x;
float2 _31_n = _29_surface.yz;
float2 _32_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _31_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _31_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _31_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _31_n.y);
float _33_magnitude = max(length(_32_gradient), 0.0001);
float3 first = float3(_30_d / _33_magnitude, _32_gradient / _33_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _34_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _35_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _34_g.xy * 0.5;
_globals.cornerCurve_1 = _34_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _36_r = min(_34_g.z, min(_34_g.x, _34_g.y) * 0.5);
float3 _37_surface = roundedBoxSample_1_f3f2f2f(_globals, _35_p, _34_g.xy * 0.5, _36_r);
float _38_d = _37_surface.x;
float2 _39_n = _37_surface.yz;
float2 _40_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _39_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _39_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _39_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _39_n.y);
float _41_magnitude = max(length(_40_gradient), 0.0001);
float3 item = float3(_38_d / _41_magnitude, _40_gradient / _41_magnitude);
float other = item.x;
if (record) _globals.distances_1[1] = other;
float k = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * clamp(0.5 - 0.5 * dot(item.yz, _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001)), 0.0, 1.0);
float h = k > 0.0001 ? clamp(0.5 + (0.5 * (other - d)) / k, 0.0, 1.0) : float(d <= other);
if (record) _globals.weights_1[0] *= h;
if (record) {
_globals.weights_1[1] = 1.0 - h;
_globals.geometryBlended_1 = _globals.geometryBlended_1 || h > 0.0 && h < 1.0;
}
_globals.mergedNormal_1 = mix(item.yz, _globals.mergedNormal_1, h);
d = mix(other, d, h) - (k * h) * (1.0 - h);
}
return d;
}
float glassRimDarkening_1_fff2fff2fffff(float rimDistance, float2 normal, float sceneLuma, float dark, float2 light, float power, float width, float scale, float strength, float shortSide) {
float pixel = 1.0 / max(scale, 0.0001);
float band = clamp((max(width, 0.0) * 0.625 - rimDistance) / pixel + 0.5, 0.0, 1.0);
if ((band <= 0.0 || width <= 0.0) || strength <= 0.0) return 0.0;
float along = abs(dot(normal, light / max(length(light), 0.0001)));
float direction = 1.0 - mix(0.7, 1.0, dark) * powr(along, max(power, 1.0));
float bright = powr(clamp((sceneLuma - 0.5) * 2.0, 0.0, 1.0), 1.5);
float large = clamp((shortSide - 52.0) * 0.0147058824, 0.0, 1.0);
float darkDensity = (0.24 + 0.31 * bright) + large * (0.05 + 0.17 * smoothstep(0.5, 0.65, sceneLuma));
float density = mix(0.25, darkDensity, dark);
return ((band * direction) * density) * max(strength, 0.0);
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
if (_globals.tintMode_1 > 1.5) {
float chroma = max(transmitted.x, max(transmitted.y, transmitted.z)) - min(transmitted.x, min(transmitted.y, transmitted.z));
if (_globals.backdropLuma_1 < 0.5) {
float size = smoothstep(52.0, 120.0, min(_globals.lensSize_1.x, _globals.lensSize_1.y));
float luma = clamp(amount - (((0.15 * chroma) * 4.0) * amount) * (1.0 - amount), 0.0, 1.0);
float knee = mix(0.54, 0.24, size);
float response = ((1.0 - exp(-luma / knee)) / (1.0 - exp(-1.0 / knee))) * mix(1.0, 0.61, size);
float3 base = color - (color * (1.0 - color)) * 0.02;
return base * (1.0 - 0.02 * response) + ((1.0 - base) * (0.12 + 0.09 * red)) * response;
}
amount = powr(clamp(amount - (((0.263 * chroma) * 4.0) * amount) * (1.0 - amount), 0.0, 1.0), 1.14);
float base = 0.4 - 0.235 * red;
amount = base + (1.0 - base) * amount;
gain = (0.395 + 0.5 * red) - 0.045 * green;
bias = (-0.014 - 0.043 * red) + 0.022 * green;
}
float3 linear = mix(color * 0.07739938, powr((color + 0.055) * 0.947867334, float3(2.4)), step(float3(0.04045), color));
float3 shaded = max(linear * gain + bias, float3(0.0));
float3 _42_color = mix(shaded, linear, amount);
_42_color = max(_42_color, float3(0.0));
return clamp(mix(_42_color * 12.92, 1.055 * powr(_42_color, float3(0.416666657)) - 0.055, step(float3(0.0031308), _42_color)), 0.0, 1.0);
}
half4 referenceGlass_1_h4f2f2ff2ff2(thread Globals& _globals, float2 xy, float2 halfDim, float r, float2 p, float sdf, float2 normal) {
float pixelWidth = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
float distanceInside = max(-sdf, 0.0);
float2 lightDirection = _globals.specLightDirection_1 / max(length(_globals.specLightDirection_1), 0.0001);
float bevel = min((18.0 * _globals.refraction_1) * 1.11111116, max(r, 0.0));
float slope = bevel > 0.0001 ? clamp(1.0 - distanceInside / bevel, 0.0, 1.0) : 0.0;
float normalZ = sqrt(max(0.0, 1.0 - slope * slope));
float reach = min(2.0 * min(halfDim.x, halfDim.y), max(_globals.maximumSampleRadius_1, 0.0));
float u = bevel > 0.0001 ? clamp(distanceInside / bevel, 0.0, 1.0) : 1.0;
float transfer = _globals.bodyMode_1 > 0.5 ? slope * slope : (1.0 - (u * u) * (3.0 - 2.0 * u)) * (1.0 - u * u);
float displacement = min((float(_globals.bodyMode_1 > 0.5 ? 40.0 : 20.0) * (_globals.curve_1 * 1.81818175)) * transfer, reach);
float2 sampleXY = xy - normal * displacement;
float2 chromaticShift = ((normal * displacement) * _globals.dispersion_1) * 0.3;
half4 pixel = sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY);
half3 frostedRGB = pixel.xyz;
half4 sharpPixel = pixel;
if (_globals.dispersion_1 > 0.0 && slope > 0.0) {
half4 red = sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY + chromaticShift);
half4 blue = sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY - chromaticShift);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
if (_globals.neutralDetail_1 > 0.0) {
float smallDetail = 1.0 - smoothstep(64.0, 160.0, min(_globals.lensSize_1.x, _globals.lensSize_1.y));
sharpPixel = sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY);
float detailRadius = min(_globals.materialFrost_1, 4.5 * smoothstep(80.0, 160.0, min(_globals.lensSize_1.x, _globals.lensSize_1.y)));
half4 softDetail = sharpPixel;
if (detailRadius > 0.0 && smallDetail > 0.0) {
softDetail = sharpPixel * 0.25h;
softDetail += (((sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(detailRadius, 0.0)) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(detailRadius, 0.0))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(0.0, detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(0.0, detailRadius))) * 0.125h;
softDetail += (((sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(detailRadius)) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(detailRadius, -detailRadius))) + sampleSharpBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(detailRadius, -detailRadius))) * 0.0625h;
}
half3 detail = softDetail.xyz - frostedRGB;
detail = _globals.backdropLuma_1 > 0.5 ? max(detail, half3(0.0h)) : min(detail, half3(0.0h));
pixel.xyz = pixel.xyz + half3((float3(detail) * smallDetail) * _globals.neutralDetail_1);
}
if ((_globals.bodyMode_1 < 0.5 && _globals.backdropLuma_1 > 0.5) && _globals.materialFrost_1 > 0.0) {
float radius = min(20.0, _globals.materialFrost_1 * 8.0);
half4 wider = (((sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(radius, 0.0)) + sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(radius, 0.0))) + sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY + float2(0.0, radius))) + sampleBackdrop_1_h4f2f2(_globals, xy, sampleXY - float2(0.0, radius))) * 0.25h;
half3 centerRGB = pixel.xyz / max(pixel.w, 0.0001h);
half3 widerRGB = wider.xyz / max(wider.w, 0.0001h);
half3 chromaDifference = widerRGB - centerRGB;
chromaDifference -= half3(half(toBrightness_1_fh3(chromaDifference)));
pixel.xyz = pixel.xyz + (chromaDifference * pixel.w) * 0.5h;
}
half4 background = half4(0.0h);
if (pixel.w <= 0.0h || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 <= 0.5) background = sampleSharpBackdrop_1_h4f2f2(_globals, xy, xy);
if (pixel.w <= 0.0h) pixel = background;
pixel.xyz = pixel.xyz / max(pixel.w, 0.0001h);
float transmittedLuma = toBrightness_1_fh3(pixel.xyz);
float vibrancy = _globals.saturation_1;
if (_globals.neutralDetail_1 > 0.0 && _globals.saturation_1 > 1.0) {
half3 sharpRGB = sharpPixel.xyz / max(sharpPixel.w, 0.0001h);
float sharpChroma = float(max(sharpRGB.x, max(sharpRGB.y, sharpRGB.z)) - min(sharpRGB.x, min(sharpRGB.y, sharpRGB.z)));
float bodyChroma = float(max(pixel.x, max(pixel.y, pixel.z)) - min(pixel.x, min(pixel.y, pixel.z)));
vibrancy = 1.0 + (_globals.saturation_1 - 1.0) * clamp(bodyChroma / max(sharpChroma, 0.0001), 0.0, 1.0);
}
half3 body = half3(mix(float3(half3(half(transmittedLuma))), float3(pixel.xyz), vibrancy));
if (_globals.bodyMode_1 > 0.5) body = clamp(body, 0.0h, 1.0h);
body = _globals.bodyMode_1 > 0.5 ? half3(clamp((float3(body - 0.5h) * _globals.contrast_1 + 0.5) + _globals.luminosity_1, 0.0, 1.0)) : half3(float3(body) * _globals.contrast_1 + _globals.luminosity_1);
float contextLuma = 0.0;
float tintAlpha = _globals.tint_1.w;
if (_globals.adaptiveTint_1 > 0.0) {
float2 span = halfDim * 0.6;
float scale = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
contextLuma = toBrightness_1_fh3(groupBackdrop_1_h4f2(_globals, _globals.lensCenter_1 * scale).xyz);
contextLuma += toBrightness_1_fh3(groupBackdrop_1_h4f2(_globals, (_globals.lensCenter_1 + float2(-span.x, -span.y)) * scale).xyz);
contextLuma += toBrightness_1_fh3(groupBackdrop_1_h4f2(_globals, (_globals.lensCenter_1 + float2(span.x, -span.y)) * scale).xyz);
contextLuma += toBrightness_1_fh3(groupBackdrop_1_h4f2(_globals, (_globals.lensCenter_1 + float2(-span.x, span.y)) * scale).xyz);
contextLuma += toBrightness_1_fh3(groupBackdrop_1_h4f2(_globals, (_globals.lensCenter_1 + span) * scale).xyz);
contextLuma *= 0.2;
tintAlpha = clamp(_globals.tint_1.w + _globals.adaptiveTint_1 * (1.0 - exp(-contextLuma * 9.090909)), 0.0, 1.0);
}
half3 directTint = half3(mix(float3(body), float3(half3(_globals.tint_1.xyz)), tintAlpha));
half3 lumaTint = half3(clamp((_globals.tint_1.xyz + toBrightness_1_fh3(body)) - toBrightness_1_fh3(half3(_globals.tint_1.xyz)), 0.0, 1.0));
float chroma = max(_globals.tint_1.x, max(_globals.tint_1.y, _globals.tint_1.z)) - min(_globals.tint_1.x, min(_globals.tint_1.y, _globals.tint_1.z));
body = half3(_globals.tintMode_1 > 0.5 ? mix(float3(body), float3(half3(nativeTintColor_1_f3f3f3(_globals, _globals.tint_1.xyz, float3(pixel.xyz)))), tintAlpha) : (_globals.bodyMode_1 > 0.5 ? float3(directTint) : mix(float3(directTint), mix(float3(body), float3(lumaTint), tintAlpha), clamp(chroma * 8.0, 0.0, 1.0))));
float bodyLuma = clamp(toBrightness_1_fh3(body), 0.0, 1.0);
if ((_globals.bodyMode_1 < 0.5 && _globals.backdropLuma_1 < 0.5) && _globals.tintMode_1 < 0.5) body *= half(1.0 - (_globals.bodyCompression_1 * bodyLuma) * bodyLuma);
body = clamp(body, 0.0h, 1.0h);
bodyLuma = clamp(toBrightness_1_fh3(body), 0.0, 1.0);
float along = dot(normal, lightDirection);
float rimThickness = 1.0 - sqrt(max(0.0, 1.0 - (1.0 - normalZ) * (1.0 - normalZ)));
float absorption = (sqrt(max(rimThickness, 0.0)) * _globals.edgeAbsorption_1) * mix(1.4, 0.6, along * 0.5 + 0.5);
half3 result = half3(float3(body) * max(0.0, 1.0 - absorption));
if (_globals.bodyMode_1 < 0.5) {
float width = max(_globals.specWidthPx_1 * 1.25, 0.0001);
float inset = max(0.0, distanceInside / width - 0.55);
float lobe = powr(abs(along), max(_globals.specDirectionalPower_1, 1.0));
float rimLight = (((((lobe * powr(max(1.0 - bodyLuma, 0.0), 0.3)) * (0.1539 * exp(-inset * 4.0) + 0.0569 * exp(-inset * 0.4))) * (_globals.specStrength_1 * 1.923077)) * _globals.edge_1) * mix(_globals.specFallbackStrength_1, _globals.specBrightFallbackStrength_1, bodyLuma)) * 1.923077;
if (_globals.specWidthPx_1 <= 0.0) rimLight = 0.0;
rimLight += ((((((lobe * smoothstep(0.8, 0.98, bodyLuma)) * smoothstep(0.5, 0.85, _globals.backdropLuma_1)) * 0.03) * exp(-distanceInside * 1.66666663)) * (_globals.specStrength_1 * 1.923077)) * _globals.edge_1) * float(_globals.specWidthPx_1 > 0.0);
result += half3(half(rimLight));
} else {
float hairline = clamp((max(_globals.specWidthPx_1, 0.0) - distanceInside) / pixelWidth + 0.5, 0.0, 1.0);
if (_globals.specWidthPx_1 <= 0.0) hairline = 0.0;
hairline *= _globals.edge_1;
float power = max(_globals.specDirectionalPower_1, 1.0);
float lobe = powr(max(along, 0.0), power) + 0.85 * powr(max(-along, 0.0), power);
float inset = max(0.0, distanceInside - 0.75);
float core = exp(-inset * 2.0);
float tail = exp(-inset * 0.375);
float rimLight = ((((lobe * powr(max(1.0 - bodyLuma, 0.0), 0.3)) * (0.175 * core + 0.035 * tail)) * (_globals.specStrength_1 * 1.923077)) * _globals.edge_1) * (1.0 - hairline);
if (rimLight > 0.0001) {
float2 _67_boundaryXY = xy - normal * sdf;
float2 _68_frameRemaining = max(halfDim - abs(_67_boundaryXY - _globals.lensCenter_1), float2(0.0));
float2 _69_absNormal = abs(normal);
float _70_toFrameX = _69_absNormal.x > 0.0001 ? _68_frameRemaining.x / _69_absNormal.x : 100000.0;
float _71_toFrameY = _69_absNormal.y > 0.0001 ? _68_frameRemaining.y / _69_absNormal.y : 100000.0;
float _72_toFrame = min(_70_toFrameX, _71_toFrameY);
float2 _73_tangent = float2(-normal.y, normal.x);
float2 _74_reflectionXY = _67_boundaryXY + normal * (_72_toFrame + max(_globals.reflectionSamplePx_1, 0.0));
float _75_spread = max(_globals.reflectionSpreadPx_1, 0.0);
half4 _76_center = sampleBackdrop_1_h4f2f2(_globals, xy, _74_reflectionXY);
half4 _77_before = sampleBackdrop_1_h4f2f2(_globals, xy, _74_reflectionXY - _73_tangent * _75_spread);
half4 _78_after = sampleBackdrop_1_h4f2f2(_globals, xy, _74_reflectionXY + _73_tangent * _75_spread);
half4 _79_reflected = _76_center * 0.5h + (_77_before + _78_after) * 0.25h;
half3 _80_backdrop = mix(body, _79_reflected.xyz, clamp(_79_reflected.w, 0.0h, 1.0h));
float _81_luminance = toBrightness_1_fh3(_80_backdrop);
half3 _82_saturated = half3(float3(_80_backdrop) / max(_81_luminance, 0.001));
_82_saturated = mix(_80_backdrop, _82_saturated, 0.75h);
float _83_colorfulness = length(float3(_80_backdrop) - float3(_81_luminance));
float _84_colorMix = clamp(_83_colorfulness + 0.35, 0.35, 0.9);
float _85_targetBrightness = clamp(_81_luminance * 1.2, 0.0, 1.0);
half3 _86_reflected = half3(clamp(mix(float3(half3(half(_85_targetBrightness))), float3(_82_saturated), _84_colorMix), 0.0, 1.0));
float _87_directionLength = max(length(_globals.specLightDirection_1), 0.0001);
float2 _88_lightDirection = _globals.specLightDirection_1 / _87_directionLength;
float _89_highlightPower = max(_globals.specDirectionalPower_1, 1.0);
float _90_key = powr(max(dot(normal, _88_lightDirection), 0.0), _89_highlightPower);
float _91_kick = powr(max(dot(normal, -_88_lightDirection), 0.0), _89_highlightPower);
float _92_directionality = clamp(_90_key + _91_kick, 0.0, 1.0);
float _93_maxChannel = float(max(_80_backdrop.x, max(_80_backdrop.y, _80_backdrop.z)));
float _94_blackFallback = 1.0 - smoothstep(0.0, 0.015, _93_maxChannel);
_86_reflected = half3(mix(float3(_86_reflected), float3(body), _94_blackFallback));
_86_reflected = half3(mix(float3(_86_reflected), float3(1.0), (_94_blackFallback * _92_directionality) * clamp(_globals.specFallbackStrength_1, 0.0, 1.0)));
float _95_minChannel = float(min(_80_backdrop.x, min(_80_backdrop.y, _80_backdrop.z)));
float _96_chroma = _93_maxChannel - _95_minChannel;
float _97_brightNeutral = smoothstep(0.85, 0.98, _81_luminance) * (1.0 - smoothstep(0.02, 0.12, _96_chroma));
float _98_brightVisibility = 0.15 + _92_directionality * 0.85;
half3 reflected = half3(mix(float3(_86_reflected), float3(0.0), (_97_brightNeutral * _98_brightVisibility) * clamp(_globals.specBrightFallbackStrength_1, 0.0, 1.0)));
result += half3(float3(half3(0.7h) + reflected * 0.3h) * rimLight);
}
float lineStrength = clamp(_globals.specFallbackStrength_1 + contextLuma * 0.5, 0.0, 1.0);
result = half3(mix(float3(result), float3(1.0), hairline * lineStrength));
}
result *= half(1.0 - smoothstep(0.85, 0.98, toBrightness_1_fh3(result)) * _globals.brightBackdropShade_1);
float glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0);
if (glow > 0.0) {
float glowRadius = max(1.0, min(_globals.lensSize_1.x, _globals.lensSize_1.y) * 1.6);
float sheen = 1.0 - smoothstep(0.0, glowRadius, length(xy - _globals.interactionLocation_1));
float ambientPressOpacity = _globals.bodyMode_1 > 0.5 ? mix(0.14, 0.22, _globals.backdropLuma_1) : (_globals.tintMode_1 > 1.5 ? 0.3 : mix(0.385, 0.47, _globals.backdropLuma_1));
half3 restingResult = result;
result = half3(mix(float3(result), float3(1.0), glow * ambientPressOpacity));
result = half3(mix(float3(result), float3(1.0), (glow * sheen) * mix(0.07, 0.1, _globals.backdropLuma_1)));
float shortSide = max(1.0, min(_globals.lensSize_1.x, _globals.lensSize_1.y));
float longSide = max(_globals.lensSize_1.x, _globals.lensSize_1.y);
float platterPress = (float(_globals.bodyMode_1 < 0.5 && _globals.tintMode_1 < 0.5) * smoothstep(160.0, 280.0, longSide)) * smoothstep(2.5, 4.0, longSide / shortSide);
float peak = float(max(restingResult.x, max(restingResult.y, restingResult.z)));
float chromaticRange = peak - float(min(restingResult.x, min(restingResult.y, restingResult.z)));
float neutralPress = 1.0 - smoothstep(0.01, 0.1, chromaticRange);
float lightAppearance = smoothstep(0.15, 0.85, _globals.backdropLuma_1);
float pressGain = mix(1.18 - 0.06 * peak, 1.06, lightAppearance);
half3 colorPress = half3(mix(float3(half3(half(toBrightness_1_fh3(restingResult)))), float3(restingResult), mix(1.03, 1.06, lightAppearance)) * pressGain);
colorPress += half3(half(neutralPress * mix(0.02745098, 0.04509804, lightAppearance)));
result = half3(mix(float3(result), mix(float3(restingResult), float3(colorPress), glow), platterPress));
}
float alpha = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 > 0.5 ? 1.0 : clamp(0.5 - sdf / pixelWidth, 0.0, 1.0);
return half4(mix(float4(background), float4(half4(clamp(result, 0.0h, 1.0h) * pixel.w, pixel.w)), alpha));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 xy = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float rawDistance = mergeGeometry_1_ff2b(_globals, xy, true);
float2 sceneNormal = _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001);
float sceneDistance = rawDistance;
if (_globals.geometryBlended_1) {
float epsilon = max(0.05, 0.25 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
float dx = mergeGeometry_1_ff2b(_globals, xy + float2(epsilon, 0.0), false) - mergeGeometry_1_ff2b(_globals, xy - float2(epsilon, 0.0), false);
float dy = mergeGeometry_1_ff2b(_globals, xy + float2(0.0, epsilon), false) - mergeGeometry_1_ff2b(_globals, xy - float2(0.0, epsilon), false);
float2 gradient = float2(dx, dy) / (2.0 * epsilon);
float magnitude = length(gradient);
if (magnitude > 0.0001) sceneNormal = gradient / magnitude;
sceneDistance = rawDistance / max(magnitude, 0.05);
}
{
float _99_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _99_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _99_cover;
_globals.weights_1[1] *= 1.0 - _99_cover;
_globals.weights_1[0] += _99_cover;
}
{
float _100_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _100_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _100_cover;
_globals.weights_1[1] *= 1.0 - _100_cover;
_globals.weights_1[1] += _100_cover;
}
{
float _101_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _101_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _101_cover;
_globals.weights_1[1] *= 1.0 - _101_cover;
_globals.weights_1[0] += _101_cover;
}
{
float _102_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _102_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _102_cover;
_globals.weights_1[1] *= 1.0 - _102_cover;
_globals.weights_1[1] += _102_cover;
}
float2 _103_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3;
half4 _104_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_4 - _103_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_104_e = saturate(_104_e + 0.5h);
half4 original = (((_104_e.x * _104_e.y) * _104_e.z) * _104_e.w) * sample(_globals.image_8, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_7 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.zw) * _103_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_5));
float materialShadow = 0.0;
materialShadow += _globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemShadow_1[0];
materialShadow += _globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemShadow_1[1];
float coverage = clamp(0.5 - sceneDistance * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float shadow = 0.0;
if ((coverage < 1.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationOpacity_1 > 0.0) && materialShadow > 0.0) {
float _0_distance = mergeGeometry_1_ff2b(_globals, xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationOffset_1, false);
float2 normal = _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001);
float2 sigmaDirection = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationBasis_1.x * normal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationBasis_1.z * normal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationBasis_1.y * normal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationBasis_1.w * normal.y);
float sigma = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationSigma_1 * max(length(sigmaDirection), 0.0001);
float _105_z = _0_distance / sigma;
float _106_t = 1.0 / (1.0 + 0.2316419 * abs(_105_z));
float _107_q = ((0.3989423 * exp((-0.5 * _105_z) * _105_z)) * _106_t) * (0.319381535 + _106_t * (-0.356563777 + _106_t * (1.78147793 + _106_t * (-1.82125592 + _106_t * 1.33027446))));
shadow = clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].elevationOpacity_1 * materialShadow) * (_105_z >= 0.0 ? _107_q : 1.0 - _107_q), 0.0, 1.0);
}
float contour = 0.0;
if (coverage < 1.0) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_referenceAppearance_1[0] > 0.5) {
float2 gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y * sceneNormal.y);
float metric = max(length(gradient), 0.0001);
float luma = float(dot(original.xyz / max(original.w, 0.0001h), half3(0.2126h, 0.7152h, 0.0722h)));
contour += (_globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0]) * glassRimDarkening_1_fff2fff2fffff(sceneDistance / metric, gradient / metric, luma, float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0] < 0.5), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specLightDirection_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specDirectionalPower_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specWidthPx_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * metric, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edge_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edgeAbsorption_1[0]) * 28.5714283, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].y));
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_referenceAppearance_1[1] > 0.5) {
float2 gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y * sceneNormal.y);
float metric = max(length(gradient), 0.0001);
float luma = float(dot(original.xyz / max(original.w, 0.0001h), half3(0.2126h, 0.7152h, 0.0722h)));
contour += (_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * glassRimDarkening_1_fff2fff2fffff(sceneDistance / metric, gradient / metric, luma, float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1] < 0.5), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specLightDirection_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specDirectionalPower_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specWidthPx_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * metric, (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edge_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edgeAbsorption_1[1]) * 28.5714283, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].y));
}
}
half3 contourLoss = min(half3(half(contour)) * original.w, original.xyz);
if (coverage <= 0.0) return half4(max(original.xyz * half(1.0 - shadow) - contourLoss, half3(0.0h)), original.w + half(shadow) * (1.0h - original.w));
half4 result = original;
float groupInteraction = 0.0;
float2 groupTouch = xy;
{
float intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[0] * clamp(0.5 - _globals.distances_1[0] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (intensity > groupInteraction) {
groupInteraction = intensity;
float3 touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[0], 1.0);
groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, touch));
}
}
{
float intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[1] * clamp(0.5 - _globals.distances_1[1] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (intensity > groupInteraction) {
groupInteraction = intensity;
float3 touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[1], 1.0);
groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, touch));
}
}
if (_globals.weights_1[0] > 0.0) {
_globals.resolution_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_resolution_1[0];
_globals.lensCenter_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensCenter_1[0];
_globals.lensSize_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0];
_globals.cornerRadius_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[0];
_globals.cornerCurve_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerCurve_1[0];
_globals.maximumSampleRadius_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[0];
_globals.refraction_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_refraction_1[0];
_globals.curve_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_curve_1[0];
_globals.dispersion_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_dispersion_1[0];
_globals.saturation_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_saturation_1[0];
_globals.contrast_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_contrast_1[0];
_globals.luminosity_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_luminosity_1[0];
_globals.tint_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0];
_globals.tintMode_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0];
_globals.edge_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edge_1[0];
_globals.specStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specStrength_1[0];
_globals.specFallbackStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specFallbackStrength_1[0];
_globals.specBrightFallbackStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specBrightFallbackStrength_1[0];
_globals.specLightDirection_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specLightDirection_1[0];
_globals.specDirectionalPower_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specDirectionalPower_1[0];
_globals.specWidthPx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specWidthPx_1[0];
_globals.reflectionSamplePx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_reflectionSamplePx_1[0];
_globals.reflectionSpreadPx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_reflectionSpreadPx_1[0];
_globals.brightBackdropShade_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_brightBackdropShade_1[0];
_globals.referenceAppearance_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_referenceAppearance_1[0];
_globals.bodyMode_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0];
_globals.backdropLuma_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0];
_globals.edgeAbsorption_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edgeAbsorption_1[0];
_globals.adaptiveTint_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_adaptiveTint_1[0];
_globals.bodyCompression_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyCompression_1[0];
_globals.neutralDetail_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_neutralDetail_1[0];
_globals.materialFrost_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_materialFrost_1[0];
_globals.interactionIntensity_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[0];
_globals.interactionLocation_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[0];
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
_globals.activeMaterial_1 = 0;
_globals.materialForwardX_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz;
_globals.materialForwardY_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz;
float2 localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0)));
float2 localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y * sceneNormal.y);
float localMetric = max(length(localGradient), 0.0001);
_globals.materialNormal_1 = localGradient / localMetric;
_globals.interactionIntensity_1 = groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(groupTouch, 1.0)));
half4 color = referenceGlass_1_h4f2f2ff2ff2(_globals, localXY, _globals.lensSize_1 * 0.5, min(_globals.cornerRadius_1, min(_globals.lensSize_1.x, _globals.lensSize_1.y) * 0.5), localXY - _globals.lensCenter_1, sceneDistance / localMetric, _globals.materialNormal_1);
result += half(_globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0]) * (color - original);
}
if (_globals.weights_1[1] > 0.0) {
_globals.resolution_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_resolution_1[1];
_globals.lensCenter_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensCenter_1[1];
_globals.lensSize_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1];
_globals.cornerRadius_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[1];
_globals.cornerCurve_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerCurve_1[1];
_globals.maximumSampleRadius_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[1];
_globals.refraction_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_refraction_1[1];
_globals.curve_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_curve_1[1];
_globals.dispersion_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_dispersion_1[1];
_globals.saturation_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_saturation_1[1];
_globals.contrast_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_contrast_1[1];
_globals.luminosity_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_luminosity_1[1];
_globals.tint_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1];
_globals.tintMode_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1];
_globals.edge_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edge_1[1];
_globals.specStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specStrength_1[1];
_globals.specFallbackStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specFallbackStrength_1[1];
_globals.specBrightFallbackStrength_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specBrightFallbackStrength_1[1];
_globals.specLightDirection_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specLightDirection_1[1];
_globals.specDirectionalPower_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specDirectionalPower_1[1];
_globals.specWidthPx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_specWidthPx_1[1];
_globals.reflectionSamplePx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_reflectionSamplePx_1[1];
_globals.reflectionSpreadPx_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_reflectionSpreadPx_1[1];
_globals.brightBackdropShade_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_brightBackdropShade_1[1];
_globals.referenceAppearance_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_referenceAppearance_1[1];
_globals.bodyMode_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1];
_globals.backdropLuma_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1];
_globals.edgeAbsorption_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_edgeAbsorption_1[1];
_globals.adaptiveTint_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_adaptiveTint_1[1];
_globals.bodyCompression_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyCompression_1[1];
_globals.neutralDetail_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_neutralDetail_1[1];
_globals.materialFrost_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_materialFrost_1[1];
_globals.interactionIntensity_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[1];
_globals.interactionLocation_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[1];
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
_globals.activeMaterial_1 = 1;
_globals.materialForwardX_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz;
_globals.materialForwardY_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz;
float2 localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0)));
float2 localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y * sceneNormal.y);
float localMetric = max(length(localGradient), 0.0001);
_globals.materialNormal_1 = localGradient / localMetric;
_globals.interactionIntensity_1 = groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(groupTouch, 1.0)));
half4 color = referenceGlass_1_h4f2f2ff2ff2(_globals, localXY, _globals.lensSize_1 * 0.5, min(_globals.cornerRadius_1, min(_globals.lensSize_1.x, _globals.lensSize_1.y) * 0.5), localXY - _globals.lensCenter_1, sceneDistance / localMetric, _globals.materialNormal_1);
result += half(_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * (color - original);
}
half4 color = mix(original, result, half(coverage));
return half4(max((color.xyz - original.xyz * half(shadow * (1.0 - coverage))) - contourLoss * half(1.0 - coverage), half3(0.0h)), color.w + half(shadow * (1.0 - coverage)) * (1.0h - color.w));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_8_Tex [[texture(0)]], sampler image_8_Smplr [[sampler(0)]], texture2d<half> image_16_Tex [[texture(1)]], sampler image_16_Smplr [[sampler(1)]], texture2d<half> image_24_Tex [[texture(2)]], sampler image_24_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_8_Tex, image_8_Smplr}, {image_16_Tex, image_16_Smplr}, {image_24_Tex, image_24_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
