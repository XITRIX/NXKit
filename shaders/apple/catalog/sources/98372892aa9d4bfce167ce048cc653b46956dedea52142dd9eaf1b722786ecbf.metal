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
float4 upper2x2_16;
float2 translation_16;
float4 decalBounds_17;
float4 upper2x2_18;
float2 translation_18;
float2 invImgSize_20;
float4 subsetInsetClamp_20;
float4 upper2x2_23;
float2 translation_23;
float4 decalBounds_24;
float4 upper2x2_25;
float2 translation_25;
float2 invImgSize_27;
float4 subsetInsetClamp_27;
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
    sampler2D image_27;
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
half4 LocalMatrix_2_h4f2(thread Globals& _globals, float2 pos) {
float2 _5_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _6_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _5_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_6_e = saturate(_6_e + 0.5h);
float4 _7_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
return (((_6_e.x * _6_e.y) * _6_e.z) * _6_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _5_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _7_c.xy, _7_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
}
half4 sk_decal_image_10_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_17_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_24_h4f2f4(float2 b, float4 c);
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
float2 groupXY_material2_1_f2f2(thread Globals& _globals, float2 xy);
half4 groupBackdrop_material2_1_h4f2(thread Globals& _globals, float2 xy);
half4 groupSharp_material2_1_h4f2(thread Globals& _globals, float2 xy);
float toBrightness_material2_1_fh3(half3 c);
float nativeLightingAA_material2_1_f();
float nativeLightingHeadroom_material2_1_f();
float nativeLightingLuma_material2_1_ff3(float3 c);
float3 nativeLightingUnpremul_material2_1_f3h4(half4 c);
float nativeGlassOutputCoverage_material2_1_ff(thread Globals& _globals, float sdf);
float3 nativeLightingTransfer_material2_1_f3f3fff(float3 c, float black, float white, float saturation);
float3 nativeLightingClampColor_material2_1_f3f3ff(float3 c, float limit, float preserveHue);
float nativeLightingErfc_material2_1_ff(float valueInput);
float nativeLightingRingAlpha_material2_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal);
half4 nativeLightingOver_material2_1_h4h4h4(half4 foreground, half4 background);
float nativeLightingBias_material2_1_fff(float coverage, float amount);
float nativeLightingSDFBand_material2_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal);
float nativeLightingSDFDirection_material2_1_ff2ff(float2 normal, float angle, float spread);
float nativeGlassFilterHighlight_material2_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale);
half4 nativeGlassHoldingTone_material2_1_h4fh4(thread Globals& _globals, float sdf, half4 material);
half4 nativeGlassBackgroundFinalize_material2_1_h4fh4h4h3ff2f(float sdf, half4 material, half4 original, half3 exteriorFace, float faceOpacity, float2 normal, float localScale);
half4 nativeGlassExterior_material2_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop);
half3 nativeGlassBleed_material2_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop);
half4 nativeGlassRingComposite_material2_1_h4fff2fh4(float sdf, float shiftedRingSDF, float2 normal, float fieldAlpha, half4 material);
float4 nativeLightingHighlightMatrix_material2_1_f4f4(float4 c);
half3 nativeGlassHighlight_material2_1_h3ff2f2h3(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop);
float2 nativeSampleXY_material2_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target);
float2 nativeSamplingWorld_material2_1_f2f2(float2 xy);
float nativeRadiusLOD_material2_1_ff(thread Globals& _globals, float radius);
float2 nativePyramidPosition_material2_1_f2f2f(thread Globals& _globals, float2 worldXY, float level);
half4 nativePyramidAt_material2_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod);
half4 nativeAtRadius_material2_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius);
half4 nativeSharp_material2_1_h4f2f2(float2 origin, float2 target);
half4 nativeBleed_material2_1_h4f2f2(float2 origin, float2 target);
half4 nativeShadow_material2_1_h4f2f2(float2 origin, float2 target);
float3 nativeUnpremul_material2_1_f3h4(half4 p);
float nativeLinearRamp_material2_1_ffff(float x, float a, float b);
float nativeBlurCoverage_material2_1_ff(thread Globals& _globals, float d);
float3 nativeFace_material2_1_f3f3f(thread Globals& _globals, float3 color, float sdf);
float4 nativeAccentMatrix_material2_1_f4f4(float4 color);
float nativeCubic_material2_1_ffff(float t, float p1, float p2);
float nativeTiming_material2_1_fff4f(float x, float4 control, float known);
float4 nativeGradientColor_material2_1_f4f(thread Globals& _globals, float d);
float nativeBevel_material2_1_ffff(float _0_distance, float height, float radius);
half4 nativeGlassBackgroundBody_material2_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius);
half4 nativeGlassDecorate_material2_1_h4f2ff2f2fh4f(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, half4 material, float surfaceCoverage);
half4 nativeGlassSurface_material2_1_h4f2ff2f2fffh4ff(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, float shadowSDF, float ringSDF, half4 original, float coverage, float localScale);
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
float2 _32_q = (abs(p) - halfDim) + r;
return float3((length(max(_32_q, 0.0)) + min(max(_32_q.x, _32_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record) {
float4 _33_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _34_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _33_g.xy * 0.5;
_globals.cornerCurve_1 = _33_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _35_r = min(_33_g.z, min(_33_g.x, _33_g.y) * 0.5);
float3 _36_surface = roundedBoxSample_1_f3f2f2f(_globals, _34_p, _33_g.xy * 0.5, _35_r);
float _37_d = _36_surface.x;
float2 _38_n = _36_surface.yz;
float2 _39_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _38_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _38_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _38_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _38_n.y);
float _40_magnitude = max(length(_39_gradient), 0.0001);
float3 first = float3(_37_d / _40_magnitude, _39_gradient / _40_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _41_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _42_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _41_g.xy * 0.5;
_globals.cornerCurve_1 = _41_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _43_r = min(_41_g.z, min(_41_g.x, _41_g.y) * 0.5);
float3 _44_surface = roundedBoxSample_1_f3f2f2f(_globals, _42_p, _41_g.xy * 0.5, _43_r);
float _45_d = _44_surface.x;
float2 _46_n = _44_surface.yz;
float2 _47_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _46_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _46_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _46_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _46_n.y);
float _48_magnitude = max(length(_47_gradient), 0.0001);
float3 item = float3(_45_d / _48_magnitude, _47_gradient / _48_magnitude);
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
float4 _49_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2];
float2 _50_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0))) - _49_g.xy * 0.5;
_globals.cornerCurve_1 = _49_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[2];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[2];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[2];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[2];
float _51_r = min(_49_g.z, min(_49_g.x, _49_g.y) * 0.5);
float3 _52_surface = roundedBoxSample_1_f3f2f2f(_globals, _50_p, _49_g.xy * 0.5, _51_r);
float _53_d = _52_surface.x;
float2 _54_n = _52_surface.yz;
float2 _55_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].x * _54_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].x * _54_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].y * _54_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].y * _54_n.y);
float _56_magnitude = max(length(_55_gradient), 0.0001);
float3 item = float3(_53_d / _56_magnitude, _55_gradient / _56_magnitude);
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
float4 _57_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[3];
float2 _58_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].xyz, float3(xy, 1.0))) - _57_g.xy * 0.5;
_globals.cornerCurve_1 = _57_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[3];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[3];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[3];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[3];
float _59_r = min(_57_g.z, min(_57_g.x, _57_g.y) * 0.5);
float3 _60_surface = roundedBoxSample_1_f3f2f2f(_globals, _58_p, _57_g.xy * 0.5, _59_r);
float _61_d = _60_surface.x;
float2 _62_n = _60_surface.yz;
float2 _63_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].x * _62_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].x * _62_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].y * _62_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].y * _62_n.y);
float _64_magnitude = max(length(_63_gradient), 0.0001);
float3 item = float3(_61_d / _64_magnitude, _63_gradient / _64_magnitude);
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
float2 groupXY_material0_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material0_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w < 0.0 ? groupXY_material0_1_f2f2(_globals, xy) : xy;
float2 _65_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _66_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _65_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_66_e = saturate(_66_e + 0.5h);
float4 _67_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
return (((_66_e.x * _66_e.y) * _66_e.z) * _66_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _65_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _67_c.xy, _67_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
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
float3 _68_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _69_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].x;
float _70_luma = dot(_68_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_69_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].y - _69_black) * _70_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].z * (_68_c - float3(_70_luma));
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
float2 _74_xy = nativeSampleXY_material0_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material0_1_f2f2(_globals, _74_xy);
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
float2 _75_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].x) * nativeBevel_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].z, radius);
rgb = float3(nativeGlassBleed_material0_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material0_1_h4f2f2f(_globals, xy, _75_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
float2 groupXY_material1_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material1_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w < 0.0 ? groupXY_material1_1_f2f2(_globals, xy) : xy;
float2 _155_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_16;
half4 _156_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_17 - _155_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_156_e = saturate(_156_e + 0.5h);
float4 _157_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_20;
return (((_156_e.x * _156_e.y) * _156_e.z) * _156_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.zw) * _155_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_18, _157_c.xy, _157_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_20, _globals.image_20);
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
float3 _158_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _159_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].x;
float _160_luma = dot(_158_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_159_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].y - _159_black) * _160_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].z * (_158_c - float3(_160_luma));
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
float2 _164_xy = nativeSampleXY_material1_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material1_1_f2f2(_globals, _164_xy);
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
float2 _165_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].x) * nativeBevel_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].z, radius);
rgb = float3(nativeGlassBleed_material1_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material1_1_h4f2f2f(_globals, xy, _165_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
float2 groupXY_material2_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material2_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w < 0.0 ? groupXY_material2_1_f2f2(_globals, xy) : xy;
float2 _245_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_23.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_23.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_23;
half4 _246_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_24 - _245_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_246_e = saturate(_246_e + 0.5h);
float4 _247_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_27;
return (((_246_e.x * _246_e.y) * _246_e.z) * _246_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_25.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_25.zw) * _245_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_25, _247_c.xy, _247_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_27, _globals.image_27);
}
half4 groupSharp_material2_1_h4f2(thread Globals& _globals, float2 xy) {
return LocalMatrix_2_h4f2(_globals, groupXY_material2_1_f2f2(_globals, xy));
}
float nativeGlassOutputCoverage_material2_1_ff(thread Globals& _globals, float sdf) {
float aa = 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
return smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[2].x - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[2].x + aa, sdf) * (1.0 - smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[2].y - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeOutputDistance_1[2].y + aa, sdf));
}
float3 nativeLightingClampColor_material2_1_f3f3ff(float3 c, float limit, float preserveHue) {
if (limit <= 0.0) return c;
if (preserveHue > 0.5) {
float maximum = max(max(c.x, c.y), c.z);
return maximum > limit ? c * (limit / maximum) : c;
}
return clamp(c, float3(-0.75), float3(limit));
}
float nativeLightingErfc_material2_1_ff(float valueInput) {
half x = half(clamp(valueInput, -2.0, 2.0));
half square = x * x;
half value = 0.002954483h * square + -0.0344543457h;
value = value * square + 0.168212891h;
value = value * square + -0.5605469h;
return clamp(float(value * x + 0.5h), 0.0, 1.0);
}
float nativeLightingRingAlpha_material2_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].w <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].y <= 0.0) return 0.0;
float inverseRadius = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].x, 1e-06);
float start = shiftedRingSDF * inverseRadius;
float finish = (shiftedRingSDF + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].y) * inverseRadius;
float ring = clamp(nativeLightingErfc_material2_1_ff(start * 0.707106769) - nativeLightingErfc_material2_1_ff(finish * 0.707106769), 0.0, 1.0);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
return (ring * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].w) * mix(1.0, coverage, clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].z, 0.0, 1.0));
}
float nativeLightingBias_material2_1_fff(float coverage, float amount) {
amount = clamp(amount, 0.0, 1.0);
if (amount <= 0.0 || coverage <= 0.0) return 0.0;
float coefficient = 1.0 / max(amount, 1e-06) - 2.0;
return coverage / max(1.0 + coefficient * (1.0 - coverage), 1e-06);
}
float nativeLightingSDFBand_material2_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal) {
if (height <= 0.0) return 0.0;
float depth = -sdf;
if (depth < -5.0) return 0.0;
float t = clamp(depth / height, 0.0, 1.0);
float shape = mix(float(t < 1.0), 1.0 - t, curvature);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
return (shape * clamp(depth / width + 0.5, 0.0, 1.0)) * clamp((height - depth) / width + 0.5, 0.0, 1.0);
}
float nativeLightingSDFDirection_material2_1_ff2ff(float2 normal, float angle, float spread) {
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(cos(angle - 1.57079637), sin(angle - 1.57079637)));
return clamp((cosine - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
}
float nativeGlassFilterHighlight_material2_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].x <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].z <= 0.0) return 0.0;
float spread = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[2]) <= 1.0 ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].z : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].w;
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(sin(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].y), -cos(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].y)));
float2 angular = clamp((float2(cosine, -cosine) - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
float2 localNormal = (normal * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) / max(localScale, 1.0);
float band;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[2].z > 0.5) {
float width = max((abs(localNormal.x) + abs(localNormal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
float depth = -(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].x);
band = (1.0 - coverage) * clamp(depth / width + 0.5, 0.0, 1.0);
} else {
band = nativeLightingSDFBand_material2_1_fffff2(_globals, sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].z, 0.75, localNormal);
}
return nativeLightingBias_material2_1_fff(band * angular.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].x) + nativeLightingBias_material2_1_fff(band * angular.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlight_1[2].x);
}
half4 nativeGlassHoldingTone_material2_1_h4fh4(thread Globals& _globals, float sdf, half4 material) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].x <= 0.0) return material;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].z;
float progress = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].z) / span, 0.0, 1.0) : 0.0;
float weight = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].x * (1.0 - progress);
float alpha = clamp(float(material.w), 0.0, 1.0);
float3 target = ((float3(material.xyz) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHoldingTone_1[2].y) * alpha) / max(float(material.w), 1e-06);
return mix(material, half4(half3(target), half(alpha)), half(weight));
}
half4 nativeGlassExterior_material2_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop) {
float outputMask = nativeGlassOutputCoverage_material2_1_ff(_globals, sdf);
if (outputMask <= 0.0) return half4(0.0h);
float3 shadowSource = float3(blurredBackdrop.xyz) / max(float(blurredBackdrop.w), 1e-06);
half4 result = half4(0.0h);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].w > 0.0) {
float coverage = nativeLightingErfc_material2_1_ff(shiftedShadowSDF / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].x, 1e-06));
float sourceLuma = dot(shadowSource, float3(0.2126, 0.7152, 0.0722));
float3 linear = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[2].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[2].x) * float3(sourceLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[2].z * (shadowSource - float3(sourceLuma));
float fillAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[2].w, 0.0, 1.0);
float3 bias = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowTransfer_1[2].x) * (1.0 - fillAlpha) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowFill_1[2].xyz * fillAlpha;
float contribution = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].z, 0.0, 1.0);
float3 transformed = (linear * (1.0 - fillAlpha)) * contribution + bias;
float nativeAlpha = mix(fillAlpha, 1.0, contribution);
float opacity = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].w, 0.0, 1.0);
float alpha = (coverage * opacity) * nativeAlpha;
transformed /= max(nativeAlpha, 1e-06);
result = half4(half3(transformed * alpha), half(alpha));
}
return result * half(outputMask);
}
half3 nativeGlassBleed_material2_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop) {
float3 color = float3(body);
if (sdf > 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) return body;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].w > 0.0) {
float luminance = clamp(dot(color, float3(0.2125, 0.7154, 0.0721)), 0.0, 1.0);
float tone = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].w > 0.5 ? luminance : 1.0 - luminance;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[2].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[2].x;
float edge = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[2].x) / span, 0.0, 1.0) : float(sdf <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedDistance_1[2].x);
float gain = (tone * tone) * edge;
gain *= gain;
float3 _248_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _249_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].x;
float _250_luma = dot(_248_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_249_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].y - _249_black) * _250_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].z * (_248_c - float3(_250_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].w, 0.0, 1.0));
}
return half3(color);
}
float2 nativeSampleXY_material2_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target) {
float2 offset = target - origin;
return (origin + offset * min(1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_maximumSampleRadius_1[2] / max(length(offset), 0.0001))) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
float nativeRadiusLOD_material2_1_ff(thread Globals& _globals, float radius) {
float r = (((max(radius, 0.0) * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[2];
float minRadius = (((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeMinimumBlurRadius_1[2] * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativePyramidStretch_1[2];
float base = float(minRadius >= 2.0);
r *= float(base > 0.5 ? 0.5 : 1.0);
return base + max(0.0, log2(r < 2.0 ? 1.0 + r * 0.5 : r));
}
float2 nativePyramidPosition_material2_1_f2f2f(thread Globals& _globals, float2 worldXY, float level) {
float divisor = exp2(level);
float width = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].x;
float height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].y;
float2 tile = float2(1.0);
if (level > 0.5) {
tile.x = width + 3.0;
tile.y = tile.y + (height * (1.0 - 2.0 / divisor) + 2.0 * (level - 1.0));
}
float2 size = float2(width, height) / divisor;
float2 p = (worldXY * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidRect_1[2].xy) / divisor;
return tile + clamp(p, float2(0.5), max(size - 0.5, float2(0.5)));
}
half4 nativePyramidAt_material2_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod) {
lod = clamp(lod, 0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w);
float first = floor(lod);
float second = min(first + 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w);
half4 low = groupBackdrop_material2_1_h4f2(_globals, nativePyramidPosition_material2_1_f2f2f(_globals, worldXY, first));
if (fract(lod) == 0.0 || first == second) return low;
return mix(low, groupBackdrop_material2_1_h4f2(_globals, nativePyramidPosition_material2_1_f2f2f(_globals, worldXY, second)), half(fract(lod)));
}
half4 nativeAtRadius_material2_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
float2 sampleXY = nativeSampleXY_material2_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w < 0.0) return radius > 0.0 ? groupBackdrop_material2_1_h4f2(_globals, sampleXY) : groupSharp_material2_1_h4f2(_globals, sampleXY);
return nativePyramidAt_material2_1_h4f2f(_globals, groupXY_material2_1_f2f2(_globals, sampleXY), nativeRadiusLOD_material2_1_ff(_globals, radius));
}
float3 nativeUnpremul_material2_1_f3h4(half4 p) {
return float3(p.xyz) / max(float(p.w), 1e-06);
}
float nativeLinearRamp_material2_1_ffff(float x, float a, float b) {
return abs(b - a) < 1e-05 ? step(a, x) : clamp((x - a) / (b - a), 0.0, 1.0);
}
float nativeBlurCoverage_material2_1_ff(thread Globals& _globals, float d) {
return ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].x + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].x) * nativeLinearRamp_material2_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].y)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].y) * nativeLinearRamp_material2_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].z)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurOpacities_1[2].z) * nativeLinearRamp_material2_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurDistances_1[2].w);
}
float3 nativeFace_material2_1_f3f3f(thread Globals& _globals, float3 color, float sdf) {
float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
float sdr = 1.0 - step(1.00001, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2]);
float maxLuma = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceLuma_1[2].y, sdr);
float compression = clamp(1.0 + (maxLuma - 1.0) * luma, 0.0, 1.0);
float qLuma = luma * compression;
float3 q = float3(qLuma) + (color * compression - float3(qLuma)) * (1.0 + 0.3 * (1.0 - compression));
float3 result = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[2] + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[2].x - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceBlack_1[2]) * qLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceTransfer_1[2].y * (q - float3(qLuma));
result = mix(result, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[2].xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceFill_1[2].w);
return result;
}
float nativeCubic_material2_1_ffff(float t, float p1, float p2) {
float u = 1.0 - t;
return ((((3.0 * u) * u) * t) * p1 + (((3.0 * u) * t) * t) * p2) + (t * t) * t;
}
float nativeTiming_material2_1_fff4f(float x, float4 control, float known) {
if (x <= 0.0 || x >= 1.0) return clamp(x, 0.0, 1.0);
if (known < 0.5) return x;
float low = 0.0;
float high = 1.0;
float t = x;
for (int i = 0;i < 14; ++i) {
float value = nativeCubic_material2_1_ffff(t, control.x, control.z);
if (value < x) low = t; else high = t;
t = (low + high) * 0.5;
}
return nativeCubic_material2_1_ffff(t, control.y, control.w);
}
float4 nativeGradientColor_material2_1_f4f(thread Globals& _globals, float d) {
float4 a = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent0_1[2] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[2];
float4 b = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent1_1[2] : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccent2_1[2];
float lo = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].x : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y;
float hi = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].z;
float raw = abs(hi - lo) > 1e-05 ? clamp((d - lo) / (hi - lo), 0.0, 1.0) : step(lo, d);
float t = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentDistances_1[2].y ? nativeTiming_material2_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming0_1[2], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[2].x) : nativeTiming_material2_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTiming1_1[2], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentTimingKnown_1[2].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[2].x > 0.5) {
a.xyz = a.xyz * a.w;
b.xyz = b.xyz * b.w;
float4 c = mix(a, b, t);
c.xyz = c.xyz / max(c.w, 1e-06);
return c;
}
return mix(a, b, t);
}
float nativeBevel_material2_1_ffff(float _0_distance, float height, float radius) {
float t = clamp(-_0_distance / max(height, 1e-06), 0.0, 1.0);
return height > 0.0 ? 1.0 - sqrt(max(t * (2.0 - t), 0.0)) : 0.0;
}
half4 nativeGlassBackgroundBody_material2_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius) {
float inside = nativeBevel_material2_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].y, radius);
float outside = nativeBevel_material2_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].w, radius);
float displacement = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].x * inside;
float2 target = xy + normal * displacement;
float effectiveRadius = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].x * max(nativeBlurCoverage_material2_1_ff(_globals, sdf + displacement), 0.0);
nativeAtRadius_material2_1_h4f2f2f(_globals, xy, target, 0.0);
half4 pixel = nativeAtRadius_material2_1_h4f2f2f(_globals, xy, target, effectiveRadius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[2] > 0.0 && abs(displacement) > 0.001) {
float2 shift = ((normal * abs(displacement)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDispersion_1[2]) * 0.3;
half4 red = nativeAtRadius_material2_1_h4f2f2f(_globals, xy, target + shift, effectiveRadius);
half4 blue = nativeAtRadius_material2_1_h4f2f2f(_globals, xy, target - shift, effectiveRadius);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
float3 rgb = nativeUnpremul_material2_1_f3h4(pixel);
float bodyAlpha = 1.0;
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].x > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].y > 0.0) || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].z > 0.0) {
half4 fill;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w < 0.0) {
fill = nativeAtRadius_material2_1_h4f2f2f(_globals, xy, xy, 0.0);
} else {
float lod = nativeRadiusLOD_material2_1_ff(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].y);
float2 _254_xy = nativeSampleXY_material2_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material2_1_f2f2(_globals, _254_xy);
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].z, 1e-06));
fill = (nativePyramidAt_material2_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_material2_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_material2_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillOpacity_1[2].x);
}
float outerMask = nativeLinearRamp_material2_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[2].y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[2].z * outerMask > 0.0) {
float outerShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].z * outside;
half4 outer = nativeAtRadius_material2_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].x * max(nativeBlurCoverage_material2_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[2].z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefractionMask_1[2].z * outerMask);
}
float3 processed = nativeFace_material2_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[2]);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].w > 0.0) {
float2 _255_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].x) * nativeBevel_material2_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].z, radius);
rgb = float3(nativeGlassBleed_material2_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material2_1_h4f2f2f(_globals, xy, _255_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], texture2d<half> image_20_Tex [[texture(2)]], sampler image_20_Smplr [[sampler(2)]], texture2d<half> image_27_Tex [[texture(3)]], sampler image_27_Smplr [[sampler(3)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}, {image_13_Tex, image_13_Smplr}, {image_20_Tex, image_20_Smplr}, {image_27_Tex, image_27_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _538_xy = _in.localCoordsVar / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float _539_rawDistance = mergeGeometry_1_ff2b(_globals, _538_xy, true);
float2 _540_sceneNormal = _globals.mergedNormal_1 / max(length(_globals.mergedNormal_1), 0.0001);
float _541_sceneDistance = _539_rawDistance;
if (_globals.geometryBlended_1) {
float _542_epsilon = max(0.05, 0.25 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
float _543_dx = mergeGeometry_1_ff2b(_globals, _538_xy + float2(_542_epsilon, 0.0), false) - mergeGeometry_1_ff2b(_globals, _538_xy - float2(_542_epsilon, 0.0), false);
float _544_dy = mergeGeometry_1_ff2b(_globals, _538_xy + float2(0.0, _542_epsilon), false) - mergeGeometry_1_ff2b(_globals, _538_xy - float2(0.0, _542_epsilon), false);
float2 _545_gradient = float2(_543_dx, _544_dy) / (2.0 * _542_epsilon);
float _546_magnitude = length(_545_gradient);
if (_546_magnitude > 0.0001) _540_sceneNormal = _545_gradient / _546_magnitude;
_541_sceneDistance = _539_rawDistance / max(_546_magnitude, 0.05);
}
{
float _547_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _547_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _547_cover;
_globals.weights_1[1] *= 1.0 - _547_cover;
_globals.weights_1[2] *= 1.0 - _547_cover;
_globals.weights_1[3] *= 1.0 - _547_cover;
_globals.weights_1[0] += _547_cover;
}
{
float _548_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _548_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _548_cover;
_globals.weights_1[1] *= 1.0 - _548_cover;
_globals.weights_1[2] *= 1.0 - _548_cover;
_globals.weights_1[3] *= 1.0 - _548_cover;
_globals.weights_1[1] += _548_cover;
}
{
float _549_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5) _549_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _549_cover;
_globals.weights_1[1] *= 1.0 - _549_cover;
_globals.weights_1[2] *= 1.0 - _549_cover;
_globals.weights_1[3] *= 1.0 - _549_cover;
_globals.weights_1[2] += _549_cover;
}
{
float _550_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] > 1.5) _550_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _550_cover;
_globals.weights_1[1] *= 1.0 - _550_cover;
_globals.weights_1[2] *= 1.0 - _550_cover;
_globals.weights_1[3] *= 1.0 - _550_cover;
_globals.weights_1[3] += _550_cover;
}
{
float _551_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _551_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _551_cover;
_globals.weights_1[1] *= 1.0 - _551_cover;
_globals.weights_1[2] *= 1.0 - _551_cover;
_globals.weights_1[3] *= 1.0 - _551_cover;
_globals.weights_1[0] += _551_cover;
}
{
float _552_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _552_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _552_cover;
_globals.weights_1[1] *= 1.0 - _552_cover;
_globals.weights_1[2] *= 1.0 - _552_cover;
_globals.weights_1[3] *= 1.0 - _552_cover;
_globals.weights_1[1] += _552_cover;
}
{
float _553_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] <= 1.5) _553_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _553_cover;
_globals.weights_1[1] *= 1.0 - _553_cover;
_globals.weights_1[2] *= 1.0 - _553_cover;
_globals.weights_1[3] *= 1.0 - _553_cover;
_globals.weights_1[2] += _553_cover;
}
{
float _554_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] <= 1.5) _554_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _554_cover;
_globals.weights_1[1] *= 1.0 - _554_cover;
_globals.weights_1[2] *= 1.0 - _554_cover;
_globals.weights_1[3] *= 1.0 - _554_cover;
_globals.weights_1[3] += _554_cover;
}
half4 _555_original = LocalMatrix_2_h4f2(_globals, _in.localCoordsVar);
float _556_coverage = clamp(0.5 - _541_sceneDistance * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
half4 _557_result = half4(0.0h);
float _558_groupInteraction = 0.0;
float2 _559_groupTouch = _538_xy;
{
float _560_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[0] * clamp(0.5 - _globals.distances_1[0] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_560_intensity > _558_groupInteraction) {
_558_groupInteraction = _560_intensity;
float3 _561_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[0], 1.0);
_559_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, _561_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, _561_touch));
}
}
{
float _562_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[1] * clamp(0.5 - _globals.distances_1[1] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_562_intensity > _558_groupInteraction) {
_558_groupInteraction = _562_intensity;
float3 _563_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[1], 1.0);
_559_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, _563_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, _563_touch));
}
}
{
float _564_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[2] * clamp(0.5 - _globals.distances_1[2] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_564_intensity > _558_groupInteraction) {
_558_groupInteraction = _564_intensity;
float3 _565_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[2], 1.0);
_559_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xyz, _565_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xyz, _565_touch));
}
}
{
float _566_intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[3] * clamp(0.5 - _globals.distances_1[3] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (_566_intensity > _558_groupInteraction) {
_558_groupInteraction = _566_intensity;
float3 _567_touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[3], 1.0);
_559_groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[3].xyz, _567_touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[3].xyz, _567_touch));
}
}
if (_globals.weights_1[0] > 0.0) {
float2 _568_localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(_538_xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(_538_xy, 1.0)));
float2 _569_localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].x * _540_sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y * _540_sceneNormal.y);
float _570_localMetric = max(length(_569_localGradient), 0.0001);
_globals.materialNormal_1 = _569_localGradient / _570_localMetric;
_globals.interactionIntensity_1 = _558_groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(_559_groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(_559_groupTouch, 1.0)));
float _571_localDistance = _541_sceneDistance / _570_localMetric;
if (nativeGlassOutputCoverage_material0_1_ff(_globals, _571_localDistance) > 0.0 || _556_coverage > 0.0) {
float _572_ringDistance = _571_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[0].y > 0.0) {
float2 _573_ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[0];
_572_ringDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _573_ringOffset, false) / _570_localMetric;
}
float _574_shadowDistance = _571_localDistance;
if (_556_coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].w > 0.0)) {
float2 _575_shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[0]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[0]));
_574_shadowDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _575_shadowOffset, false) / _570_localMetric;
}
float _576_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[0], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 0.5);
half4 _577_field;
if (_556_coverage >= 1.0) _577_field = nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, _568_localXY, _571_localDistance, _globals.materialNormal_1, _576_radius); else {
float _578_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].x * nativeBevel_material0_1_ffff(_571_localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].z, _576_radius);
half4 _579_shadow = nativeAtRadius_material0_1_h4f2f2f(_globals, _568_localXY, _568_localXY + _globals.materialNormal_1 * _578_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].w);
_577_field = nativeGlassExterior_material0_1_h4ffff2h4h4h4(_globals, _571_localDistance, _574_shadowDistance, _572_ringDistance, _globals.materialNormal_1, _555_original, _579_shadow, _555_original);
if (_556_coverage > 0.0) _577_field = mix(_577_field, nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, _568_localXY, _571_localDistance, _globals.materialNormal_1, _576_radius), half(_556_coverage));
}
float _580_alpha = clamp(nativeLightingRingAlpha_material0_1_ffff2(_globals, _571_localDistance, _572_ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _581_foreground = half4(0.0h, 0.0h, 0.0h, half(_580_alpha));
_577_field = _581_foreground + _577_field * (1.0h - _581_foreground.w);
half4 _582_material = _577_field;
half3 _583_exteriorFace = half3(nativeFace_material0_1_f3f3f(_globals, nativeUnpremul_material0_1_f3h4(_555_original), _571_localDistance));
float _584_amount = nativeGlassFilterHighlight_material0_1_fff2f(_globals, _571_localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * _570_localMetric);
if (_584_amount > 1e-06) {
float3 _585_raw = float3(_555_original.xyz) / max(float(_555_original.w), 1e-06);
float3 _586_face = float3(_583_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].x > 0.5) _586_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y < 0.0 ? min(_586_face, _585_raw) : max(_586_face, _585_raw);
float _587_alpha = float(_582_material.w);
float3 _588_color = float3(_582_material.xyz) + (1.0 - _587_alpha) * mix(_585_raw, _586_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[0] * _584_amount);
_588_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y * _584_amount) * (3.0 - 2.0 * _588_color);
float _589_resultAlpha = _587_alpha + ((1.0 - _587_alpha) * _584_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].y;
_582_material = half4(half3(_588_color - (1.0 - _589_resultAlpha) * _585_raw), half(_589_resultAlpha));
}
_577_field = nativeGlassHoldingTone_material0_1_h4fh4(_globals, _571_localDistance, _582_material);
float3 _590_fieldColor = nativeLightingClampColor_material0_1_f3f3ff(nativeUnpremul_material0_1_f3h4(_577_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
_577_field = half4(half3(_590_fieldColor) * _577_field.w, _577_field.w);
half4 _591_composited = _577_field + _555_original * (1.0h - _577_field.w);
float3 _592_clamped = clamp(nativeUnpremul_material0_1_f3h4(_591_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0])));
_591_composited = half4(half3(_592_clamped) * _591_composited.w, _591_composited.w);
float3 _593_rgb = nativeUnpremul_material0_1_f3h4(_591_composited);
float _594_materialAlpha = float(_591_composited.w);
float _595_inside = nativeBevel_material0_1_ffff(_571_localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].y, _576_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[0] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w > 0.0) {
float4 _596_gradient = nativeGradientColor_material0_1_f4f(_globals, _571_localDistance);
float4 _597_matrixInput = float4(_593_rgb, float(_591_composited.w));
float4 _598_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[0], _597_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[0], _597_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[0], _597_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[0], _597_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[0];
_598_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_598_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].y);
float4 _599_accent = _598_result;
_599_accent.w = clamp(_599_accent.w, 0.0, 1.0);
_599_accent.xyz = _599_accent.xyz * _599_accent.w;
float _600_mask = _596_gradient.w * _556_coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[0].y > 0.5) _600_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w; else {
_599_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _599_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
_599_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _599_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
}
_599_accent *= clamp(_600_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w, 0.0, 1.0);
float _601_outAlpha = _599_accent.w + _594_materialAlpha * (1.0 - _599_accent.w);
_593_rgb = (_599_accent.xyz + (_593_rgb * _594_materialAlpha) * (1.0 - _599_accent.w)) / max(_601_outAlpha, 1e-06);
_594_materialAlpha = _601_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] >= 0.0) {
_593_rgb = mix(_593_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w) * _556_coverage, 0.0, 1.0));
}
_593_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[0] * _595_inside;
_593_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[0] * smoothstep(0.85, 0.98, dot(_593_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _602_unlit = _593_rgb;
float3 _603_color = float3(half3(_593_rgb));
float2 _604_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? _540_sceneNormal : _globals.materialNormal_1;
float _605_norm = length(_604_normal);
if (_605_norm > 1e-05) _604_normal /= _605_norm;
float _606_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].y);
float _607_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].y);
float _608_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].w;
float _609_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].w;
float _610_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].x;
float _611_key = 0.0;
float _612_fill = 0.0;
float _613_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? _571_localDistance / max(_605_norm, 1e-06) : _571_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z > 0.0) {
float _614_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _613_localSDF, _606_keyHeight, _610_curvature, _604_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_604_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _608_keySpread);
float _615_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _613_localSDF, _606_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _604_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_604_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _608_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_611_key = nativeLightingBias_material0_1_fff(_614_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_615_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z > 0.0) {
float _616_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _613_localSDF, _607_fillHeight, _610_curvature, _604_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_604_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _609_fillSpread);
float _617_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _613_localSDF, _607_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _604_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_604_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _609_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_612_fill = nativeLightingBias_material0_1_fff(_616_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_617_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
float3 _618_unlitColor = _603_color;
float4 _619_c = float4(_618_unlitColor, 1.0);
float4 _620_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[0], _619_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[0], _619_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[0], _619_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[0], _619_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[0];
_620_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_620_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].y);
float4 _621_transformed = _620_result;
float _622_coverage = _611_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[0].w + _612_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[0].w;
_622_coverage *= clamp(_621_transformed.w, 0.0, 1.0);
_603_color = _618_unlitColor + _622_coverage * (_621_transformed.xyz - _618_unlitColor);
_593_rgb = float3(half3(nativeLightingClampColor_material0_1_f3f3ff(_603_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[0])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y)));
float _623_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[0], clamp(dot(_602_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_593_rgb = mix(_602_unlit, _593_rgb, _623_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0] > 0.0 && _595_inside > 0.0) {
float2 _624_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[0];
half4 _625_reflected = (nativeAtRadius_material0_1_h4f2f2f(_globals, _568_localXY, (_568_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) + _624_tangent, 0.0) + nativeAtRadius_material0_1_h4f2f2f(_globals, _568_localXY, (_568_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) - _624_tangent, 0.0)) * 0.5h;
_593_rgb = mix(_593_rgb, nativeUnpremul_material0_1_f3h4(_625_reflected), (_595_inside * _595_inside) * 0.1);
}
float _626_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * _556_coverage;
if (_626_glow > 0.0) {
float _627_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 1.6);
float _628_sheen = 1.0 - smoothstep(0.0, _627_glowRadius, length(_568_localXY - _globals.interactionLocation_1));
float _629_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float3 _630_restingResult = _593_rgb;
_593_rgb = mix(_593_rgb, float3(1.0), _626_glow * _629_ambientPressOpacity);
_593_rgb = mix(_593_rgb, float3(1.0), (_626_glow * _628_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float _631_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y));
float _632_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y);
float _633_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] < 0.5) * smoothstep(160.0, 280.0, _632_longSide)) * smoothstep(2.5, 4.0, _632_longSide / _631_shortSide);
float _634_peak = max(_630_restingResult.x, max(_630_restingResult.y, _630_restingResult.z));
float _635_chromaticRange = _634_peak - min(_630_restingResult.x, min(_630_restingResult.y, _630_restingResult.z));
float _636_neutralPress = 1.0 - smoothstep(0.01, 0.1, _635_chromaticRange);
float _637_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]);
float _638_pressGain = mix(1.18 - 0.06 * _634_peak, 1.06, _637_lightAppearance);
float3 _639_colorPress = mix(float3(float(dot(half3(_630_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _630_restingResult, mix(1.03, 1.06, _637_lightAppearance)) * _638_pressGain;
_639_colorPress += float3(_636_neutralPress * mix(0.02745098, 0.04509804, _637_lightAppearance));
_593_rgb = mix(_593_rgb, mix(_630_restingResult, _639_colorPress, _626_glow), _633_platterPress);
}
_593_rgb = nativeLightingClampColor_material0_1_f3f3ff(_593_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
float _640_alpha = clamp(_594_materialAlpha, 0.0, 1.0);
half4 _641_color = half4(half3(_593_rgb * _640_alpha), half(_640_alpha));
_557_result += half(_globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0]) * _641_color;
} else _557_result += half(_globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0]) * _555_original;
}
if (_globals.weights_1[1] > 0.0) {
float2 _642_localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(_538_xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(_538_xy, 1.0)));
float2 _643_localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].x * _540_sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y * _540_sceneNormal.y);
float _644_localMetric = max(length(_643_localGradient), 0.0001);
_globals.materialNormal_1 = _643_localGradient / _644_localMetric;
_globals.interactionIntensity_1 = _558_groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(_559_groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(_559_groupTouch, 1.0)));
float _645_localDistance = _541_sceneDistance / _644_localMetric;
if (nativeGlassOutputCoverage_material1_1_ff(_globals, _645_localDistance) > 0.0 || _556_coverage > 0.0) {
float _646_ringDistance = _645_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[1].y > 0.0) {
float2 _647_ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[1];
_646_ringDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _647_ringOffset, false) / _644_localMetric;
}
float _648_shadowDistance = _645_localDistance;
if (_556_coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].w > 0.0)) {
float2 _649_shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[1]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[1]));
_648_shadowDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _649_shadowOffset, false) / _644_localMetric;
}
float _650_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[1], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 0.5);
half4 _651_field;
if (_556_coverage >= 1.0) _651_field = nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, _642_localXY, _645_localDistance, _globals.materialNormal_1, _650_radius); else {
float _652_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].x * nativeBevel_material1_1_ffff(_645_localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].z, _650_radius);
half4 _653_shadow = nativeAtRadius_material1_1_h4f2f2f(_globals, _642_localXY, _642_localXY + _globals.materialNormal_1 * _652_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].w);
_651_field = nativeGlassExterior_material1_1_h4ffff2h4h4h4(_globals, _645_localDistance, _648_shadowDistance, _646_ringDistance, _globals.materialNormal_1, _555_original, _653_shadow, _555_original);
if (_556_coverage > 0.0) _651_field = mix(_651_field, nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, _642_localXY, _645_localDistance, _globals.materialNormal_1, _650_radius), half(_556_coverage));
}
float _654_alpha = clamp(nativeLightingRingAlpha_material1_1_ffff2(_globals, _645_localDistance, _646_ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _655_foreground = half4(0.0h, 0.0h, 0.0h, half(_654_alpha));
_651_field = _655_foreground + _651_field * (1.0h - _655_foreground.w);
half4 _656_material = _651_field;
half3 _657_exteriorFace = half3(nativeFace_material1_1_f3f3f(_globals, nativeUnpremul_material1_1_f3h4(_555_original), _645_localDistance));
float _658_amount = nativeGlassFilterHighlight_material1_1_fff2f(_globals, _645_localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * _644_localMetric);
if (_658_amount > 1e-06) {
float3 _659_raw = float3(_555_original.xyz) / max(float(_555_original.w), 1e-06);
float3 _660_face = float3(_657_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].x > 0.5) _660_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y < 0.0 ? min(_660_face, _659_raw) : max(_660_face, _659_raw);
float _661_alpha = float(_656_material.w);
float3 _662_color = float3(_656_material.xyz) + (1.0 - _661_alpha) * mix(_659_raw, _660_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[1] * _658_amount);
_662_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y * _658_amount) * (3.0 - 2.0 * _662_color);
float _663_resultAlpha = _661_alpha + ((1.0 - _661_alpha) * _658_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].y;
_656_material = half4(half3(_662_color - (1.0 - _663_resultAlpha) * _659_raw), half(_663_resultAlpha));
}
_651_field = nativeGlassHoldingTone_material1_1_h4fh4(_globals, _645_localDistance, _656_material);
float3 _664_fieldColor = nativeLightingClampColor_material1_1_f3f3ff(nativeUnpremul_material1_1_f3h4(_651_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
_651_field = half4(half3(_664_fieldColor) * _651_field.w, _651_field.w);
half4 _665_composited = _651_field + _555_original * (1.0h - _651_field.w);
float3 _666_clamped = clamp(nativeUnpremul_material1_1_f3h4(_665_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1])));
_665_composited = half4(half3(_666_clamped) * _665_composited.w, _665_composited.w);
float3 _667_rgb = nativeUnpremul_material1_1_f3h4(_665_composited);
float _668_materialAlpha = float(_665_composited.w);
float _669_inside = nativeBevel_material1_1_ffff(_645_localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].y, _650_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[1] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w > 0.0) {
float4 _670_gradient = nativeGradientColor_material1_1_f4f(_globals, _645_localDistance);
float4 _671_matrixInput = float4(_667_rgb, float(_665_composited.w));
float4 _672_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[1], _671_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[1], _671_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[1], _671_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[1], _671_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[1];
_672_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_672_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].y);
float4 _673_accent = _672_result;
_673_accent.w = clamp(_673_accent.w, 0.0, 1.0);
_673_accent.xyz = _673_accent.xyz * _673_accent.w;
float _674_mask = _670_gradient.w * _556_coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[1].y > 0.5) _674_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w; else {
_673_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _673_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
_673_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _673_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
}
_673_accent *= clamp(_674_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w, 0.0, 1.0);
float _675_outAlpha = _673_accent.w + _668_materialAlpha * (1.0 - _673_accent.w);
_667_rgb = (_673_accent.xyz + (_667_rgb * _668_materialAlpha) * (1.0 - _673_accent.w)) / max(_675_outAlpha, 1e-06);
_668_materialAlpha = _675_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] >= 0.0) {
_667_rgb = mix(_667_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w) * _556_coverage, 0.0, 1.0));
}
_667_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[1] * _669_inside;
_667_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[1] * smoothstep(0.85, 0.98, dot(_667_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _676_unlit = _667_rgb;
float3 _677_color = float3(half3(_667_rgb));
float2 _678_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? _540_sceneNormal : _globals.materialNormal_1;
float _679_norm = length(_678_normal);
if (_679_norm > 1e-05) _678_normal /= _679_norm;
float _680_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].y);
float _681_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].y);
float _682_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].w;
float _683_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].w;
float _684_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].x;
float _685_key = 0.0;
float _686_fill = 0.0;
float _687_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? _645_localDistance / max(_679_norm, 1e-06) : _645_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z > 0.0) {
float _688_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _687_localSDF, _680_keyHeight, _684_curvature, _678_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_678_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _682_keySpread);
float _689_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _687_localSDF, _680_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _678_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_678_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _682_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_685_key = nativeLightingBias_material1_1_fff(_688_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_689_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z > 0.0) {
float _690_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _687_localSDF, _681_fillHeight, _684_curvature, _678_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_678_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _683_fillSpread);
float _691_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _687_localSDF, _681_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _678_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_678_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _683_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_686_fill = nativeLightingBias_material1_1_fff(_690_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_691_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
float3 _692_unlitColor = _677_color;
float4 _693_c = float4(_692_unlitColor, 1.0);
float4 _694_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[1], _693_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[1], _693_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[1], _693_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[1], _693_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[1];
_694_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_694_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].y);
float4 _695_transformed = _694_result;
float _696_coverage = _685_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[1].w + _686_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[1].w;
_696_coverage *= clamp(_695_transformed.w, 0.0, 1.0);
_677_color = _692_unlitColor + _696_coverage * (_695_transformed.xyz - _692_unlitColor);
_667_rgb = float3(half3(nativeLightingClampColor_material1_1_f3f3ff(_677_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[1])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y)));
float _697_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[1], clamp(dot(_676_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_667_rgb = mix(_676_unlit, _667_rgb, _697_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1] > 0.0 && _669_inside > 0.0) {
float2 _698_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[1];
half4 _699_reflected = (nativeAtRadius_material1_1_h4f2f2f(_globals, _642_localXY, (_642_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) + _698_tangent, 0.0) + nativeAtRadius_material1_1_h4f2f2f(_globals, _642_localXY, (_642_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) - _698_tangent, 0.0)) * 0.5h;
_667_rgb = mix(_667_rgb, nativeUnpremul_material1_1_f3h4(_699_reflected), (_669_inside * _669_inside) * 0.1);
}
float _700_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * _556_coverage;
if (_700_glow > 0.0) {
float _701_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 1.6);
float _702_sheen = 1.0 - smoothstep(0.0, _701_glowRadius, length(_642_localXY - _globals.interactionLocation_1));
float _703_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float3 _704_restingResult = _667_rgb;
_667_rgb = mix(_667_rgb, float3(1.0), _700_glow * _703_ambientPressOpacity);
_667_rgb = mix(_667_rgb, float3(1.0), (_700_glow * _702_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float _705_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y));
float _706_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y);
float _707_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] < 0.5) * smoothstep(160.0, 280.0, _706_longSide)) * smoothstep(2.5, 4.0, _706_longSide / _705_shortSide);
float _708_peak = max(_704_restingResult.x, max(_704_restingResult.y, _704_restingResult.z));
float _709_chromaticRange = _708_peak - min(_704_restingResult.x, min(_704_restingResult.y, _704_restingResult.z));
float _710_neutralPress = 1.0 - smoothstep(0.01, 0.1, _709_chromaticRange);
float _711_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]);
float _712_pressGain = mix(1.18 - 0.06 * _708_peak, 1.06, _711_lightAppearance);
float3 _713_colorPress = mix(float3(float(dot(half3(_704_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _704_restingResult, mix(1.03, 1.06, _711_lightAppearance)) * _712_pressGain;
_713_colorPress += float3(_710_neutralPress * mix(0.02745098, 0.04509804, _711_lightAppearance));
_667_rgb = mix(_667_rgb, mix(_704_restingResult, _713_colorPress, _700_glow), _707_platterPress);
}
_667_rgb = nativeLightingClampColor_material1_1_f3f3ff(_667_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
float _714_alpha = clamp(_668_materialAlpha, 0.0, 1.0);
half4 _715_color = half4(half3(_667_rgb * _714_alpha), half(_714_alpha));
_557_result += half(_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * _715_color;
} else _557_result += half(_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * _555_original;
}
if (_globals.weights_1[2] > 0.0) {
float2 _716_localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(_538_xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(_538_xy, 1.0)));
float2 _717_localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].x * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].x * _540_sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].y * _540_sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].y * _540_sceneNormal.y);
float _718_localMetric = max(length(_717_localGradient), 0.0001);
_globals.materialNormal_1 = _717_localGradient / _718_localMetric;
_globals.interactionIntensity_1 = _558_groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(_559_groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(_559_groupTouch, 1.0)));
float _719_localDistance = _541_sceneDistance / _718_localMetric;
if (nativeGlassOutputCoverage_material2_1_ff(_globals, _719_localDistance) > 0.0 || _556_coverage > 0.0) {
float _720_ringDistance = _719_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].y > 0.0) {
float2 _721_ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[2];
_720_ringDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _721_ringOffset, false) / _718_localMetric;
}
float _722_shadowDistance = _719_localDistance;
if (_556_coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].w > 0.0)) {
float2 _723_shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[2]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[2]));
_722_shadowDistance = mergeGeometry_1_ff2b(_globals, _538_xy - _723_shadowOffset, false) / _718_localMetric;
}
float _724_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[2], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y) * 0.5);
half4 _725_field;
if (_556_coverage >= 1.0) _725_field = nativeGlassBackgroundBody_material2_1_h4f2ff2f(_globals, _716_localXY, _719_localDistance, _globals.materialNormal_1, _724_radius); else {
float _726_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].x * nativeBevel_material2_1_ffff(_719_localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].z, _724_radius);
half4 _727_shadow = nativeAtRadius_material2_1_h4f2f2f(_globals, _716_localXY, _716_localXY + _globals.materialNormal_1 * _726_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].w);
_725_field = nativeGlassExterior_material2_1_h4ffff2h4h4h4(_globals, _719_localDistance, _722_shadowDistance, _720_ringDistance, _globals.materialNormal_1, _555_original, _727_shadow, _555_original);
if (_556_coverage > 0.0) _725_field = mix(_725_field, nativeGlassBackgroundBody_material2_1_h4f2ff2f(_globals, _716_localXY, _719_localDistance, _globals.materialNormal_1, _724_radius), half(_556_coverage));
}
float _728_alpha = clamp(nativeLightingRingAlpha_material2_1_ffff2(_globals, _719_localDistance, _720_ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _729_foreground = half4(0.0h, 0.0h, 0.0h, half(_728_alpha));
_725_field = _729_foreground + _725_field * (1.0h - _729_foreground.w);
half4 _730_material = _725_field;
half3 _731_exteriorFace = half3(nativeFace_material2_1_f3f3f(_globals, nativeUnpremul_material2_1_f3h4(_555_original), _719_localDistance));
float _732_amount = nativeGlassFilterHighlight_material2_1_fff2f(_globals, _719_localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * _718_localMetric);
if (_732_amount > 1e-06) {
float3 _733_raw = float3(_555_original.xyz) / max(float(_555_original.w), 1e-06);
float3 _734_face = float3(_731_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[2].x > 0.5) _734_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].y < 0.0 ? min(_734_face, _733_raw) : max(_734_face, _733_raw);
float _735_alpha = float(_730_material.w);
float3 _736_color = float3(_730_material.xyz) + (1.0 - _735_alpha) * mix(_733_raw, _734_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[2] * _732_amount);
_736_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].y * _732_amount) * (3.0 - 2.0 * _736_color);
float _737_resultAlpha = _735_alpha + ((1.0 - _735_alpha) * _732_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[2].y;
_730_material = half4(half3(_736_color - (1.0 - _737_resultAlpha) * _733_raw), half(_737_resultAlpha));
}
_725_field = nativeGlassHoldingTone_material2_1_h4fh4(_globals, _719_localDistance, _730_material);
float3 _738_fieldColor = nativeLightingClampColor_material2_1_f3f3ff(nativeUnpremul_material2_1_f3h4(_725_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y);
_725_field = half4(half3(_738_fieldColor) * _725_field.w, _725_field.w);
half4 _739_composited = _725_field + _555_original * (1.0h - _725_field.w);
float3 _740_clamped = clamp(nativeUnpremul_material2_1_f3h4(_739_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2])));
_739_composited = half4(half3(_740_clamped) * _739_composited.w, _739_composited.w);
float3 _741_rgb = nativeUnpremul_material2_1_f3h4(_739_composited);
float _742_materialAlpha = float(_739_composited.w);
float _743_inside = nativeBevel_material2_1_ffff(_719_localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].y, _724_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[2] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w > 0.0) {
float4 _744_gradient = nativeGradientColor_material2_1_f4f(_globals, _719_localDistance);
float4 _745_matrixInput = float4(_741_rgb, float(_739_composited.w));
float4 _746_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[2], _745_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[2], _745_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[2], _745_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[2], _745_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[2];
_746_result.xyz = nativeLightingClampColor_material2_1_f3f3ff(_746_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[2].y);
float4 _747_accent = _746_result;
_747_accent.w = clamp(_747_accent.w, 0.0, 1.0);
_747_accent.xyz = _747_accent.xyz * _747_accent.w;
float _748_mask = _744_gradient.w * _556_coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[2].y > 0.5) _748_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w; else {
_747_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w + _747_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w);
_747_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w + _747_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w);
}
_747_accent *= clamp(_748_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w, 0.0, 1.0);
float _749_outAlpha = _747_accent.w + _742_materialAlpha * (1.0 - _747_accent.w);
_741_rgb = (_747_accent.xyz + (_741_rgb * _742_materialAlpha) * (1.0 - _747_accent.w)) / max(_749_outAlpha, 1e-06);
_742_materialAlpha = _749_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[2] >= 0.0) {
_741_rgb = mix(_741_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w) * _556_coverage, 0.0, 1.0));
}
_741_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[2] * _743_inside;
_741_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[2] * smoothstep(0.85, 0.98, dot(_741_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _750_unlit = _741_rgb;
float3 _751_color = float3(half3(_741_rgb));
float2 _752_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].w > 0.5 ? _540_sceneNormal : _globals.materialNormal_1;
float _753_norm = length(_752_normal);
if (_753_norm > 1e-05) _752_normal /= _753_norm;
float _754_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].y);
float _755_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].y);
float _756_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].w;
float _757_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].w;
float _758_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].x;
float _759_key = 0.0;
float _760_fill = 0.0;
float _761_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].w > 0.5 ? _719_localDistance / max(_753_norm, 1e-06) : _719_localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z > 0.0) {
float _762_primary = nativeLightingSDFBand_material2_1_fffff2(_globals, _761_localSDF, _754_keyHeight, _758_curvature, _752_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_752_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].x, _756_keySpread);
float _763_diffuse = nativeLightingSDFBand_material2_1_fffff2(_globals, _761_localSDF, _754_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].w, 1.0, _752_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_752_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].x, _756_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].z);
_759_key = nativeLightingBias_material2_1_fff(_762_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z) + nativeLightingBias_material2_1_fff(_763_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z > 0.0) {
float _764_primary = nativeLightingSDFBand_material2_1_fffff2(_globals, _761_localSDF, _755_fillHeight, _758_curvature, _752_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_752_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].x, _757_fillSpread);
float _765_diffuse = nativeLightingSDFBand_material2_1_fffff2(_globals, _761_localSDF, _755_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].w, 1.0, _752_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_752_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].x, _757_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].z);
_760_fill = nativeLightingBias_material2_1_fff(_764_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z) + nativeLightingBias_material2_1_fff(_765_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].y);
}
float3 _766_unlitColor = _751_color;
float4 _767_c = float4(_766_unlitColor, 1.0);
float4 _768_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[2], _767_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[2], _767_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[2], _767_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[2], _767_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[2];
_768_result.xyz = nativeLightingClampColor_material2_1_f3f3ff(_768_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].y);
float4 _769_transformed = _768_result;
float _770_coverage = _759_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[2].w + _760_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[2].w;
_770_coverage *= clamp(_769_transformed.w, 0.0, 1.0);
_751_color = _766_unlitColor + _770_coverage * (_769_transformed.xyz - _766_unlitColor);
_741_rgb = float3(half3(nativeLightingClampColor_material2_1_f3f3ff(_751_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[2])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y)));
float _771_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[2], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[2], clamp(dot(_750_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_741_rgb = mix(_750_unlit, _741_rgb, _771_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2] > 0.0 && _743_inside > 0.0) {
float2 _772_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[2];
half4 _773_reflected = (nativeAtRadius_material2_1_h4f2f2f(_globals, _716_localXY, (_716_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2]) + _772_tangent, 0.0) + nativeAtRadius_material2_1_h4f2f2f(_globals, _716_localXY, (_716_localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2]) - _772_tangent, 0.0)) * 0.5h;
_741_rgb = mix(_741_rgb, nativeUnpremul_material2_1_f3h4(_773_reflected), (_743_inside * _743_inside) * 0.1);
}
float _774_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * _556_coverage;
if (_774_glow > 0.0) {
float _775_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y) * 1.6);
float _776_sheen = 1.0 - smoothstep(0.0, _775_glowRadius, length(_716_localXY - _globals.interactionLocation_1));
float _777_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[2] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]));
float3 _778_restingResult = _741_rgb;
_741_rgb = mix(_741_rgb, float3(1.0), _774_glow * _777_ambientPressOpacity);
_741_rgb = mix(_741_rgb, float3(1.0), (_774_glow * _776_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]));
float _779_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y));
float _780_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y);
float _781_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[2] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] < 0.5) * smoothstep(160.0, 280.0, _780_longSide)) * smoothstep(2.5, 4.0, _780_longSide / _779_shortSide);
float _782_peak = max(_778_restingResult.x, max(_778_restingResult.y, _778_restingResult.z));
float _783_chromaticRange = _782_peak - min(_778_restingResult.x, min(_778_restingResult.y, _778_restingResult.z));
float _784_neutralPress = 1.0 - smoothstep(0.01, 0.1, _783_chromaticRange);
float _785_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]);
float _786_pressGain = mix(1.18 - 0.06 * _782_peak, 1.06, _785_lightAppearance);
float3 _787_colorPress = mix(float3(float(dot(half3(_778_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _778_restingResult, mix(1.03, 1.06, _785_lightAppearance)) * _786_pressGain;
_787_colorPress += float3(_784_neutralPress * mix(0.02745098, 0.04509804, _785_lightAppearance));
_741_rgb = mix(_741_rgb, mix(_778_restingResult, _787_colorPress, _774_glow), _781_platterPress);
}
_741_rgb = nativeLightingClampColor_material2_1_f3f3ff(_741_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y);
float _788_alpha = clamp(_742_materialAlpha, 0.0, 1.0);
half4 _789_color = half4(half3(_741_rgb * _788_alpha), half(_788_alpha));
_557_result += half(_globals.weights_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[2]) * _789_color;
} else _557_result += half(_globals.weights_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[2]) * _555_original;
}
half4 outColor_1 = _557_result;
_out.sk_FragColor = outColor_1;
return _out;
}
