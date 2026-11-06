#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<float2, 4> material_resolution_1;
array<float2, 4> material_lensCenter_1;
array<float2, 4> material_lensSize_1;
array<float, 4> material_cornerRadius_1;
array<float, 4> material_cornerCurve_1;
float externalShapeCoverage_1;
array<float, 4> material_maximumSampleRadius_1;
float contentScale_1;
float filterScale_1;
array<float, 4> material_interactionIntensity_1;
array<float2, 4> material_interactionLocation_1;
array<float, 4> material_bodyMode_1;
array<float, 4> material_tintMode_1;
array<float, 4> material_referenceAppearance_1;
array<float, 4> material_backdropLuma_1;
array<float4, 4> material_tint_1;
array<float, 4> material_nativeFaceBlack_1;
array<float3, 4> material_nativeFaceTransfer_1;
array<float4, 4> material_nativeFaceFill_1;
array<float2, 4> material_nativeFaceLuma_1;
array<float, 4> material_nativeFaceOpacity_1;
array<float3, 4> material_nativeFillOpacity_1;
array<float4, 4> material_nativeBlurDistances_1;
array<float4, 4> material_nativeBlurOpacities_1;
array<float4, 4> material_nativeRefraction_1;
array<float3, 4> material_nativeRefractionMask_1;
array<float2, 4> material_nativeSDRGradient_1;
array<float, 4> material_nativeDisplayHeadroom_1;
array<float, 4> material_nativeAccentEnabled_1;
array<float4, 4> material_nativeAccent0_1;
array<float4, 4> material_nativeAccent1_1;
array<float4, 4> material_nativeAccent2_1;
array<float3, 4> material_nativeAccentDistances_1;
array<float4, 4> material_nativeAccentTiming0_1;
array<float4, 4> material_nativeAccentTiming1_1;
array<float2, 4> material_nativeAccentTimingKnown_1;
array<float2, 4> material_nativeAccentMaskOptions_1;
array<float4, 4> material_nativeAccentFill_1;
array<float4, 4> material_nativeAccentMatrix0_1;
array<float4, 4> material_nativeAccentMatrix1_1;
array<float4, 4> material_nativeAccentMatrix2_1;
array<float4, 4> material_nativeAccentMatrix3_1;
array<float4, 4> material_nativeAccentMatrixBias_1;
array<float3, 4> material_nativeAccentMatrixOptions_1;
array<float, 4> material_nativeDispersion_1;
array<float, 4> material_nativeEdgeAbsorption_1;
array<float, 4> material_nativeTintOpacity_1;
array<float, 4> material_nativeBrightBackdropShade_1;
array<float, 4> material_nativeDarkSpecularGain_1;
array<float, 4> material_nativeBrightSpecularGain_1;
array<float, 4> material_nativeReflectionDistance_1;
array<float, 4> material_nativeReflectionSpread_1;
array<float4, 4> material_backdropPyramidRect_1;
array<float4, 4> material_backdropPyramidInfo_1;
array<float, 4> material_nativePyramidStretch_1;
array<float, 4> material_nativeMinimumBlurRadius_1;
array<float4, 4> material_nativeBlurRadii_1;
array<float4, 4> inverseX_1;
array<float4, 4> inverseY_1;
array<float4, 4> forwardX_1;
array<float4, 4> forwardY_1;
array<float4, 4> geometry_1;
array<float4, 4> outputBounds_1;
array<float, 4> itemAlpha_1;
float mergeSpacing_1;
array<float4, 4> material_cornerTop_1;
array<float4, 4> material_cornerTopCircle_1;
array<float4, 4> material_cornerRight_1;
array<float4, 4> material_cornerRightCircle_1;
array<float2, 4> material_nativeOutputDistance_1;
array<float4, 4> material_nativeShadowShape_1;
array<float2, 4> material_nativeShadowOffset_1;
array<float4, 4> material_nativeShadowTransfer_1;
array<float4, 4> material_nativeShadowFill_1;
array<float4, 4> material_nativeShadowResponse_1;
array<float4, 4> material_nativeRingShadow_1;
array<float, 4> material_nativeRingShadowOffset_1;
array<float4, 4> material_nativeBleedShape_1;
array<float4, 4> material_nativeBleedTransfer_1;
array<float2, 4> material_nativeBleedDistance_1;
array<float4, 4> material_nativeFilterHighlight_1;
array<float3, 4> material_nativeFilterHighlightColor_1;
array<float4, 4> material_nativeKeyHighlight_1;
array<float4, 4> material_nativeFillHighlight_1;
array<float4, 4> material_nativeKeyHighlightShape_1;
array<float4, 4> material_nativeFillHighlightShape_1;
array<float4, 4> material_nativeKeyHighlightColor_1;
array<float4, 4> material_nativeFillHighlightColor_1;
array<float4, 4> material_nativeHighlightResponse_1;
array<float4, 4> material_nativeHighlightMatrix0_1;
array<float4, 4> material_nativeHighlightMatrix1_1;
array<float4, 4> material_nativeHighlightMatrix2_1;
array<float4, 4> material_nativeHighlightMatrix3_1;
array<float4, 4> material_nativeHighlightMatrixBias_1;
array<float4, 4> material_nativeHighlightMatrixOptions_1;
array<float3, 4> material_nativeLightingClamp_1;
array<float, 4> material_nativeLightingDisplayHeadroom_1;
array<float4, 4> material_nativeHoldingTone_1;
array<float3, 4> material_nativeFilterOptions_1;
float4 upper2x2_2;
float2 translation_2;
float4 decalBounds_3;
float4 upper2x2_4;
float2 translation_4;
float2 invImgSize_6;
float4 subsetInsetClamp_6;
float4 upper2x2_9;
float2 translation_9;
float4 decalBounds_10;
float4 upper2x2_11;
float2 translation_11;
float2 invImgSize_13;
float4 subsetInsetClamp_13;
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
    sampler2D image_6;
    sampler2D image_13;
    float cornerCurve_1;
    float interactionIntensity_1;
    float2 interactionLocation_1;
    array<float, 4> weights_1;
    array<float, 4> distances_1;
    float2 mergedNormal_1;
    float2 materialNormal_1;
    bool geometryBlended_1;
    float4 cornerTop_1;
    float4 cornerTopCircle_1;
    float4 cornerRight_1;
    float4 cornerRightCircle_1;
};
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c) {
return sample(c, a * b);
}
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_10_h4f2f4(float2 b, float4 c);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 itemDistance0_1_f3f2(float2 xy);
float3 itemDistance1_1_f3f2(float2 xy);
float3 itemDistance2_1_f3f2(float2 xy);
float3 itemDistance3_1_f3f2(float2 xy);
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record);
void materialOwnership_1_v();
float2 groupXY_material3_1_f2f2(thread Globals& _globals, float2 xy);
half4 groupBackdrop_material3_1_h4f2(thread Globals& _globals, float2 xy);
half4 groupSharp_material3_1_h4f2(thread Globals& _globals, float2 xy);
float toBrightness_material3_1_fh3(half3 c);
float nativeLightingAA_material3_1_f();
float nativeLightingHeadroom_material3_1_f();
float nativeLightingLuma_material3_1_ff3(float3 c);
float3 nativeLightingUnpremul_material3_1_f3h4(half4 c);
float nativeGlassOutputCoverage_material3_1_ff(thread Globals& _globals, float sdf);
float3 nativeLightingTransfer_material3_1_f3f3fff(float3 c, float black, float white, float saturation);
float3 nativeLightingClampColor_material3_1_f3f3ff(float3 c, float limit, float preserveHue);
float nativeLightingErfc_material3_1_ff(float valueInput);
float nativeLightingRingAlpha_material3_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal);
half4 nativeLightingOver_material3_1_h4h4h4(half4 foreground, half4 background);
float nativeLightingBias_material3_1_fff(float coverage, float amount);
float nativeLightingSDFBand_material3_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal);
float nativeLightingSDFDirection_material3_1_ff2ff(float2 normal, float angle, float spread);
float nativeGlassFilterHighlight_material3_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale);
half4 nativeGlassHoldingTone_material3_1_h4fh4(thread Globals& _globals, float sdf, half4 material);
half4 nativeGlassBackgroundFinalize_material3_1_h4fh4h4h3ff2f(float sdf, half4 material, half4 original, half3 exteriorFace, float faceOpacity, float2 normal, float localScale);
half4 nativeGlassExterior_material3_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop);
half3 nativeGlassBleed_material3_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop);
half4 nativeGlassRingComposite_material3_1_h4fff2fh4(float sdf, float shiftedRingSDF, float2 normal, float fieldAlpha, half4 material);
float4 nativeLightingHighlightMatrix_material3_1_f4f4(float4 c);
half3 nativeGlassHighlight_material3_1_h3ff2f2h3(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop);
float2 nativeSampleXY_material3_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target);
float2 nativeSamplingWorld_material3_1_f2f2(float2 xy);
float nativeRadiusLOD_material3_1_ff(thread Globals& _globals, float radius);
float2 nativePyramidPosition_material3_1_f2f2f(thread Globals& _globals, float2 worldXY, float level);
half4 nativePyramidAt_material3_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod);
half4 nativeAtRadius_material3_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius);
half4 nativeSharp_material3_1_h4f2f2(float2 origin, float2 target);
half4 nativeBleed_material3_1_h4f2f2(float2 origin, float2 target);
half4 nativeShadow_material3_1_h4f2f2(float2 origin, float2 target);
float3 nativeUnpremul_material3_1_f3h4(half4 p);
float nativeLinearRamp_material3_1_ffff(float x, float a, float b);
float nativeBlurCoverage_material3_1_ff(thread Globals& _globals, float d);
float3 nativeFace_material3_1_f3f3f(thread Globals& _globals, float3 color, float sdf);
float4 nativeAccentMatrix_material3_1_f4f4(float4 color);
float nativeCubic_material3_1_ffff(float t, float p1, float p2);
float nativeTiming_material3_1_fff4f(float x, float4 control, float known);
float4 nativeGradientColor_material3_1_f4f(thread Globals& _globals, float d);
float nativeBevel_material3_1_ffff(float _0_distance, float height, float radius);
half4 nativeGlassBackgroundBody_material3_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius);
half4 nativeGlassDecorate_material3_1_h4f2ff2f2fh4f(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, half4 material, float surfaceCoverage);
half4 nativeGlassSurface_material3_1_h4f2ff2f2fffh4ff(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, float shadowSDF, float ringSDF, half4 original, float coverage, float localScale);
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
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
return len == 0.0 && (_globals.cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
}
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
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
float2 normal = len == 0.0 && (_globals.cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
return float3(len - r, normal);
}
if (_globals.cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _16_q = (abs(p) - halfDim) + r;
return float3((length(max(_16_q, 0.0)) + min(max(_16_q.x, _16_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record) {
float4 _17_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _18_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _17_g.xy * 0.5;
_globals.cornerCurve_1 = _17_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _19_r = min(_17_g.z, min(_17_g.x, _17_g.y) * 0.5);
float3 _20_surface = roundedBoxSample_1_f3f2f2f(_globals, _18_p, _17_g.xy * 0.5, _19_r);
float _21_d = _20_surface.x;
float2 _22_n = _20_surface.yz;
float2 _23_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _22_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _22_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _22_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _22_n.y);
float _24_magnitude = max(length(_23_gradient), 0.0001);
float3 first = float3(_21_d / _24_magnitude, _23_gradient / _24_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _25_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _26_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _25_g.xy * 0.5;
_globals.cornerCurve_1 = _25_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _27_r = min(_25_g.z, min(_25_g.x, _25_g.y) * 0.5);
float3 _28_surface = roundedBoxSample_1_f3f2f2f(_globals, _26_p, _25_g.xy * 0.5, _27_r);
float _29_d = _28_surface.x;
float2 _30_n = _28_surface.yz;
float2 _31_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _30_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _30_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _30_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _30_n.y);
float _32_magnitude = max(length(_31_gradient), 0.0001);
float3 item = float3(_29_d / _32_magnitude, _31_gradient / _32_magnitude);
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
{
float4 _33_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2];
float2 _34_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0))) - _33_g.xy * 0.5;
_globals.cornerCurve_1 = _33_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[2];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[2];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[2];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[2];
float _35_r = min(_33_g.z, min(_33_g.x, _33_g.y) * 0.5);
float3 _36_surface = roundedBoxSample_1_f3f2f2f(_globals, _34_p, _33_g.xy * 0.5, _35_r);
float _37_d = _36_surface.x;
float2 _38_n = _36_surface.yz;
float2 _39_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].x * _38_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].x * _38_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].y * _38_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].y * _38_n.y);
float _40_magnitude = max(length(_39_gradient), 0.0001);
float3 item = float3(_37_d / _40_magnitude, _39_gradient / _40_magnitude);
float other = item.x;
if (record) _globals.distances_1[2] = other;
float k = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * clamp(0.5 - 0.5 * dot(item.yz, _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001)), 0.0, 1.0);
float h = k > 0.0001 ? clamp(0.5 + (0.5 * (other - d)) / k, 0.0, 1.0) : float(d <= other);
if (record) _globals.weights_1[0] *= h;
if (record) _globals.weights_1[1] *= h;
if (record) {
_globals.weights_1[2] = 1.0 - h;
_globals.geometryBlended_1 = _globals.geometryBlended_1 || h > 0.0 && h < 1.0;
}
_globals.mergedNormal_1 = mix(item.yz, _globals.mergedNormal_1, h);
d = mix(other, d, h) - (k * h) * (1.0 - h);
}
{
float4 _41_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[3];
float2 _42_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].xyz, float3(xy, 1.0))) - _41_g.xy * 0.5;
_globals.cornerCurve_1 = _41_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[3];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[3];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[3];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[3];
float _43_r = min(_41_g.z, min(_41_g.x, _41_g.y) * 0.5);
float3 _44_surface = roundedBoxSample_1_f3f2f2f(_globals, _42_p, _41_g.xy * 0.5, _43_r);
float _45_d = _44_surface.x;
float2 _46_n = _44_surface.yz;
float2 _47_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].x * _46_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].x * _46_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].y * _46_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].y * _46_n.y);
float _48_magnitude = max(length(_47_gradient), 0.0001);
float3 item = float3(_45_d / _48_magnitude, _47_gradient / _48_magnitude);
float other = item.x;
if (record) _globals.distances_1[3] = other;
float k = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * clamp(0.5 - 0.5 * dot(item.yz, _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001)), 0.0, 1.0);
float h = k > 0.0001 ? clamp(0.5 + (0.5 * (other - d)) / k, 0.0, 1.0) : float(d <= other);
if (record) _globals.weights_1[0] *= h;
if (record) _globals.weights_1[1] *= h;
if (record) _globals.weights_1[2] *= h;
if (record) {
_globals.weights_1[3] = 1.0 - h;
_globals.geometryBlended_1 = _globals.geometryBlended_1 || h > 0.0 && h < 1.0;
}
_globals.mergedNormal_1 = mix(item.yz, _globals.mergedNormal_1, h);
d = mix(other, d, h) - (k * h) * (1.0 - h);
}
return d;
}
float2 groupXY_material3_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material3_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].w < 0.0 ? groupXY_material3_1_f2f2(_globals, xy) : xy;
float2 _49_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _50_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _49_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_50_e = saturate(_50_e + 0.5h);
float4 _51_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
return (((_50_e.x * _50_e.y) * _50_e.z) * _50_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _49_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _51_c.xy, _51_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
}
half4 groupSharp_material3_1_h4f2(thread Globals& _globals, float2 xy) {
float2 _52_pos = groupXY_material3_1_f2f2(_globals, xy);
float2 _53_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _52_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _54_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _53_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_54_e = saturate(_54_e + 0.5h);
float4 _55_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
return (((_54_e.x * _54_e.y) * _54_e.z) * _54_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _53_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _55_c.xy, _55_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
}
float nativeGlassOutputCoverage_material3_1_ff(thread Globals& _globals, float sdf) {
float aa = 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
return smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[3].x - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[3].x + aa, sdf) * (1.0 - smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[3].y - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[3].y + aa, sdf));
}
float3 nativeLightingClampColor_material3_1_f3f3ff(float3 c, float limit, float preserveHue) {
if (limit <= 0.0) return c;
if (preserveHue > 0.5) {
float maximum = max(max(c.x, c.y), c.z);
return maximum > limit ? c * (limit / maximum) : c;
}
return clamp(c, float3(-0.75), float3(limit));
}
float nativeLightingErfc_material3_1_ff(float valueInput) {
half x = half(clamp(valueInput, -2.0, 2.0));
half square = x * x;
half value = 0.002954483h * square + -0.0344543457h;
value = value * square + 0.168212891h;
value = value * square + -0.5605469h;
return clamp(float(value * x + 0.5h), 0.0, 1.0);
}
float nativeLightingRingAlpha_material3_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].w <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].y <= 0.0) return 0.0;
float inverseRadius = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].x, 1e-06);
float start = shiftedRingSDF * inverseRadius;
float finish = (shiftedRingSDF + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].y) * inverseRadius;
float ring = clamp(nativeLightingErfc_material3_1_ff(start * 0.707106769) - nativeLightingErfc_material3_1_ff(finish * 0.707106769), 0.0, 1.0);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
return (ring * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].w) * mix(1.0, coverage, clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].z, 0.0, 1.0));
}
float nativeLightingBias_material3_1_fff(float coverage, float amount) {
amount = clamp(amount, 0.0, 1.0);
if (amount <= 0.0 || coverage <= 0.0) return 0.0;
float coefficient = 1.0 / max(amount, 1e-06) - 2.0;
return coverage / max(1.0 + coefficient * (1.0 - coverage), 1e-06);
}
float nativeLightingSDFBand_material3_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal) {
if (height <= 0.0) return 0.0;
float depth = -sdf;
if (depth < -5.0) return 0.0;
float t = clamp(depth / height, 0.0, 1.0);
float shape = mix(float(t < 1.0), 1.0 - t, curvature);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
return (shape * clamp(depth / width + 0.5, 0.0, 1.0)) * clamp((height - depth) / width + 0.5, 0.0, 1.0);
}
float nativeLightingSDFDirection_material3_1_ff2ff(float2 normal, float angle, float spread) {
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(cos(angle - 1.57079637), sin(angle - 1.57079637)));
return clamp((cosine - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
}
float nativeGlassFilterHighlight_material3_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].x <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].z <= 0.0) return 0.0;
float spread = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[3]) <= 1.0 ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[3].z : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].w;
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(sin(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].y), -cos(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].y)));
float2 angular = clamp((float2(cosine, -cosine) - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
float2 localNormal = (normal * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) / max(localScale, 1.0);
float band;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[3].z > 0.5) {
float width = max((abs(localNormal.x) + abs(localNormal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
float depth = -(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[3].x);
band = (1.0 - coverage) * clamp(depth / width + 0.5, 0.0, 1.0);
} else {
band = nativeLightingSDFBand_material3_1_fffff2(_globals, sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].z, 0.75, localNormal);
}
return nativeLightingBias_material3_1_fff(band * angular.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].x) + nativeLightingBias_material3_1_fff(band * angular.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[3].x);
}
half4 nativeGlassHoldingTone_material3_1_h4fh4(thread Globals& _globals, float sdf, half4 material) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].x <= 0.0) return material;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].z;
float progress = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].z) / span, 0.0, 1.0) : 0.0;
float weight = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].x * (1.0 - progress);
float alpha = clamp(float(material.w), 0.0, 1.0);
float3 target = ((float3(material.xyz) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[3].y) * alpha) / max(float(material.w), 1e-06);
return mix(material, half4(half3(target), half(alpha)), half(weight));
}
half4 nativeGlassExterior_material3_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop) {
float outputMask = nativeGlassOutputCoverage_material3_1_ff(_globals, sdf);
if (outputMask <= 0.0) return half4(0.0h);
float3 shadowSource = float3(blurredBackdrop.xyz) / max(float(blurredBackdrop.w), 1e-06);
half4 result = half4(0.0h);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[3].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[3].w > 0.0) {
float coverage = nativeLightingErfc_material3_1_ff(shiftedShadowSDF / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[3].x, 1e-06));
float sourceLuma = dot(shadowSource, float3(0.2126, 0.7152, 0.0722));
float3 linear = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[3].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[3].x) * float3(sourceLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[3].z * (shadowSource - float3(sourceLuma));
float fillAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[3].w, 0.0, 1.0);
float3 bias = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[3].x) * (1.0 - fillAlpha) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[3].xyz * fillAlpha;
float contribution = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[3].z, 0.0, 1.0);
float3 transformed = (linear * (1.0 - fillAlpha)) * contribution + bias;
float nativeAlpha = mix(fillAlpha, 1.0, contribution);
float opacity = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[3].w, 0.0, 1.0);
float alpha = (coverage * opacity) * nativeAlpha;
transformed /= max(nativeAlpha, 1e-06);
result = half4(half3(transformed * alpha), half(alpha));
}
return result * half(outputMask);
}
half3 nativeGlassBleed_material3_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop) {
float3 color = float3(body);
if (sdf > 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) return body;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[3].w > 0.0) {
float luminance = clamp(dot(color, float3(0.2125, 0.7154, 0.0721)), 0.0, 1.0);
float tone = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[3].w > 0.5 ? luminance : 1.0 - luminance;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[3].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[3].x;
float edge = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[3].x) / span, 0.0, 1.0) : float(sdf <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[3].x);
float gain = (tone * tone) * edge;
gain *= gain;
float3 _56_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _57_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[3].x;
float _58_luma = dot(_56_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_57_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[3].y - _57_black) * _58_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[3].z * (_56_c - float3(_58_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[3].w, 0.0, 1.0));
}
return half3(color);
}
float2 nativeSampleXY_material3_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target) {
float2 offset = target - origin;
return (origin + offset * min(1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[3] / max(length(offset), 0.0001))) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
float nativeRadiusLOD_material3_1_ff(thread Globals& _globals, float radius) {
float r = (((max(radius, 0.0) * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[3];
float minRadius = (((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeMinimumBlurRadius_1[3] * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[3];
float base = float(minRadius >= 2.0);
r *= float(base > 0.5 ? 0.5 : 1.0);
return base + max(0.0, log2(r < 2.0 ? 1.0 + r * 0.5 : r));
}
float2 nativePyramidPosition_material3_1_f2f2f(thread Globals& _globals, float2 worldXY, float level) {
float divisor = exp2(level);
float width = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].x;
float height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].y;
float2 tile = float2(1.0);
if (level > 0.5) {
tile.x = width + 3.0;
tile.y = tile.y + (height * (1.0 - 2.0 / divisor) + 2.0 * (level - 1.0));
}
float2 size = float2(width, height) / divisor;
float2 p = (worldXY * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidRect_1[3].xy) / divisor;
return tile + clamp(p, float2(0.5), max(size - 0.5, float2(0.5)));
}
half4 nativePyramidAt_material3_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod) {
lod = clamp(lod, 0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].w);
float first = floor(lod);
float second = min(first + 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].w);
half4 low = groupBackdrop_material3_1_h4f2(_globals, nativePyramidPosition_material3_1_f2f2f(_globals, worldXY, first));
if (fract(lod) == 0.0 || first == second) return low;
return mix(low, groupBackdrop_material3_1_h4f2(_globals, nativePyramidPosition_material3_1_f2f2f(_globals, worldXY, second)), half(fract(lod)));
}
half4 nativeAtRadius_material3_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
float2 sampleXY = nativeSampleXY_material3_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].w < 0.0) return radius > 0.0 ? groupBackdrop_material3_1_h4f2(_globals, sampleXY) : groupSharp_material3_1_h4f2(_globals, sampleXY);
return nativePyramidAt_material3_1_h4f2f(_globals, groupXY_material3_1_f2f2(_globals, sampleXY), nativeRadiusLOD_material3_1_ff(_globals, radius));
}
float3 nativeUnpremul_material3_1_f3h4(half4 p) {
return float3(p.xyz) / max(float(p.w), 1e-06);
}
float nativeLinearRamp_material3_1_ffff(float x, float a, float b) {
return abs(b - a) < 1e-05 ? step(a, x) : clamp((x - a) / (b - a), 0.0, 1.0);
}
float nativeBlurCoverage_material3_1_ff(thread Globals& _globals, float d) {
return ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].x + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].x) * nativeLinearRamp_material3_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].y)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].y) * nativeLinearRamp_material3_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].z)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[3].z) * nativeLinearRamp_material3_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[3].w);
}
float3 nativeFace_material3_1_f3f3f(thread Globals& _globals, float3 color, float sdf) {
float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
float sdr = 1.0 - step(1.00001, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[3]);
float maxLuma = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[3].y, sdr);
float compression = clamp(1.0 + (maxLuma - 1.0) * luma, 0.0, 1.0);
float qLuma = luma * compression;
float3 q = float3(qLuma) + (color * compression - float3(qLuma)) * (1.0 + 0.3 * (1.0 - compression));
float3 result = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[3] + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[3].x - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[3]) * qLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[3].y * (q - float3(qLuma));
result = mix(result, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[3].xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[3].w);
return result;
}
float nativeCubic_material3_1_ffff(float t, float p1, float p2) {
float u = 1.0 - t;
return ((((3.0 * u) * u) * t) * p1 + (((3.0 * u) * t) * t) * p2) + (t * t) * t;
}
float nativeTiming_material3_1_fff4f(float x, float4 control, float known) {
if (x <= 0.0 || x >= 1.0) return clamp(x, 0.0, 1.0);
if (known < 0.5) return x;
float low = 0.0;
float high = 1.0;
float t = x;
for (int i = 0;i < 14; ++i) {
float value = nativeCubic_material3_1_ffff(t, control.x, control.z);
if (value < x) low = t; else high = t;
t = (low + high) * 0.5;
}
return nativeCubic_material3_1_ffff(t, control.y, control.w);
}
float4 nativeGradientColor_material3_1_f4f(thread Globals& _globals, float d) {
float4 a = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent0_1[3] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[3];
float4 b = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[3] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent2_1[3];
float lo = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].x : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y;
float hi = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].z;
float raw = abs(hi - lo) > 1e-05 ? clamp((d - lo) / (hi - lo), 0.0, 1.0) : step(lo, d);
float t = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[3].y ? nativeTiming_material3_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming0_1[3], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[3].x) : nativeTiming_material3_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming1_1[3], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[3].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[3].x > 0.5) {
a.xyz = a.xyz * a.w;
b.xyz = b.xyz * b.w;
float4 c = mix(a, b, t);
c.xyz = c.xyz / max(c.w, 1e-06);
return c;
}
return mix(a, b, t);
}
float nativeBevel_material3_1_ffff(float _0_distance, float height, float radius) {
float t = clamp(-_0_distance / max(height, 1e-06), 0.0, 1.0);
return height > 0.0 ? 1.0 - sqrt(max(t * (2.0 - t), 0.0)) : 0.0;
}
half4 nativeGlassBackgroundBody_material3_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius) {
float inside = nativeBevel_material3_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[3].y, radius);
float outside = nativeBevel_material3_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[3].w, radius);
float displacement = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[3].x * inside;
float2 target = xy + normal * displacement;
float effectiveRadius = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[3].x * max(nativeBlurCoverage_material3_1_ff(_globals, sdf + displacement), 0.0);
nativeAtRadius_material3_1_h4f2f2f(_globals, xy, target, 0.0);
half4 pixel = nativeAtRadius_material3_1_h4f2f2f(_globals, xy, target, effectiveRadius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[3] > 0.0 && abs(displacement) > 0.001) {
float2 shift = ((normal * abs(displacement)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[3]) * 0.3;
half4 red = nativeAtRadius_material3_1_h4f2f2f(_globals, xy, target + shift, effectiveRadius);
half4 blue = nativeAtRadius_material3_1_h4f2f2f(_globals, xy, target - shift, effectiveRadius);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
float3 rgb = nativeUnpremul_material3_1_f3h4(pixel);
float bodyAlpha = 1.0;
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].x > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].y > 0.0) || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].z > 0.0) {
half4 fill;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].w < 0.0) {
fill = nativeAtRadius_material3_1_h4f2f2f(_globals, xy, xy, 0.0);
} else {
float lod = nativeRadiusLOD_material3_1_ff(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[3].y);
float2 _62_xy = nativeSampleXY_material3_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material3_1_f2f2(_globals, _62_xy);
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[3].z, 1e-06));
fill = (nativePyramidAt_material3_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_material3_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_material3_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[3].x);
}
float outerMask = nativeLinearRamp_material3_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[3].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[3].z * outerMask > 0.0) {
float outerShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[3].z * outside;
half4 outer = nativeAtRadius_material3_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[3].x * max(nativeBlurCoverage_material3_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[3].z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[3].z * outerMask);
}
float3 processed = nativeFace_material3_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[3]);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[3].w > 0.0) {
float2 _63_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[3].x) * nativeBevel_material3_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[3].z, radius);
rgb = float3(nativeGlassBleed_material3_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material3_1_h4f2f2f(_globals, xy, _63_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[3].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}, {image_13_Tex, image_13_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _219_xy = _in.localCoordsVar / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float _220_rawDistance = mergeGeometry_1_ff2b(_globals, _219_xy, true);
float2 _221_sceneNormal = _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001);
float _222_sceneDistance = _220_rawDistance;
if (_globals.geometryBlended_1) {
float _223_epsilon = max(0.05, 0.25 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
float _224_dx = mergeGeometry_1_ff2b(_globals, _219_xy + float2(_223_epsilon, 0.0), false) - mergeGeometry_1_ff2b(_globals, _219_xy - float2(_223_epsilon, 0.0), false);
float _225_dy = mergeGeometry_1_ff2b(_globals, _219_xy + float2(0.0, _223_epsilon), false) - mergeGeometry_1_ff2b(_globals, _219_xy - float2(0.0, _223_epsilon), false);
float2 _226_gradient = float2(_224_dx, _225_dy) / (2.0 * _223_epsilon);
float _227_magnitude = length(_226_gradient);
if (_227_magnitude > 0.0001) _221_sceneNormal = _226_gradient / _227_magnitude;
_222_sceneDistance = _220_rawDistance / max(_227_magnitude, 0.05);
}
{
float _228_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _228_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _228_cover;
_globals.weights_1[1] *= 1.0 - _228_cover;
_globals.weights_1[2] *= 1.0 - _228_cover;
_globals.weights_1[3] *= 1.0 - _228_cover;
_globals.weights_1[0] += _228_cover;
}
{
float _229_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _229_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _229_cover;
_globals.weights_1[1] *= 1.0 - _229_cover;
_globals.weights_1[2] *= 1.0 - _229_cover;
_globals.weights_1[3] *= 1.0 - _229_cover;
_globals.weights_1[1] += _229_cover;
}
{
float _230_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5) _230_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _230_cover;
_globals.weights_1[1] *= 1.0 - _230_cover;
_globals.weights_1[2] *= 1.0 - _230_cover;
_globals.weights_1[3] *= 1.0 - _230_cover;
_globals.weights_1[2] += _230_cover;
}
{
float _231_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] > 1.5) _231_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _231_cover;
_globals.weights_1[1] *= 1.0 - _231_cover;
_globals.weights_1[2] *= 1.0 - _231_cover;
_globals.weights_1[3] *= 1.0 - _231_cover;
_globals.weights_1[3] += _231_cover;
}
{
float _232_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _232_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _232_cover;
_globals.weights_1[1] *= 1.0 - _232_cover;
_globals.weights_1[2] *= 1.0 - _232_cover;
_globals.weights_1[3] *= 1.0 - _232_cover;
_globals.weights_1[0] += _232_cover;
}
{
float _233_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _233_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _233_cover;
_globals.weights_1[1] *= 1.0 - _233_cover;
_globals.weights_1[2] *= 1.0 - _233_cover;
_globals.weights_1[3] *= 1.0 - _233_cover;
_globals.weights_1[1] += _233_cover;
}
{
float _234_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] <= 1.5) _234_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _234_cover;
_globals.weights_1[1] *= 1.0 - _234_cover;
_globals.weights_1[2] *= 1.0 - _234_cover;
_globals.weights_1[3] *= 1.0 - _234_cover;
_globals.weights_1[2] += _234_cover;
}
{
float _235_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] <= 1.5) _235_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _235_cover;
_globals.weights_1[1] *= 1.0 - _235_cover;
_globals.weights_1[2] *= 1.0 - _235_cover;
_globals.weights_1[3] *= 1.0 - _235_cover;
_globals.weights_1[3] += _235_cover;
}
float2 _236_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _in.localCoordsVar + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _237_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _236_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_237_e = saturate(_237_e + 0.5h);
float4 _238_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
half4 _239_original = (((_237_e.x * _237_e.y) * _237_e.z) * _237_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _236_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _238_c.xy, _238_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
float _240_coverage = clamp(0.5 - _222_sceneDistance * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
half4 _241_result = half4(0.0h);
float _242_groupInteraction = 0.0;
float2 _243_groupTouch = _219_xy;
{
float _244_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[0] * clamp(0.5 - _globals.distances_1[0] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_244_intensity > _242_groupInteraction) {
_242_groupInteraction = _244_intensity;
float3 _245_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[0], 1.0);
_243_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, _245_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, _245_touch));
}
}
{
float _246_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[1] * clamp(0.5 - _globals.distances_1[1] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_246_intensity > _242_groupInteraction) {
_242_groupInteraction = _246_intensity;
float3 _247_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[1], 1.0);
_243_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, _247_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, _247_touch));
}
}
{
float _248_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[2] * clamp(0.5 - _globals.distances_1[2] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_248_intensity > _242_groupInteraction) {
_242_groupInteraction = _248_intensity;
float3 _249_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[2], 1.0);
_243_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xyz, _249_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xyz, _249_touch));
}
}
{
float _250_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[3] * clamp(0.5 - _globals.distances_1[3] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_250_intensity > _242_groupInteraction) {
_242_groupInteraction = _250_intensity;
float3 _251_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[3], 1.0);
_243_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].xyz, _251_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].xyz, _251_touch));
}
}
if (_globals.weights_1[3] > 0.0) {
float2 _252_localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].xyz, float3(_219_xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].xyz, float3(_219_xy, 1.0)));
float2 _253_localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].x * _221_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].x * _221_sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].y * _221_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].y * _221_sceneNormal.y);
float _254_localMetric = max(length(_253_localGradient), 0.0001);
_globals.materialNormal_1 = _253_localGradient / _254_localMetric;
_globals.interactionIntensity_1 = _242_groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].xyz, float3(_243_groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].xyz, float3(_243_groupTouch, 1.0)));
float _255_localDistance = _222_sceneDistance / _254_localMetric;
if (nativeGlassOutputCoverage_material3_1_ff(_globals, _255_localDistance) > 0.0 || _240_coverage > 0.0) {
float _256_ringDistance = _255_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[3].y > 0.0) {
float2 _257_ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[3];
_256_ringDistance = mergeGeometry_1_ff2b(_globals, _219_xy - _257_ringOffset, false) / _254_localMetric;
}
float _258_shadowDistance = _255_localDistance;
if (_240_coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[3].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[3].w > 0.0)) {
float2 _259_shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[3]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[3]));
_258_shadowDistance = mergeGeometry_1_ff2b(_globals, _219_xy - _259_shadowOffset, false) / _254_localMetric;
}
float _260_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[3], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].y) * 0.5);
half4 _261_field;
if (_240_coverage >= 1.0) _261_field = nativeGlassBackgroundBody_material3_1_h4f2ff2f(_globals, _252_localXY, _255_localDistance, _globals.materialNormal_1, _260_radius); else {
float _262_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[3].x * nativeBevel_material3_1_ffff(_255_localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[3].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[3].z, _260_radius);
half4 _263_shadow = nativeAtRadius_material3_1_h4f2f2f(_globals, _252_localXY, _252_localXY + _globals.materialNormal_1 * _262_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[3].w);
_261_field = nativeGlassExterior_material3_1_h4ffff2h4h4h4(_globals, _255_localDistance, _258_shadowDistance, _256_ringDistance, _globals.materialNormal_1, _239_original, _263_shadow, _239_original);
if (_240_coverage > 0.0) _261_field = mix(_261_field, nativeGlassBackgroundBody_material3_1_h4f2ff2f(_globals, _252_localXY, _255_localDistance, _globals.materialNormal_1, _260_radius), half(_240_coverage));
}
float _264_alpha = clamp(nativeLightingRingAlpha_material3_1_ffff2(_globals, _255_localDistance, _256_ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _265_foreground = half4(0.0h, 0.0h, 0.0h, half(_264_alpha));
_261_field = _265_foreground + _261_field * (1.0h - _265_foreground.w);
half4 _266_material = _261_field;
half3 _267_exteriorFace = half3(nativeFace_material3_1_f3f3f(_globals, nativeUnpremul_material3_1_f3h4(_239_original), _255_localDistance));
float _268_amount = nativeGlassFilterHighlight_material3_1_fff2f(_globals, _255_localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * _254_localMetric);
if (_268_amount > 1e-06) {
float3 _269_raw = float3(_239_original.xyz) / max(float(_239_original.w), 1e-06);
float3 _270_face = float3(_267_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[3].x > 0.5) _270_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[3].y < 0.0 ? min(_270_face, _269_raw) : max(_270_face, _269_raw);
float _271_alpha = float(_266_material.w);
float3 _272_color = float3(_266_material.xyz) + (1.0 - _271_alpha) * mix(_269_raw, _270_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[3] * _268_amount);
_272_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[3].y * _268_amount) * (3.0 - 2.0 * _272_color);
float _273_resultAlpha = _271_alpha + ((1.0 - _271_alpha) * _268_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[3].y;
_266_material = half4(half3(_272_color - (1.0 - _273_resultAlpha) * _269_raw), half(_273_resultAlpha));
}
_261_field = nativeGlassHoldingTone_material3_1_h4fh4(_globals, _255_localDistance, _266_material);
float3 _274_fieldColor = nativeLightingClampColor_material3_1_f3f3ff(nativeUnpremul_material3_1_f3h4(_261_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[3]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].y);
_261_field = half4(half3(_274_fieldColor) * _261_field.w, _261_field.w);
half4 _275_composited = _261_field + _239_original * (1.0h - _261_field.w);
float3 _276_clamped = clamp(nativeUnpremul_material3_1_f3h4(_275_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[3])));
_275_composited = half4(half3(_276_clamped) * _275_composited.w, _275_composited.w);
float3 _277_rgb = nativeUnpremul_material3_1_f3h4(_275_composited);
float _278_materialAlpha = float(_275_composited.w);
float _279_inside = nativeBevel_material3_1_ffff(_255_localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[3].y, _260_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[3] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[3].w > 0.0) {
float4 _280_gradient = nativeGradientColor_material3_1_f4f(_globals, _255_localDistance);
float4 _281_matrixInput = float4(_277_rgb, float(_275_composited.w));
float4 _282_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[3], _281_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[3], _281_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[3], _281_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[3], _281_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[3];
_282_result.xyz = nativeLightingClampColor_material3_1_f3f3ff(_282_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[3].y);
float4 _283_accent = _282_result;
_283_accent.w = clamp(_283_accent.w, 0.0, 1.0);
_283_accent.xyz = _283_accent.xyz * _283_accent.w;
float _284_mask = _280_gradient.w * _240_coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[3].y > 0.5) _284_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].w; else {
_283_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].w + _283_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].w);
_283_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].w + _283_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[3].w);
}
_283_accent *= clamp(_284_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[3].w, 0.0, 1.0);
float _285_outAlpha = _283_accent.w + _278_materialAlpha * (1.0 - _283_accent.w);
_277_rgb = (_283_accent.xyz + (_277_rgb * _278_materialAlpha) * (1.0 - _283_accent.w)) / max(_285_outAlpha, 1e-06);
_278_materialAlpha = _285_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[3] >= 0.0) {
_277_rgb = mix(_277_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[3].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[3] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[3].w) * _240_coverage, 0.0, 1.0));
}
_277_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[3] * _279_inside;
_277_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[3] * smoothstep(0.85, 0.98, dot(_277_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _286_unlit = _277_rgb;
float3 _287_color = float3(half3(_277_rgb));
float2 _288_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].w > 0.5 ? _221_sceneNormal : _globals.materialNormal_1;
float _289_norm = length(_288_normal);
if (_289_norm > 1e-05) _288_normal /= _289_norm;
float _290_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[3].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[3].y);
float _291_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[3].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[3].y);
float _292_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[3].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[3].w;
float _293_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[3].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[3].w;
float _294_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].x;
float _295_key = 0.0;
float _296_fill = 0.0;
float _297_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].w > 0.5 ? _255_localDistance / max(_289_norm, 1e-06) : _255_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].z > 0.0) {
float _298_primary = nativeLightingSDFBand_material3_1_fffff2(_globals, _297_localSDF, _290_keyHeight, _294_curvature, _288_normal) * nativeLightingSDFDirection_material3_1_ff2ff(_288_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].x, _292_keySpread);
float _299_diffuse = nativeLightingSDFBand_material3_1_fffff2(_globals, _297_localSDF, _290_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[3].w, 1.0, _288_normal) * nativeLightingSDFDirection_material3_1_ff2ff(_288_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].x, _292_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].z);
_295_key = nativeLightingBias_material3_1_fff(_298_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].z) + nativeLightingBias_material3_1_fff(_299_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[3].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].z > 0.0) {
float _300_primary = nativeLightingSDFBand_material3_1_fffff2(_globals, _297_localSDF, _291_fillHeight, _294_curvature, _288_normal) * nativeLightingSDFDirection_material3_1_ff2ff(_288_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].x, _293_fillSpread);
float _301_diffuse = nativeLightingSDFBand_material3_1_fffff2(_globals, _297_localSDF, _291_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[3].w, 1.0, _288_normal) * nativeLightingSDFDirection_material3_1_ff2ff(_288_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].x, _293_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].z);
_296_fill = nativeLightingBias_material3_1_fff(_300_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].z) + nativeLightingBias_material3_1_fff(_301_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[3].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[3].y);
}
float3 _302_unlitColor = _287_color;
float4 _303_c = float4(_302_unlitColor, 1.0);
float4 _304_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[3], _303_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[3], _303_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[3], _303_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[3], _303_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[3];
_304_result.xyz = nativeLightingClampColor_material3_1_f3f3ff(_304_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[3].y);
float4 _305_transformed = _304_result;
float _306_coverage = _295_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[3].w + _296_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[3].w;
_306_coverage *= clamp(_305_transformed.w, 0.0, 1.0);
_287_color = _302_unlitColor + _306_coverage * (_305_transformed.xyz - _302_unlitColor);
_277_rgb = float3(half3(nativeLightingClampColor_material3_1_f3f3ff(_287_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[3])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].y)));
float _307_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[3], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[3], clamp(dot(_286_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_277_rgb = mix(_286_unlit, _277_rgb, _307_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[3] > 0.0 && _279_inside > 0.0) {
float2 _308_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[3];
half4 _309_reflected = (nativeAtRadius_material3_1_h4f2f2f(_globals, _252_localXY, (_252_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[3]) + _308_tangent, 0.0) + nativeAtRadius_material3_1_h4f2f2f(_globals, _252_localXY, (_252_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[3]) - _308_tangent, 0.0)) * 0.5h;
_277_rgb = mix(_277_rgb, nativeUnpremul_material3_1_f3h4(_309_reflected), (_279_inside * _279_inside) * 0.1);
}
float _310_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * _240_coverage;
if (_310_glow > 0.0) {
float _311_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].y) * 1.6);
float _312_sheen = 1.0 - smoothstep(0.0, _311_glowRadius, length(_252_localXY - _globals.interactionLocation_1));
float _313_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[3] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[3]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[3]));
float3 _314_restingResult = _277_rgb;
_277_rgb = mix(_277_rgb, float3(1.0), _310_glow * _313_ambientPressOpacity);
_277_rgb = mix(_277_rgb, float3(1.0), (_310_glow * _312_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[3]));
float _315_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].y));
float _316_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[3].y);
float _317_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[3] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] < 0.5) * smoothstep(160.0, 280.0, _316_longSide)) * smoothstep(2.5, 4.0, _316_longSide / _315_shortSide);
float _318_peak = max(_314_restingResult.x, max(_314_restingResult.y, _314_restingResult.z));
float _319_chromaticRange = _318_peak - min(_314_restingResult.x, min(_314_restingResult.y, _314_restingResult.z));
float _320_neutralPress = 1.0 - smoothstep(0.01, 0.1, _319_chromaticRange);
float _321_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[3]);
float _322_pressGain = mix(1.18 - 0.06 * _318_peak, 1.06, _321_lightAppearance);
float3 _323_colorPress = mix(float3(float(dot(half3(_314_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _314_restingResult, mix(1.03, 1.06, _321_lightAppearance)) * _322_pressGain;
_323_colorPress += float3(_320_neutralPress * mix(0.02745098, 0.04509804, _321_lightAppearance));
_277_rgb = mix(_277_rgb, mix(_314_restingResult, _323_colorPress, _310_glow), _317_platterPress);
}
_277_rgb = nativeLightingClampColor_material3_1_f3f3ff(_277_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[3]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[3].y);
float _324_alpha = clamp(_278_materialAlpha, 0.0, 1.0);
half4 _325_color = half4(half3(_277_rgb * _324_alpha), half(_324_alpha));
_241_result += half(_globals.weights_1[3] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[3]) * _325_color;
} else _241_result += half(_globals.weights_1[3] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[3]) * _239_original;
}
half4 outColor_1 = _241_result;
_out.sk_FragColor = outColor_1;
return _out;
}
