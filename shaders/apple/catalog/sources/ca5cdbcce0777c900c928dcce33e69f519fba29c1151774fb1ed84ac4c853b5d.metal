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
float contentScale_1;
float filterScale_1;
array<float, 2> material_interactionIntensity_1;
array<float2, 2> material_interactionLocation_1;
array<float, 2> material_bodyMode_1;
array<float, 2> material_tintMode_1;
array<float, 2> material_referenceAppearance_1;
array<float, 2> material_backdropLuma_1;
array<float4, 2> material_tint_1;
array<float, 2> material_nativeFaceBlack_1;
array<float3, 2> material_nativeFaceTransfer_1;
array<float4, 2> material_nativeFaceFill_1;
array<float2, 2> material_nativeFaceLuma_1;
array<float, 2> material_nativeFaceOpacity_1;
array<float3, 2> material_nativeFillOpacity_1;
array<float4, 2> material_nativeBlurDistances_1;
array<float4, 2> material_nativeBlurOpacities_1;
array<float4, 2> material_nativeRefraction_1;
array<float3, 2> material_nativeRefractionMask_1;
array<float2, 2> material_nativeSDRGradient_1;
array<float, 2> material_nativeDisplayHeadroom_1;
array<float, 2> material_nativeAccentEnabled_1;
array<float4, 2> material_nativeAccent0_1;
array<float4, 2> material_nativeAccent1_1;
array<float4, 2> material_nativeAccent2_1;
array<float3, 2> material_nativeAccentDistances_1;
array<float4, 2> material_nativeAccentTiming0_1;
array<float4, 2> material_nativeAccentTiming1_1;
array<float2, 2> material_nativeAccentTimingKnown_1;
array<float2, 2> material_nativeAccentMaskOptions_1;
array<float4, 2> material_nativeAccentFill_1;
array<float4, 2> material_nativeAccentMatrix0_1;
array<float4, 2> material_nativeAccentMatrix1_1;
array<float4, 2> material_nativeAccentMatrix2_1;
array<float4, 2> material_nativeAccentMatrix3_1;
array<float4, 2> material_nativeAccentMatrixBias_1;
array<float3, 2> material_nativeAccentMatrixOptions_1;
array<float, 2> material_nativeDispersion_1;
array<float, 2> material_nativeEdgeAbsorption_1;
array<float, 2> material_nativeTintOpacity_1;
array<float, 2> material_nativeBrightBackdropShade_1;
array<float, 2> material_nativeDarkSpecularGain_1;
array<float, 2> material_nativeBrightSpecularGain_1;
array<float, 2> material_nativeReflectionDistance_1;
array<float, 2> material_nativeReflectionSpread_1;
array<float4, 2> material_backdropPyramidRect_1;
array<float4, 2> material_backdropPyramidInfo_1;
array<float, 2> material_nativePyramidStretch_1;
array<float, 2> material_nativeMinimumBlurRadius_1;
array<float4, 2> material_nativeBlurRadii_1;
array<float4, 2> inverseX_1;
array<float4, 2> inverseY_1;
array<float4, 2> forwardX_1;
array<float4, 2> forwardY_1;
array<float4, 2> geometry_1;
array<float4, 2> outputBounds_1;
array<float, 2> itemAlpha_1;
float mergeSpacing_1;
array<float4, 2> material_cornerTop_1;
array<float4, 2> material_cornerTopCircle_1;
array<float4, 2> material_cornerRight_1;
array<float4, 2> material_cornerRightCircle_1;
array<float2, 2> material_nativeOutputDistance_1;
array<float4, 2> material_nativeShadowShape_1;
array<float2, 2> material_nativeShadowOffset_1;
array<float4, 2> material_nativeShadowTransfer_1;
array<float4, 2> material_nativeShadowFill_1;
array<float4, 2> material_nativeShadowResponse_1;
array<float4, 2> material_nativeRingShadow_1;
array<float, 2> material_nativeRingShadowOffset_1;
array<float4, 2> material_nativeBleedShape_1;
array<float4, 2> material_nativeBleedTransfer_1;
array<float2, 2> material_nativeBleedDistance_1;
array<float4, 2> material_nativeFilterHighlight_1;
array<float3, 2> material_nativeFilterHighlightColor_1;
array<float4, 2> material_nativeKeyHighlight_1;
array<float4, 2> material_nativeFillHighlight_1;
array<float4, 2> material_nativeKeyHighlightShape_1;
array<float4, 2> material_nativeFillHighlightShape_1;
array<float4, 2> material_nativeKeyHighlightColor_1;
array<float4, 2> material_nativeFillHighlightColor_1;
array<float4, 2> material_nativeHighlightResponse_1;
array<float4, 2> material_nativeHighlightMatrix0_1;
array<float4, 2> material_nativeHighlightMatrix1_1;
array<float4, 2> material_nativeHighlightMatrix2_1;
array<float4, 2> material_nativeHighlightMatrix3_1;
array<float4, 2> material_nativeHighlightMatrixBias_1;
array<float4, 2> material_nativeHighlightMatrixOptions_1;
array<float3, 2> material_nativeLightingClamp_1;
array<float, 2> material_nativeLightingDisplayHeadroom_1;
array<float4, 2> material_nativeHoldingTone_1;
array<float3, 2> material_nativeFilterOptions_1;
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
float4 upper2x2_16;
float2 translation_16;
float4 decalBounds_17;
float4 upper2x2_18;
float2 translation_18;
float2 invImgSize_20;
float4 subsetInsetClamp_20;
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
    sampler2D image_20;
    float cornerCurve_1;
    float interactionIntensity_1;
    float2 interactionLocation_1;
    array<float, 2> weights_1;
    array<float, 2> distances_1;
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
half4 LocalMatrix_2_h4f2(thread Globals& _globals, float2 pos) {
float2 _5_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _6_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _5_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_6_e = saturate(_6_e + 0.5h);
float4 _7_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
return (((_6_e.x * _6_e.y) * _6_e.z) * _6_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _5_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _7_c.xy, _7_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
}
half4 sk_decal_image_10_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_17_h4f2f4(float2 b, float4 c);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 itemDistance0_1_f3f2(float2 xy);
float3 itemDistance1_1_f3f2(float2 xy);
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record);
void materialOwnership_1_v();
float2 groupXY_material0_1_f2f2(thread Globals& _globals, float2 xy);
half4 groupBackdrop_material0_1_h4f2(thread Globals& _globals, float2 xy);
half4 groupSharp_material0_1_h4f2(thread Globals& _globals, float2 xy);
float toBrightness_material0_1_fh3(half3 c);
float nativeLightingAA_material0_1_f();
float nativeLightingHeadroom_material0_1_f();
float nativeLightingLuma_material0_1_ff3(float3 c);
float3 nativeLightingUnpremul_material0_1_f3h4(half4 c);
float nativeGlassOutputCoverage_material0_1_ff(thread Globals& _globals, float sdf);
float3 nativeLightingTransfer_material0_1_f3f3fff(float3 c, float black, float white, float saturation);
float3 nativeLightingClampColor_material0_1_f3f3ff(float3 c, float limit, float preserveHue);
float nativeLightingErfc_material0_1_ff(float valueInput);
float nativeLightingRingAlpha_material0_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal);
half4 nativeLightingOver_material0_1_h4h4h4(half4 foreground, half4 background);
float nativeLightingBias_material0_1_fff(float coverage, float amount);
float nativeLightingSDFBand_material0_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal);
float nativeLightingSDFDirection_material0_1_ff2ff(float2 normal, float angle, float spread);
float nativeGlassFilterHighlight_material0_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale);
half4 nativeGlassHoldingTone_material0_1_h4fh4(thread Globals& _globals, float sdf, half4 material);
half4 nativeGlassBackgroundFinalize_material0_1_h4fh4h4h3ff2f(float sdf, half4 material, half4 original, half3 exteriorFace, float faceOpacity, float2 normal, float localScale);
half4 nativeGlassExterior_material0_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop);
half3 nativeGlassBleed_material0_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop);
half4 nativeGlassRingComposite_material0_1_h4fff2fh4(float sdf, float shiftedRingSDF, float2 normal, float fieldAlpha, half4 material);
float4 nativeLightingHighlightMatrix_material0_1_f4f4(float4 c);
half3 nativeGlassHighlight_material0_1_h3ff2f2h3(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop);
float2 nativeSampleXY_material0_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target);
float2 nativeSamplingWorld_material0_1_f2f2(float2 xy);
float nativeRadiusLOD_material0_1_ff(thread Globals& _globals, float radius);
float2 nativePyramidPosition_material0_1_f2f2f(thread Globals& _globals, float2 worldXY, float level);
half4 nativePyramidAt_material0_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod);
half4 nativeAtRadius_material0_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius);
half4 nativeSharp_material0_1_h4f2f2(float2 origin, float2 target);
half4 nativeBleed_material0_1_h4f2f2(float2 origin, float2 target);
half4 nativeShadow_material0_1_h4f2f2(float2 origin, float2 target);
float3 nativeUnpremul_material0_1_f3h4(half4 p);
float nativeLinearRamp_material0_1_ffff(float x, float a, float b);
float nativeBlurCoverage_material0_1_ff(thread Globals& _globals, float d);
float3 nativeFace_material0_1_f3f3f(thread Globals& _globals, float3 color, float sdf);
float4 nativeAccentMatrix_material0_1_f4f4(float4 color);
float nativeCubic_material0_1_ffff(float t, float p1, float p2);
float nativeTiming_material0_1_fff4f(float x, float4 control, float known);
float4 nativeGradientColor_material0_1_f4f(thread Globals& _globals, float d);
float nativeBevel_material0_1_ffff(float _0_distance, float height, float radius);
half4 nativeGlassBackgroundBody_material0_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius);
half4 nativeGlassDecorate_material0_1_h4f2ff2f2fh4f(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, half4 material, float surfaceCoverage);
half4 nativeGlassSurface_material0_1_h4f2ff2f2fffh4ff(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, float shadowSDF, float ringSDF, half4 original, float coverage, float localScale);
float2 groupXY_material1_1_f2f2(thread Globals& _globals, float2 xy);
half4 groupBackdrop_material1_1_h4f2(thread Globals& _globals, float2 xy);
half4 groupSharp_material1_1_h4f2(thread Globals& _globals, float2 xy);
float toBrightness_material1_1_fh3(half3 c);
float nativeLightingAA_material1_1_f();
float nativeLightingHeadroom_material1_1_f();
float nativeLightingLuma_material1_1_ff3(float3 c);
float3 nativeLightingUnpremul_material1_1_f3h4(half4 c);
float nativeGlassOutputCoverage_material1_1_ff(thread Globals& _globals, float sdf);
float3 nativeLightingTransfer_material1_1_f3f3fff(float3 c, float black, float white, float saturation);
float3 nativeLightingClampColor_material1_1_f3f3ff(float3 c, float limit, float preserveHue);
float nativeLightingErfc_material1_1_ff(float valueInput);
float nativeLightingRingAlpha_material1_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal);
half4 nativeLightingOver_material1_1_h4h4h4(half4 foreground, half4 background);
float nativeLightingBias_material1_1_fff(float coverage, float amount);
float nativeLightingSDFBand_material1_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal);
float nativeLightingSDFDirection_material1_1_ff2ff(float2 normal, float angle, float spread);
float nativeGlassFilterHighlight_material1_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale);
half4 nativeGlassHoldingTone_material1_1_h4fh4(thread Globals& _globals, float sdf, half4 material);
half4 nativeGlassBackgroundFinalize_material1_1_h4fh4h4h3ff2f(float sdf, half4 material, half4 original, half3 exteriorFace, float faceOpacity, float2 normal, float localScale);
half4 nativeGlassExterior_material1_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop);
half3 nativeGlassBleed_material1_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop);
half4 nativeGlassRingComposite_material1_1_h4fff2fh4(float sdf, float shiftedRingSDF, float2 normal, float fieldAlpha, half4 material);
float4 nativeLightingHighlightMatrix_material1_1_f4f4(float4 c);
half3 nativeGlassHighlight_material1_1_h3ff2f2h3(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop);
float2 nativeSampleXY_material1_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target);
float2 nativeSamplingWorld_material1_1_f2f2(float2 xy);
float nativeRadiusLOD_material1_1_ff(thread Globals& _globals, float radius);
float2 nativePyramidPosition_material1_1_f2f2f(thread Globals& _globals, float2 worldXY, float level);
half4 nativePyramidAt_material1_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod);
half4 nativeAtRadius_material1_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius);
half4 nativeSharp_material1_1_h4f2f2(float2 origin, float2 target);
half4 nativeBleed_material1_1_h4f2f2(float2 origin, float2 target);
half4 nativeShadow_material1_1_h4f2f2(float2 origin, float2 target);
float3 nativeUnpremul_material1_1_f3h4(half4 p);
float nativeLinearRamp_material1_1_ffff(float x, float a, float b);
float nativeBlurCoverage_material1_1_ff(thread Globals& _globals, float d);
float3 nativeFace_material1_1_f3f3f(thread Globals& _globals, float3 color, float sdf);
float4 nativeAccentMatrix_material1_1_f4f4(float4 color);
float nativeCubic_material1_1_ffff(float t, float p1, float p2);
float nativeTiming_material1_1_fff4f(float x, float4 control, float known);
float4 nativeGradientColor_material1_1_f4f(thread Globals& _globals, float d);
float nativeBevel_material1_1_ffff(float _0_distance, float height, float radius);
half4 nativeGlassBackgroundBody_material1_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius);
half4 nativeGlassDecorate_material1_1_h4f2ff2f2fh4f(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, half4 material, float surfaceCoverage);
half4 nativeGlassSurface_material1_1_h4f2ff2f2fffh4ff(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, float shadowSDF, float ringSDF, half4 original, float coverage, float localScale);
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
return float3(circularBoxSDF_1_ff2f2f(p, halfDim, r), lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record) {
float4 _24_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _25_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _24_g.xy * 0.5;
_globals.cornerCurve_1 = _24_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _26_r = min(_24_g.z, min(_24_g.x, _24_g.y) * 0.5);
float3 _27_surface = roundedBoxSample_1_f3f2f2f(_globals, _25_p, _24_g.xy * 0.5, _26_r);
float _28_d = _27_surface.x;
float2 _29_n = _27_surface.yz;
float2 _30_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _29_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _29_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _29_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _29_n.y);
float _31_magnitude = max(length(_30_gradient), 0.0001);
float3 first = float3(_28_d / _31_magnitude, _30_gradient / _31_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _32_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _33_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _32_g.xy * 0.5;
_globals.cornerCurve_1 = _32_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _34_r = min(_32_g.z, min(_32_g.x, _32_g.y) * 0.5);
float3 _35_surface = roundedBoxSample_1_f3f2f2f(_globals, _33_p, _32_g.xy * 0.5, _34_r);
float _36_d = _35_surface.x;
float2 _37_n = _35_surface.yz;
float2 _38_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _37_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _37_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _37_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _37_n.y);
float _39_magnitude = max(length(_38_gradient), 0.0001);
float3 item = float3(_36_d / _39_magnitude, _38_gradient / _39_magnitude);
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
float2 groupXY_material0_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material0_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w < 0.0 ? groupXY_material0_1_f2f2(_globals, xy) : xy;
float2 _40_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _41_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _40_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_41_e = saturate(_41_e + 0.5h);
float4 _42_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
return (((_41_e.x * _41_e.y) * _41_e.z) * _41_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _40_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _42_c.xy, _42_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
}
half4 groupSharp_material0_1_h4f2(thread Globals& _globals, float2 xy) {
return LocalMatrix_2_h4f2(_globals, groupXY_material0_1_f2f2(_globals, xy));
}
float nativeGlassOutputCoverage_material0_1_ff(thread Globals& _globals, float sdf) {
float aa = 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
return smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[0].x - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[0].x + aa, sdf) * (1.0 - smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[0].y - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[0].y + aa, sdf));
}
float3 nativeLightingClampColor_material0_1_f3f3ff(float3 c, float limit, float preserveHue) {
if (limit <= 0.0) return c;
if (preserveHue > 0.5) {
float maximum = max(max(c.x, c.y), c.z);
return maximum > limit ? c * (limit / maximum) : c;
}
return clamp(c, float3(-0.75), float3(limit));
}
float nativeLightingErfc_material0_1_ff(float valueInput) {
half x = half(clamp(valueInput, -2.0, 2.0));
half square = x * x;
half value = 0.002954483h * square + -0.0344543457h;
value = value * square + 0.168212891h;
value = value * square + -0.5605469h;
return clamp(float(value * x + 0.5h), 0.0, 1.0);
}
float nativeLightingRingAlpha_material0_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].w <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].y <= 0.0) return 0.0;
float inverseRadius = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].x, 1e-06);
float start = shiftedRingSDF * inverseRadius;
float finish = (shiftedRingSDF + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].y) * inverseRadius;
float ring = clamp(nativeLightingErfc_material0_1_ff(start * 0.707106769) - nativeLightingErfc_material0_1_ff(finish * 0.707106769), 0.0, 1.0);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
return (ring * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].w) * mix(1.0, coverage, clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].z, 0.0, 1.0));
}
float nativeLightingBias_material0_1_fff(float coverage, float amount) {
amount = clamp(amount, 0.0, 1.0);
if (amount <= 0.0 || coverage <= 0.0) return 0.0;
float coefficient = 1.0 / max(amount, 1e-06) - 2.0;
return coverage / max(1.0 + coefficient * (1.0 - coverage), 1e-06);
}
float nativeLightingSDFBand_material0_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal) {
if (height <= 0.0) return 0.0;
float depth = -sdf;
if (depth < -5.0) return 0.0;
float t = clamp(depth / height, 0.0, 1.0);
float shape = mix(float(t < 1.0), 1.0 - t, curvature);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
return (shape * clamp(depth / width + 0.5, 0.0, 1.0)) * clamp((height - depth) / width + 0.5, 0.0, 1.0);
}
float nativeLightingSDFDirection_material0_1_ff2ff(float2 normal, float angle, float spread) {
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(cos(angle - 1.57079637), sin(angle - 1.57079637)));
return clamp((cosine - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
}
float nativeGlassFilterHighlight_material0_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].x <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].z <= 0.0) return 0.0;
float spread = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[0]) <= 1.0 ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].z : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].w;
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(sin(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].y), -cos(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].y)));
float2 angular = clamp((float2(cosine, -cosine) - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
float2 localNormal = (normal * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) / max(localScale, 1.0);
float band;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].z > 0.5) {
float width = max((abs(localNormal.x) + abs(localNormal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
float depth = -(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].x);
band = (1.0 - coverage) * clamp(depth / width + 0.5, 0.0, 1.0);
} else {
band = nativeLightingSDFBand_material0_1_fffff2(_globals, sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].z, 0.75, localNormal);
}
return nativeLightingBias_material0_1_fff(band * angular.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].x) + nativeLightingBias_material0_1_fff(band * angular.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[0].x);
}
half4 nativeGlassHoldingTone_material0_1_h4fh4(thread Globals& _globals, float sdf, half4 material) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].x <= 0.0) return material;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].z;
float progress = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].z) / span, 0.0, 1.0) : 0.0;
float weight = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].x * (1.0 - progress);
float alpha = clamp(float(material.w), 0.0, 1.0);
float3 target = ((float3(material.xyz) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[0].y) * alpha) / max(float(material.w), 1e-06);
return mix(material, half4(half3(target), half(alpha)), half(weight));
}
half4 nativeGlassExterior_material0_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop) {
float outputMask = nativeGlassOutputCoverage_material0_1_ff(_globals, sdf);
if (outputMask <= 0.0) return half4(0.0h);
float3 shadowSource = float3(blurredBackdrop.xyz) / max(float(blurredBackdrop.w), 1e-06);
half4 result = half4(0.0h);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].w > 0.0) {
float coverage = nativeLightingErfc_material0_1_ff(shiftedShadowSDF / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].x, 1e-06));
float sourceLuma = dot(shadowSource, float3(0.2126, 0.7152, 0.0722));
float3 linear = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[0].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[0].x) * float3(sourceLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[0].z * (shadowSource - float3(sourceLuma));
float fillAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[0].w, 0.0, 1.0);
float3 bias = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[0].x) * (1.0 - fillAlpha) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[0].xyz * fillAlpha;
float contribution = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].z, 0.0, 1.0);
float3 transformed = (linear * (1.0 - fillAlpha)) * contribution + bias;
float nativeAlpha = mix(fillAlpha, 1.0, contribution);
float opacity = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].w, 0.0, 1.0);
float alpha = (coverage * opacity) * nativeAlpha;
transformed /= max(nativeAlpha, 1e-06);
result = half4(half3(transformed * alpha), half(alpha));
}
return result * half(outputMask);
}
half3 nativeGlassBleed_material0_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop) {
float3 color = float3(body);
if (sdf > 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) return body;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].w > 0.0) {
float luminance = clamp(dot(color, float3(0.2125, 0.7154, 0.0721)), 0.0, 1.0);
float tone = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].w > 0.5 ? luminance : 1.0 - luminance;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[0].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[0].x;
float edge = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[0].x) / span, 0.0, 1.0) : float(sdf <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[0].x);
float gain = (tone * tone) * edge;
gain *= gain;
float3 _43_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _44_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].x;
float _45_luma = dot(_43_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_44_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].y - _44_black) * _45_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].z * (_43_c - float3(_45_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].w, 0.0, 1.0));
}
return half3(color);
}
float2 nativeSampleXY_material0_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target) {
float2 offset = target - origin;
return (origin + offset * min(1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[0] / max(length(offset), 0.0001))) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
float nativeRadiusLOD_material0_1_ff(thread Globals& _globals, float radius) {
float r = (((max(radius, 0.0) * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[0];
float minRadius = (((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeMinimumBlurRadius_1[0] * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[0];
float base = float(minRadius >= 2.0);
r *= float(base > 0.5 ? 0.5 : 1.0);
return base + max(0.0, log2(r < 2.0 ? 1.0 + r * 0.5 : r));
}
float2 nativePyramidPosition_material0_1_f2f2f(thread Globals& _globals, float2 worldXY, float level) {
float divisor = exp2(level);
float width = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].x;
float height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].y;
float2 tile = float2(1.0);
if (level > 0.5) {
tile.x = width + 3.0;
tile.y = tile.y + (height * (1.0 - 2.0 / divisor) + 2.0 * (level - 1.0));
}
float2 size = float2(width, height) / divisor;
float2 p = (worldXY * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidRect_1[0].xy) / divisor;
return tile + clamp(p, float2(0.5), max(size - 0.5, float2(0.5)));
}
half4 nativePyramidAt_material0_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod) {
lod = clamp(lod, 0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w);
float first = floor(lod);
float second = min(first + 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w);
half4 low = groupBackdrop_material0_1_h4f2(_globals, nativePyramidPosition_material0_1_f2f2f(_globals, worldXY, first));
if (fract(lod) == 0.0 || first == second) return low;
return mix(low, groupBackdrop_material0_1_h4f2(_globals, nativePyramidPosition_material0_1_f2f2f(_globals, worldXY, second)), half(fract(lod)));
}
half4 nativeAtRadius_material0_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
float2 sampleXY = nativeSampleXY_material0_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w < 0.0) return radius > 0.0 ? groupBackdrop_material0_1_h4f2(_globals, sampleXY) : groupSharp_material0_1_h4f2(_globals, sampleXY);
return nativePyramidAt_material0_1_h4f2f(_globals, groupXY_material0_1_f2f2(_globals, sampleXY), nativeRadiusLOD_material0_1_ff(_globals, radius));
}
float3 nativeUnpremul_material0_1_f3h4(half4 p) {
return float3(p.xyz) / max(float(p.w), 1e-06);
}
float nativeLinearRamp_material0_1_ffff(float x, float a, float b) {
return abs(b - a) < 1e-05 ? step(a, x) : clamp((x - a) / (b - a), 0.0, 1.0);
}
float nativeBlurCoverage_material0_1_ff(thread Globals& _globals, float d) {
return ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].x + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].x) * nativeLinearRamp_material0_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].y)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].y) * nativeLinearRamp_material0_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].z)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[0].z) * nativeLinearRamp_material0_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[0].w);
}
float3 nativeFace_material0_1_f3f3f(thread Globals& _globals, float3 color, float sdf) {
float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
float sdr = 1.0 - step(1.00001, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]);
float maxLuma = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[0].y, sdr);
float compression = clamp(1.0 + (maxLuma - 1.0) * luma, 0.0, 1.0);
float qLuma = luma * compression;
float3 q = float3(qLuma) + (color * compression - float3(qLuma)) * (1.0 + 0.3 * (1.0 - compression));
float3 result = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[0] + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[0].x - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[0]) * qLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[0].y * (q - float3(qLuma));
result = mix(result, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[0].xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[0].w);
return result;
}
float nativeCubic_material0_1_ffff(float t, float p1, float p2) {
float u = 1.0 - t;
return ((((3.0 * u) * u) * t) * p1 + (((3.0 * u) * t) * t) * p2) + (t * t) * t;
}
float nativeTiming_material0_1_fff4f(float x, float4 control, float known) {
if (x <= 0.0 || x >= 1.0) return clamp(x, 0.0, 1.0);
if (known < 0.5) return x;
float low = 0.0;
float high = 1.0;
float t = x;
for (int i = 0;i < 14; ++i) {
float value = nativeCubic_material0_1_ffff(t, control.x, control.z);
if (value < x) low = t; else high = t;
t = (low + high) * 0.5;
}
return nativeCubic_material0_1_ffff(t, control.y, control.w);
}
float4 nativeGradientColor_material0_1_f4f(thread Globals& _globals, float d) {
float4 a = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent0_1[0] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[0];
float4 b = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[0] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent2_1[0];
float lo = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].x : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y;
float hi = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].z;
float raw = abs(hi - lo) > 1e-05 ? clamp((d - lo) / (hi - lo), 0.0, 1.0) : step(lo, d);
float t = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[0].y ? nativeTiming_material0_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming0_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[0].x) : nativeTiming_material0_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming1_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[0].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[0].x > 0.5) {
a.xyz = a.xyz * a.w;
b.xyz = b.xyz * b.w;
float4 c = mix(a, b, t);
c.xyz = c.xyz / max(c.w, 1e-06);
return c;
}
return mix(a, b, t);
}
float nativeBevel_material0_1_ffff(float _0_distance, float height, float radius) {
float t = clamp(-_0_distance / max(height, 1e-06), 0.0, 1.0);
return height > 0.0 ? 1.0 - sqrt(max(t * (2.0 - t), 0.0)) : 0.0;
}
half4 nativeGlassBackgroundBody_material0_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius) {
float inside = nativeBevel_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].y, radius);
float outside = nativeBevel_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].w, radius);
float displacement = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].x * inside;
float2 target = xy + normal * displacement;
float effectiveRadius = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].x * max(nativeBlurCoverage_material0_1_ff(_globals, sdf + displacement), 0.0);
nativeAtRadius_material0_1_h4f2f2f(_globals, xy, target, 0.0);
half4 pixel = nativeAtRadius_material0_1_h4f2f2f(_globals, xy, target, effectiveRadius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[0] > 0.0 && abs(displacement) > 0.001) {
float2 shift = ((normal * abs(displacement)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[0]) * 0.3;
half4 red = nativeAtRadius_material0_1_h4f2f2f(_globals, xy, target + shift, effectiveRadius);
half4 blue = nativeAtRadius_material0_1_h4f2f2f(_globals, xy, target - shift, effectiveRadius);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
float3 rgb = nativeUnpremul_material0_1_f3h4(pixel);
float bodyAlpha = 1.0;
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].x > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].y > 0.0) || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].z > 0.0) {
half4 fill;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w < 0.0) {
fill = nativeAtRadius_material0_1_h4f2f2f(_globals, xy, xy, 0.0);
} else {
float lod = nativeRadiusLOD_material0_1_ff(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].y);
float2 _49_xy = nativeSampleXY_material0_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material0_1_f2f2(_globals, _49_xy);
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].z, 1e-06));
fill = (nativePyramidAt_material0_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_material0_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_material0_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[0].x);
}
float outerMask = nativeLinearRamp_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[0].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[0].z * outerMask > 0.0) {
float outerShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].z * outside;
half4 outer = nativeAtRadius_material0_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].x * max(nativeBlurCoverage_material0_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[0].z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[0].z * outerMask);
}
float3 processed = nativeFace_material0_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[0]);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].w > 0.0) {
float2 _50_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].x) * nativeBevel_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].z, radius);
rgb = float3(nativeGlassBleed_material0_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material0_1_h4f2f2f(_globals, xy, _50_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
float2 groupXY_material1_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material1_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w < 0.0 ? groupXY_material1_1_f2f2(_globals, xy) : xy;
float2 _130_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_16;
half4 _131_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_17 - _130_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_131_e = saturate(_131_e + 0.5h);
float4 _132_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_20;
return (((_131_e.x * _131_e.y) * _131_e.z) * _131_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.zw) * _130_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_18, _132_c.xy, _132_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_20, _globals.image_20);
}
half4 groupSharp_material1_1_h4f2(thread Globals& _globals, float2 xy) {
return LocalMatrix_2_h4f2(_globals, groupXY_material1_1_f2f2(_globals, xy));
}
float nativeGlassOutputCoverage_material1_1_ff(thread Globals& _globals, float sdf) {
float aa = 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
return smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[1].x - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[1].x + aa, sdf) * (1.0 - smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[1].y - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[1].y + aa, sdf));
}
float3 nativeLightingClampColor_material1_1_f3f3ff(float3 c, float limit, float preserveHue) {
if (limit <= 0.0) return c;
if (preserveHue > 0.5) {
float maximum = max(max(c.x, c.y), c.z);
return maximum > limit ? c * (limit / maximum) : c;
}
return clamp(c, float3(-0.75), float3(limit));
}
float nativeLightingErfc_material1_1_ff(float valueInput) {
half x = half(clamp(valueInput, -2.0, 2.0));
half square = x * x;
half value = 0.002954483h * square + -0.0344543457h;
value = value * square + 0.168212891h;
value = value * square + -0.5605469h;
return clamp(float(value * x + 0.5h), 0.0, 1.0);
}
float nativeLightingRingAlpha_material1_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].w <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].y <= 0.0) return 0.0;
float inverseRadius = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].x, 1e-06);
float start = shiftedRingSDF * inverseRadius;
float finish = (shiftedRingSDF + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].y) * inverseRadius;
float ring = clamp(nativeLightingErfc_material1_1_ff(start * 0.707106769) - nativeLightingErfc_material1_1_ff(finish * 0.707106769), 0.0, 1.0);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
return (ring * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].w) * mix(1.0, coverage, clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].z, 0.0, 1.0));
}
float nativeLightingBias_material1_1_fff(float coverage, float amount) {
amount = clamp(amount, 0.0, 1.0);
if (amount <= 0.0 || coverage <= 0.0) return 0.0;
float coefficient = 1.0 / max(amount, 1e-06) - 2.0;
return coverage / max(1.0 + coefficient * (1.0 - coverage), 1e-06);
}
float nativeLightingSDFBand_material1_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal) {
if (height <= 0.0) return 0.0;
float depth = -sdf;
if (depth < -5.0) return 0.0;
float t = clamp(depth / height, 0.0, 1.0);
float shape = mix(float(t < 1.0), 1.0 - t, curvature);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
return (shape * clamp(depth / width + 0.5, 0.0, 1.0)) * clamp((height - depth) / width + 0.5, 0.0, 1.0);
}
float nativeLightingSDFDirection_material1_1_ff2ff(float2 normal, float angle, float spread) {
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(cos(angle - 1.57079637), sin(angle - 1.57079637)));
return clamp((cosine - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
}
float nativeGlassFilterHighlight_material1_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].x <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].z <= 0.0) return 0.0;
float spread = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[1]) <= 1.0 ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].z : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].w;
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(sin(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].y), -cos(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].y)));
float2 angular = clamp((float2(cosine, -cosine) - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
float2 localNormal = (normal * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) / max(localScale, 1.0);
float band;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].z > 0.5) {
float width = max((abs(localNormal.x) + abs(localNormal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
float depth = -(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].x);
band = (1.0 - coverage) * clamp(depth / width + 0.5, 0.0, 1.0);
} else {
band = nativeLightingSDFBand_material1_1_fffff2(_globals, sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].z, 0.75, localNormal);
}
return nativeLightingBias_material1_1_fff(band * angular.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].x) + nativeLightingBias_material1_1_fff(band * angular.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[1].x);
}
half4 nativeGlassHoldingTone_material1_1_h4fh4(thread Globals& _globals, float sdf, half4 material) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].x <= 0.0) return material;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].z;
float progress = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].z) / span, 0.0, 1.0) : 0.0;
float weight = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].x * (1.0 - progress);
float alpha = clamp(float(material.w), 0.0, 1.0);
float3 target = ((float3(material.xyz) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[1].y) * alpha) / max(float(material.w), 1e-06);
return mix(material, half4(half3(target), half(alpha)), half(weight));
}
half4 nativeGlassExterior_material1_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop) {
float outputMask = nativeGlassOutputCoverage_material1_1_ff(_globals, sdf);
if (outputMask <= 0.0) return half4(0.0h);
float3 shadowSource = float3(blurredBackdrop.xyz) / max(float(blurredBackdrop.w), 1e-06);
half4 result = half4(0.0h);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].w > 0.0) {
float coverage = nativeLightingErfc_material1_1_ff(shiftedShadowSDF / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].x, 1e-06));
float sourceLuma = dot(shadowSource, float3(0.2126, 0.7152, 0.0722));
float3 linear = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[1].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[1].x) * float3(sourceLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[1].z * (shadowSource - float3(sourceLuma));
float fillAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[1].w, 0.0, 1.0);
float3 bias = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[1].x) * (1.0 - fillAlpha) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[1].xyz * fillAlpha;
float contribution = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].z, 0.0, 1.0);
float3 transformed = (linear * (1.0 - fillAlpha)) * contribution + bias;
float nativeAlpha = mix(fillAlpha, 1.0, contribution);
float opacity = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].w, 0.0, 1.0);
float alpha = (coverage * opacity) * nativeAlpha;
transformed /= max(nativeAlpha, 1e-06);
result = half4(half3(transformed * alpha), half(alpha));
}
return result * half(outputMask);
}
half3 nativeGlassBleed_material1_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop) {
float3 color = float3(body);
if (sdf > 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) return body;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].w > 0.0) {
float luminance = clamp(dot(color, float3(0.2125, 0.7154, 0.0721)), 0.0, 1.0);
float tone = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].w > 0.5 ? luminance : 1.0 - luminance;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[1].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[1].x;
float edge = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[1].x) / span, 0.0, 1.0) : float(sdf <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[1].x);
float gain = (tone * tone) * edge;
gain *= gain;
float3 _133_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _134_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].x;
float _135_luma = dot(_133_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_134_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].y - _134_black) * _135_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].z * (_133_c - float3(_135_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].w, 0.0, 1.0));
}
return half3(color);
}
float2 nativeSampleXY_material1_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target) {
float2 offset = target - origin;
return (origin + offset * min(1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[1] / max(length(offset), 0.0001))) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
float nativeRadiusLOD_material1_1_ff(thread Globals& _globals, float radius) {
float r = (((max(radius, 0.0) * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[1];
float minRadius = (((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeMinimumBlurRadius_1[1] * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[1];
float base = float(minRadius >= 2.0);
r *= float(base > 0.5 ? 0.5 : 1.0);
return base + max(0.0, log2(r < 2.0 ? 1.0 + r * 0.5 : r));
}
float2 nativePyramidPosition_material1_1_f2f2f(thread Globals& _globals, float2 worldXY, float level) {
float divisor = exp2(level);
float width = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].x;
float height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].y;
float2 tile = float2(1.0);
if (level > 0.5) {
tile.x = width + 3.0;
tile.y = tile.y + (height * (1.0 - 2.0 / divisor) + 2.0 * (level - 1.0));
}
float2 size = float2(width, height) / divisor;
float2 p = (worldXY * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidRect_1[1].xy) / divisor;
return tile + clamp(p, float2(0.5), max(size - 0.5, float2(0.5)));
}
half4 nativePyramidAt_material1_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod) {
lod = clamp(lod, 0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w);
float first = floor(lod);
float second = min(first + 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w);
half4 low = groupBackdrop_material1_1_h4f2(_globals, nativePyramidPosition_material1_1_f2f2f(_globals, worldXY, first));
if (fract(lod) == 0.0 || first == second) return low;
return mix(low, groupBackdrop_material1_1_h4f2(_globals, nativePyramidPosition_material1_1_f2f2f(_globals, worldXY, second)), half(fract(lod)));
}
half4 nativeAtRadius_material1_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
float2 sampleXY = nativeSampleXY_material1_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w < 0.0) return radius > 0.0 ? groupBackdrop_material1_1_h4f2(_globals, sampleXY) : groupSharp_material1_1_h4f2(_globals, sampleXY);
return nativePyramidAt_material1_1_h4f2f(_globals, groupXY_material1_1_f2f2(_globals, sampleXY), nativeRadiusLOD_material1_1_ff(_globals, radius));
}
float3 nativeUnpremul_material1_1_f3h4(half4 p) {
return float3(p.xyz) / max(float(p.w), 1e-06);
}
float nativeLinearRamp_material1_1_ffff(float x, float a, float b) {
return abs(b - a) < 1e-05 ? step(a, x) : clamp((x - a) / (b - a), 0.0, 1.0);
}
float nativeBlurCoverage_material1_1_ff(thread Globals& _globals, float d) {
return ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].x + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].x) * nativeLinearRamp_material1_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].y)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].y) * nativeLinearRamp_material1_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].z)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[1].z) * nativeLinearRamp_material1_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[1].w);
}
float3 nativeFace_material1_1_f3f3f(thread Globals& _globals, float3 color, float sdf) {
float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
float sdr = 1.0 - step(1.00001, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]);
float maxLuma = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[1].y, sdr);
float compression = clamp(1.0 + (maxLuma - 1.0) * luma, 0.0, 1.0);
float qLuma = luma * compression;
float3 q = float3(qLuma) + (color * compression - float3(qLuma)) * (1.0 + 0.3 * (1.0 - compression));
float3 result = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[1] + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[1].x - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[1]) * qLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[1].y * (q - float3(qLuma));
result = mix(result, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[1].xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[1].w);
return result;
}
float nativeCubic_material1_1_ffff(float t, float p1, float p2) {
float u = 1.0 - t;
return ((((3.0 * u) * u) * t) * p1 + (((3.0 * u) * t) * t) * p2) + (t * t) * t;
}
float nativeTiming_material1_1_fff4f(float x, float4 control, float known) {
if (x <= 0.0 || x >= 1.0) return clamp(x, 0.0, 1.0);
if (known < 0.5) return x;
float low = 0.0;
float high = 1.0;
float t = x;
for (int i = 0;i < 14; ++i) {
float value = nativeCubic_material1_1_ffff(t, control.x, control.z);
if (value < x) low = t; else high = t;
t = (low + high) * 0.5;
}
return nativeCubic_material1_1_ffff(t, control.y, control.w);
}
float4 nativeGradientColor_material1_1_f4f(thread Globals& _globals, float d) {
float4 a = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent0_1[1] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[1];
float4 b = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[1] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent2_1[1];
float lo = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].x : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y;
float hi = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].z;
float raw = abs(hi - lo) > 1e-05 ? clamp((d - lo) / (hi - lo), 0.0, 1.0) : step(lo, d);
float t = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[1].y ? nativeTiming_material1_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming0_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[1].x) : nativeTiming_material1_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming1_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[1].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[1].x > 0.5) {
a.xyz = a.xyz * a.w;
b.xyz = b.xyz * b.w;
float4 c = mix(a, b, t);
c.xyz = c.xyz / max(c.w, 1e-06);
return c;
}
return mix(a, b, t);
}
float nativeBevel_material1_1_ffff(float _0_distance, float height, float radius) {
float t = clamp(-_0_distance / max(height, 1e-06), 0.0, 1.0);
return height > 0.0 ? 1.0 - sqrt(max(t * (2.0 - t), 0.0)) : 0.0;
}
half4 nativeGlassBackgroundBody_material1_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius) {
float inside = nativeBevel_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].y, radius);
float outside = nativeBevel_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].w, radius);
float displacement = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].x * inside;
float2 target = xy + normal * displacement;
float effectiveRadius = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].x * max(nativeBlurCoverage_material1_1_ff(_globals, sdf + displacement), 0.0);
nativeAtRadius_material1_1_h4f2f2f(_globals, xy, target, 0.0);
half4 pixel = nativeAtRadius_material1_1_h4f2f2f(_globals, xy, target, effectiveRadius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[1] > 0.0 && abs(displacement) > 0.001) {
float2 shift = ((normal * abs(displacement)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[1]) * 0.3;
half4 red = nativeAtRadius_material1_1_h4f2f2f(_globals, xy, target + shift, effectiveRadius);
half4 blue = nativeAtRadius_material1_1_h4f2f2f(_globals, xy, target - shift, effectiveRadius);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
float3 rgb = nativeUnpremul_material1_1_f3h4(pixel);
float bodyAlpha = 1.0;
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].x > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].y > 0.0) || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].z > 0.0) {
half4 fill;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w < 0.0) {
fill = nativeAtRadius_material1_1_h4f2f2f(_globals, xy, xy, 0.0);
} else {
float lod = nativeRadiusLOD_material1_1_ff(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].y);
float2 _139_xy = nativeSampleXY_material1_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material1_1_f2f2(_globals, _139_xy);
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].z, 1e-06));
fill = (nativePyramidAt_material1_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_material1_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_material1_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[1].x);
}
float outerMask = nativeLinearRamp_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[1].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[1].z * outerMask > 0.0) {
float outerShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].z * outside;
half4 outer = nativeAtRadius_material1_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].x * max(nativeBlurCoverage_material1_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[1].z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[1].z * outerMask);
}
float3 processed = nativeFace_material1_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[1]);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].w > 0.0) {
float2 _140_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].x) * nativeBevel_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].z, radius);
rgb = float3(nativeGlassBleed_material1_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material1_1_h4f2f2f(_globals, xy, _140_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 xy = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
bool covered = false;
covered = covered || ((xy.x >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].x && xy.y >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].y) && xy.x <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].z) && xy.y <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].w;
covered = covered || ((xy.x >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].x && xy.y >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].y) && xy.x <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].z) && xy.y <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].w;
if (!covered) {
bool blendPossible = false;
float2 bp0 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].xy * 0.5;
float bd0 = circularBoxSDF_1_ff2f2f(bp0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].xy * 0.5, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].z, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].y) * 0.5)) * abs(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x);
float2 bp1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].xy * 0.5;
float bd1 = circularBoxSDF_1_ff2f2f(bp1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].xy * 0.5, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].z, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].y) * 0.5)) * abs(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x);
blendPossible = blendPossible || abs(bd1 - bd0) <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * 2.0;
if (!blendPossible) return LocalMatrix_2_h4f2(_globals, pos);
}
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
float _220_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _220_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _220_cover;
_globals.weights_1[1] *= 1.0 - _220_cover;
_globals.weights_1[0] += _220_cover;
}
{
float _221_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _221_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _221_cover;
_globals.weights_1[1] *= 1.0 - _221_cover;
_globals.weights_1[1] += _221_cover;
}
{
float _222_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _222_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _222_cover;
_globals.weights_1[1] *= 1.0 - _222_cover;
_globals.weights_1[0] += _222_cover;
}
{
float _223_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _223_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _223_cover;
_globals.weights_1[1] *= 1.0 - _223_cover;
_globals.weights_1[1] += _223_cover;
}
half4 original = LocalMatrix_2_h4f2(_globals, pos);
float coverage = clamp(0.5 - sceneDistance * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
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
float2 localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0)));
float2 localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y * sceneNormal.y);
float localMetric = max(length(localGradient), 0.0001);
_globals.materialNormal_1 = localGradient / localMetric;
_globals.interactionIntensity_1 = groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(groupTouch, 1.0)));
float localDistance = sceneDistance / localMetric;
if (nativeGlassOutputCoverage_material0_1_ff(_globals, localDistance) > 0.0 || coverage > 0.0) {
float ringDistance = localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].y > 0.0) {
float2 ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[0];
ringDistance = mergeGeometry_1_ff2b(_globals, xy - ringOffset, false) / localMetric;
}
float shadowDistance = localDistance;
if (coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].w > 0.0)) {
float2 shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[0]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[0]));
shadowDistance = mergeGeometry_1_ff2b(_globals, xy - shadowOffset, false) / localMetric;
}
float _224_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[0], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 0.5);
half4 _225_field;
if (coverage >= 1.0) _225_field = nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _224_radius); else {
float _226_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].x * nativeBevel_material0_1_ffff(localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].z, _224_radius);
half4 _227_shadow = nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, localXY + _globals.materialNormal_1 * _226_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].w);
_225_field = nativeGlassExterior_material0_1_h4ffff2h4h4h4(_globals, localDistance, shadowDistance, ringDistance, _globals.materialNormal_1, original, _227_shadow, original);
if (coverage > 0.0) _225_field = mix(_225_field, nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _224_radius), half(coverage));
}
float _228_alpha = clamp(nativeLightingRingAlpha_material0_1_ffff2(_globals, localDistance, ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _229_foreground = half4(0.0h, 0.0h, 0.0h, half(_228_alpha));
_225_field = _229_foreground + _225_field * (1.0h - _229_foreground.w);
half4 _230_material = _225_field;
half3 _231_exteriorFace = half3(nativeFace_material0_1_f3f3f(_globals, nativeUnpremul_material0_1_f3h4(original), localDistance));
float _232_amount = nativeGlassFilterHighlight_material0_1_fff2f(_globals, localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * localMetric);
if (_232_amount > 1e-06) {
float3 _233_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _234_face = float3(_231_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].x > 0.5) _234_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y < 0.0 ? min(_234_face, _233_raw) : max(_234_face, _233_raw);
float _235_alpha = float(_230_material.w);
float3 _236_color = float3(_230_material.xyz) + (1.0 - _235_alpha) * mix(_233_raw, _234_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[0] * _232_amount);
_236_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y * _232_amount) * (3.0 - 2.0 * _236_color);
float _237_resultAlpha = _235_alpha + ((1.0 - _235_alpha) * _232_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].y;
_230_material = half4(half3(_236_color - (1.0 - _237_resultAlpha) * _233_raw), half(_237_resultAlpha));
}
_225_field = nativeGlassHoldingTone_material0_1_h4fh4(_globals, localDistance, _230_material);
float3 _238_fieldColor = nativeLightingClampColor_material0_1_f3f3ff(nativeUnpremul_material0_1_f3h4(_225_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
_225_field = half4(half3(_238_fieldColor) * _225_field.w, _225_field.w);
half4 _239_composited = _225_field + original * (1.0h - _225_field.w);
float3 _240_clamped = clamp(nativeUnpremul_material0_1_f3h4(_239_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0])));
_239_composited = half4(half3(_240_clamped) * _239_composited.w, _239_composited.w);
float3 _241_rgb = nativeUnpremul_material0_1_f3h4(_239_composited);
float _242_materialAlpha = float(_239_composited.w);
float _243_inside = nativeBevel_material0_1_ffff(localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].y, _224_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[0] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w > 0.0) {
float4 _244_gradient = nativeGradientColor_material0_1_f4f(_globals, localDistance);
float4 _245_matrixInput = float4(_241_rgb, float(_239_composited.w));
float4 _246_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[0], _245_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[0], _245_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[0], _245_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[0], _245_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[0];
_246_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_246_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].y);
float4 _247_accent = _246_result;
_247_accent.w = clamp(_247_accent.w, 0.0, 1.0);
_247_accent.xyz = _247_accent.xyz * _247_accent.w;
float _248_mask = _244_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[0].y > 0.5) _248_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w; else {
_247_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _247_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
_247_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _247_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
}
_247_accent *= clamp(_248_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w, 0.0, 1.0);
float _249_outAlpha = _247_accent.w + _242_materialAlpha * (1.0 - _247_accent.w);
_241_rgb = (_247_accent.xyz + (_241_rgb * _242_materialAlpha) * (1.0 - _247_accent.w)) / max(_249_outAlpha, 1e-06);
_242_materialAlpha = _249_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] >= 0.0) {
_241_rgb = mix(_241_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w) * coverage, 0.0, 1.0));
}
_241_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[0] * _243_inside;
_241_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[0] * smoothstep(0.85, 0.98, dot(_241_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _250_unlit = _241_rgb;
float3 _251_color = float3(half3(_241_rgb));
float2 _252_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? sceneNormal : _globals.materialNormal_1;
float _253_norm = length(_252_normal);
if (_253_norm > 1e-05) _252_normal /= _253_norm;
float _254_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].y);
float _255_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].y);
float _256_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].w;
float _257_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].w;
float _258_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].x;
float _259_key = 0.0;
float _260_fill = 0.0;
float _261_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? localDistance / max(_253_norm, 1e-06) : localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z > 0.0) {
float _262_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _261_localSDF, _254_keyHeight, _258_curvature, _252_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_252_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _256_keySpread);
float _263_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _261_localSDF, _254_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _252_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_252_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _256_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_259_key = nativeLightingBias_material0_1_fff(_262_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_263_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z > 0.0) {
float _264_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _261_localSDF, _255_fillHeight, _258_curvature, _252_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_252_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _257_fillSpread);
float _265_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _261_localSDF, _255_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _252_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_252_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _257_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_260_fill = nativeLightingBias_material0_1_fff(_264_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_265_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
float3 _266_unlitColor = _251_color;
float4 _267_c = float4(_266_unlitColor, 1.0);
float4 _268_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[0], _267_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[0], _267_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[0], _267_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[0], _267_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[0];
_268_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_268_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].y);
float4 _269_transformed = _268_result;
float _270_coverage = _259_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[0].w + _260_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[0].w;
_270_coverage *= clamp(_269_transformed.w, 0.0, 1.0);
_251_color = _266_unlitColor + _270_coverage * (_269_transformed.xyz - _266_unlitColor);
_241_rgb = float3(half3(nativeLightingClampColor_material0_1_f3f3ff(_251_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[0])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y)));
float _271_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[0], clamp(dot(_250_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_241_rgb = mix(_250_unlit, _241_rgb, _271_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0] > 0.0 && _243_inside > 0.0) {
float2 _272_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[0];
half4 _273_reflected = (nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) + _272_tangent, 0.0) + nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) - _272_tangent, 0.0)) * 0.5h;
_241_rgb = mix(_241_rgb, nativeUnpremul_material0_1_f3h4(_273_reflected), (_243_inside * _243_inside) * 0.1);
}
float _274_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * coverage;
if (_274_glow > 0.0) {
float _275_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 1.6);
float _276_sheen = 1.0 - smoothstep(0.0, _275_glowRadius, length(localXY - _globals.interactionLocation_1));
float _277_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float3 _278_restingResult = _241_rgb;
_241_rgb = mix(_241_rgb, float3(1.0), _274_glow * _277_ambientPressOpacity);
_241_rgb = mix(_241_rgb, float3(1.0), (_274_glow * _276_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float _279_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y));
float _280_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y);
float _281_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] < 0.5) * smoothstep(160.0, 280.0, _280_longSide)) * smoothstep(2.5, 4.0, _280_longSide / _279_shortSide);
float _282_peak = max(_278_restingResult.x, max(_278_restingResult.y, _278_restingResult.z));
float _283_chromaticRange = _282_peak - min(_278_restingResult.x, min(_278_restingResult.y, _278_restingResult.z));
float _284_neutralPress = 1.0 - smoothstep(0.01, 0.1, _283_chromaticRange);
float _285_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]);
float _286_pressGain = mix(1.18 - 0.06 * _282_peak, 1.06, _285_lightAppearance);
float3 _287_colorPress = mix(float3(float(dot(half3(_278_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _278_restingResult, mix(1.03, 1.06, _285_lightAppearance)) * _286_pressGain;
_287_colorPress += float3(_284_neutralPress * mix(0.02745098, 0.04509804, _285_lightAppearance));
_241_rgb = mix(_241_rgb, mix(_278_restingResult, _287_colorPress, _274_glow), _281_platterPress);
}
_241_rgb = nativeLightingClampColor_material0_1_f3f3ff(_241_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
float _288_alpha = clamp(_242_materialAlpha, 0.0, 1.0);
half4 color = half4(half3(_241_rgb * _288_alpha), half(_288_alpha));
result += half(_globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0]) * (color - original);
}
}
if (_globals.weights_1[1] > 0.0) {
float2 localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0)));
float2 localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y * sceneNormal.y);
float localMetric = max(length(localGradient), 0.0001);
_globals.materialNormal_1 = localGradient / localMetric;
_globals.interactionIntensity_1 = groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(groupTouch, 1.0)));
float localDistance = sceneDistance / localMetric;
if (nativeGlassOutputCoverage_material1_1_ff(_globals, localDistance) > 0.0 || coverage > 0.0) {
float ringDistance = localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].y > 0.0) {
float2 ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[1];
ringDistance = mergeGeometry_1_ff2b(_globals, xy - ringOffset, false) / localMetric;
}
float shadowDistance = localDistance;
if (coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].w > 0.0)) {
float2 shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[1]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[1]));
shadowDistance = mergeGeometry_1_ff2b(_globals, xy - shadowOffset, false) / localMetric;
}
float _289_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[1], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 0.5);
half4 _290_field;
if (coverage >= 1.0) _290_field = nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _289_radius); else {
float _291_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].x * nativeBevel_material1_1_ffff(localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].z, _289_radius);
half4 _292_shadow = nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, localXY + _globals.materialNormal_1 * _291_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].w);
_290_field = nativeGlassExterior_material1_1_h4ffff2h4h4h4(_globals, localDistance, shadowDistance, ringDistance, _globals.materialNormal_1, original, _292_shadow, original);
if (coverage > 0.0) _290_field = mix(_290_field, nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _289_radius), half(coverage));
}
float _293_alpha = clamp(nativeLightingRingAlpha_material1_1_ffff2(_globals, localDistance, ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _294_foreground = half4(0.0h, 0.0h, 0.0h, half(_293_alpha));
_290_field = _294_foreground + _290_field * (1.0h - _294_foreground.w);
half4 _295_material = _290_field;
half3 _296_exteriorFace = half3(nativeFace_material1_1_f3f3f(_globals, nativeUnpremul_material1_1_f3h4(original), localDistance));
float _297_amount = nativeGlassFilterHighlight_material1_1_fff2f(_globals, localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * localMetric);
if (_297_amount > 1e-06) {
float3 _298_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _299_face = float3(_296_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].x > 0.5) _299_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y < 0.0 ? min(_299_face, _298_raw) : max(_299_face, _298_raw);
float _300_alpha = float(_295_material.w);
float3 _301_color = float3(_295_material.xyz) + (1.0 - _300_alpha) * mix(_298_raw, _299_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[1] * _297_amount);
_301_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y * _297_amount) * (3.0 - 2.0 * _301_color);
float _302_resultAlpha = _300_alpha + ((1.0 - _300_alpha) * _297_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].y;
_295_material = half4(half3(_301_color - (1.0 - _302_resultAlpha) * _298_raw), half(_302_resultAlpha));
}
_290_field = nativeGlassHoldingTone_material1_1_h4fh4(_globals, localDistance, _295_material);
float3 _303_fieldColor = nativeLightingClampColor_material1_1_f3f3ff(nativeUnpremul_material1_1_f3h4(_290_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
_290_field = half4(half3(_303_fieldColor) * _290_field.w, _290_field.w);
half4 _304_composited = _290_field + original * (1.0h - _290_field.w);
float3 _305_clamped = clamp(nativeUnpremul_material1_1_f3h4(_304_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1])));
_304_composited = half4(half3(_305_clamped) * _304_composited.w, _304_composited.w);
float3 _306_rgb = nativeUnpremul_material1_1_f3h4(_304_composited);
float _307_materialAlpha = float(_304_composited.w);
float _308_inside = nativeBevel_material1_1_ffff(localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].y, _289_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[1] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w > 0.0) {
float4 _309_gradient = nativeGradientColor_material1_1_f4f(_globals, localDistance);
float4 _310_matrixInput = float4(_306_rgb, float(_304_composited.w));
float4 _311_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[1], _310_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[1], _310_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[1], _310_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[1], _310_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[1];
_311_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_311_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].y);
float4 _312_accent = _311_result;
_312_accent.w = clamp(_312_accent.w, 0.0, 1.0);
_312_accent.xyz = _312_accent.xyz * _312_accent.w;
float _313_mask = _309_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[1].y > 0.5) _313_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w; else {
_312_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _312_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
_312_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _312_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
}
_312_accent *= clamp(_313_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w, 0.0, 1.0);
float _314_outAlpha = _312_accent.w + _307_materialAlpha * (1.0 - _312_accent.w);
_306_rgb = (_312_accent.xyz + (_306_rgb * _307_materialAlpha) * (1.0 - _312_accent.w)) / max(_314_outAlpha, 1e-06);
_307_materialAlpha = _314_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] >= 0.0) {
_306_rgb = mix(_306_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w) * coverage, 0.0, 1.0));
}
_306_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[1] * _308_inside;
_306_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[1] * smoothstep(0.85, 0.98, dot(_306_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _315_unlit = _306_rgb;
float3 _316_color = float3(half3(_306_rgb));
float2 _317_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? sceneNormal : _globals.materialNormal_1;
float _318_norm = length(_317_normal);
if (_318_norm > 1e-05) _317_normal /= _318_norm;
float _319_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].y);
float _320_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].y);
float _321_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].w;
float _322_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].w;
float _323_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].x;
float _324_key = 0.0;
float _325_fill = 0.0;
float _326_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? localDistance / max(_318_norm, 1e-06) : localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z > 0.0) {
float _327_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _326_localSDF, _319_keyHeight, _323_curvature, _317_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_317_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _321_keySpread);
float _328_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _326_localSDF, _319_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _317_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_317_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _321_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_324_key = nativeLightingBias_material1_1_fff(_327_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_328_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z > 0.0) {
float _329_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _326_localSDF, _320_fillHeight, _323_curvature, _317_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_317_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _322_fillSpread);
float _330_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _326_localSDF, _320_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _317_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_317_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _322_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_325_fill = nativeLightingBias_material1_1_fff(_329_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_330_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
float3 _331_unlitColor = _316_color;
float4 _332_c = float4(_331_unlitColor, 1.0);
float4 _333_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[1], _332_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[1], _332_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[1], _332_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[1], _332_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[1];
_333_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_333_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].y);
float4 _334_transformed = _333_result;
float _335_coverage = _324_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[1].w + _325_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[1].w;
_335_coverage *= clamp(_334_transformed.w, 0.0, 1.0);
_316_color = _331_unlitColor + _335_coverage * (_334_transformed.xyz - _331_unlitColor);
_306_rgb = float3(half3(nativeLightingClampColor_material1_1_f3f3ff(_316_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[1])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y)));
float _336_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[1], clamp(dot(_315_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_306_rgb = mix(_315_unlit, _306_rgb, _336_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1] > 0.0 && _308_inside > 0.0) {
float2 _337_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[1];
half4 _338_reflected = (nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) + _337_tangent, 0.0) + nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) - _337_tangent, 0.0)) * 0.5h;
_306_rgb = mix(_306_rgb, nativeUnpremul_material1_1_f3h4(_338_reflected), (_308_inside * _308_inside) * 0.1);
}
float _339_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * coverage;
if (_339_glow > 0.0) {
float _340_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 1.6);
float _341_sheen = 1.0 - smoothstep(0.0, _340_glowRadius, length(localXY - _globals.interactionLocation_1));
float _342_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float3 _343_restingResult = _306_rgb;
_306_rgb = mix(_306_rgb, float3(1.0), _339_glow * _342_ambientPressOpacity);
_306_rgb = mix(_306_rgb, float3(1.0), (_339_glow * _341_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float _344_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y));
float _345_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y);
float _346_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] < 0.5) * smoothstep(160.0, 280.0, _345_longSide)) * smoothstep(2.5, 4.0, _345_longSide / _344_shortSide);
float _347_peak = max(_343_restingResult.x, max(_343_restingResult.y, _343_restingResult.z));
float _348_chromaticRange = _347_peak - min(_343_restingResult.x, min(_343_restingResult.y, _343_restingResult.z));
float _349_neutralPress = 1.0 - smoothstep(0.01, 0.1, _348_chromaticRange);
float _350_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]);
float _351_pressGain = mix(1.18 - 0.06 * _347_peak, 1.06, _350_lightAppearance);
float3 _352_colorPress = mix(float3(float(dot(half3(_343_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _343_restingResult, mix(1.03, 1.06, _350_lightAppearance)) * _351_pressGain;
_352_colorPress += float3(_349_neutralPress * mix(0.02745098, 0.04509804, _350_lightAppearance));
_306_rgb = mix(_306_rgb, mix(_343_restingResult, _352_colorPress, _339_glow), _346_platterPress);
}
_306_rgb = nativeLightingClampColor_material1_1_f3f3ff(_306_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
float _353_alpha = clamp(_307_materialAlpha, 0.0, 1.0);
half4 color = half4(half3(_306_rgb * _353_alpha), half(_353_alpha));
result += half(_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * (color - original);
}
}
return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], texture2d<half> image_20_Tex [[texture(2)]], sampler image_20_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}, {image_13_Tex, image_13_Smplr}, {image_20_Tex, image_20_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
