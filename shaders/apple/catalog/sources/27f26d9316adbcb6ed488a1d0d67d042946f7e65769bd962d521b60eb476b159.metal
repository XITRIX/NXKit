#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<float2, 3> material_resolution_1;
array<float2, 3> material_lensCenter_1;
array<float2, 3> material_lensSize_1;
array<float, 3> material_cornerRadius_1;
array<float, 3> material_cornerCurve_1;
float externalShapeCoverage_1;
array<float, 3> material_maximumSampleRadius_1;
float contentScale_1;
float filterScale_1;
array<float, 3> material_interactionIntensity_1;
array<float2, 3> material_interactionLocation_1;
array<float, 3> material_bodyMode_1;
array<float, 3> material_tintMode_1;
array<float, 3> material_referenceAppearance_1;
array<float, 3> material_backdropLuma_1;
array<float4, 3> material_tint_1;
array<float, 3> material_nativeFaceBlack_1;
array<float3, 3> material_nativeFaceTransfer_1;
array<float4, 3> material_nativeFaceFill_1;
array<float2, 3> material_nativeFaceLuma_1;
array<float, 3> material_nativeFaceOpacity_1;
array<float3, 3> material_nativeFillOpacity_1;
array<float4, 3> material_nativeBlurDistances_1;
array<float4, 3> material_nativeBlurOpacities_1;
array<float4, 3> material_nativeRefraction_1;
array<float3, 3> material_nativeRefractionMask_1;
array<float2, 3> material_nativeSDRGradient_1;
array<float, 3> material_nativeDisplayHeadroom_1;
array<float, 3> material_nativeAccentEnabled_1;
array<float4, 3> material_nativeAccent0_1;
array<float4, 3> material_nativeAccent1_1;
array<float4, 3> material_nativeAccent2_1;
array<float3, 3> material_nativeAccentDistances_1;
array<float4, 3> material_nativeAccentTiming0_1;
array<float4, 3> material_nativeAccentTiming1_1;
array<float2, 3> material_nativeAccentTimingKnown_1;
array<float2, 3> material_nativeAccentMaskOptions_1;
array<float4, 3> material_nativeAccentFill_1;
array<float4, 3> material_nativeAccentMatrix0_1;
array<float4, 3> material_nativeAccentMatrix1_1;
array<float4, 3> material_nativeAccentMatrix2_1;
array<float4, 3> material_nativeAccentMatrix3_1;
array<float4, 3> material_nativeAccentMatrixBias_1;
array<float3, 3> material_nativeAccentMatrixOptions_1;
array<float, 3> material_nativeDispersion_1;
array<float, 3> material_nativeEdgeAbsorption_1;
array<float, 3> material_nativeTintOpacity_1;
array<float, 3> material_nativeBrightBackdropShade_1;
array<float, 3> material_nativeDarkSpecularGain_1;
array<float, 3> material_nativeBrightSpecularGain_1;
array<float, 3> material_nativeReflectionDistance_1;
array<float, 3> material_nativeReflectionSpread_1;
array<float4, 3> material_backdropPyramidRect_1;
array<float4, 3> material_backdropPyramidInfo_1;
array<float, 3> material_nativePyramidStretch_1;
array<float, 3> material_nativeMinimumBlurRadius_1;
array<float4, 3> material_nativeBlurRadii_1;
array<float4, 3> inverseX_1;
array<float4, 3> inverseY_1;
array<float4, 3> forwardX_1;
array<float4, 3> forwardY_1;
array<float4, 3> geometry_1;
array<float4, 3> outputBounds_1;
array<float, 3> itemAlpha_1;
float mergeSpacing_1;
array<float4, 3> material_cornerTop_1;
array<float4, 3> material_cornerTopCircle_1;
array<float4, 3> material_cornerRight_1;
array<float4, 3> material_cornerRightCircle_1;
array<float2, 3> material_nativeOutputDistance_1;
array<float4, 3> material_nativeShadowShape_1;
array<float2, 3> material_nativeShadowOffset_1;
array<float4, 3> material_nativeShadowTransfer_1;
array<float4, 3> material_nativeShadowFill_1;
array<float4, 3> material_nativeShadowResponse_1;
array<float4, 3> material_nativeRingShadow_1;
array<float, 3> material_nativeRingShadowOffset_1;
array<float4, 3> material_nativeBleedShape_1;
array<float4, 3> material_nativeBleedTransfer_1;
array<float2, 3> material_nativeBleedDistance_1;
array<float4, 3> material_nativeFilterHighlight_1;
array<float3, 3> material_nativeFilterHighlightColor_1;
array<float4, 3> material_nativeKeyHighlight_1;
array<float4, 3> material_nativeFillHighlight_1;
array<float4, 3> material_nativeKeyHighlightShape_1;
array<float4, 3> material_nativeFillHighlightShape_1;
array<float4, 3> material_nativeKeyHighlightColor_1;
array<float4, 3> material_nativeFillHighlightColor_1;
array<float4, 3> material_nativeHighlightResponse_1;
array<float4, 3> material_nativeHighlightMatrix0_1;
array<float4, 3> material_nativeHighlightMatrix1_1;
array<float4, 3> material_nativeHighlightMatrix2_1;
array<float4, 3> material_nativeHighlightMatrix3_1;
array<float4, 3> material_nativeHighlightMatrixBias_1;
array<float4, 3> material_nativeHighlightMatrixOptions_1;
array<float3, 3> material_nativeLightingClamp_1;
array<float, 3> material_nativeLightingDisplayHeadroom_1;
array<float4, 3> material_nativeHoldingTone_1;
array<float3, 3> material_nativeFilterOptions_1;
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
    array<float, 3> weights_1;
    array<float, 3> distances_1;
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
float4 _32_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _33_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _32_g.xy * 0.5;
_globals.cornerCurve_1 = _32_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _34_r = min(_32_g.z, min(_32_g.x, _32_g.y) * 0.5);
float3 _35_surface = roundedBoxSample_1_f3f2f2f(_globals, _33_p, _32_g.xy * 0.5, _34_r);
float _36_d = _35_surface.x;
float2 _37_n = _35_surface.yz;
float2 _38_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _37_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _37_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _37_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _37_n.y);
float _39_magnitude = max(length(_38_gradient), 0.0001);
float3 first = float3(_36_d / _39_magnitude, _38_gradient / _39_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _40_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _41_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _40_g.xy * 0.5;
_globals.cornerCurve_1 = _40_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _42_r = min(_40_g.z, min(_40_g.x, _40_g.y) * 0.5);
float3 _43_surface = roundedBoxSample_1_f3f2f2f(_globals, _41_p, _40_g.xy * 0.5, _42_r);
float _44_d = _43_surface.x;
float2 _45_n = _43_surface.yz;
float2 _46_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _45_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _45_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _45_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _45_n.y);
float _47_magnitude = max(length(_46_gradient), 0.0001);
float3 item = float3(_44_d / _47_magnitude, _46_gradient / _47_magnitude);
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
float4 _48_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2];
float2 _49_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0))) - _48_g.xy * 0.5;
_globals.cornerCurve_1 = _48_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[2];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[2];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[2];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[2];
float _50_r = min(_48_g.z, min(_48_g.x, _48_g.y) * 0.5);
float3 _51_surface = roundedBoxSample_1_f3f2f2f(_globals, _49_p, _48_g.xy * 0.5, _50_r);
float _52_d = _51_surface.x;
float2 _53_n = _51_surface.yz;
float2 _54_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].x * _53_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].x * _53_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].y * _53_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].y * _53_n.y);
float _55_magnitude = max(length(_54_gradient), 0.0001);
float3 item = float3(_52_d / _55_magnitude, _54_gradient / _55_magnitude);
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
return d;
}
float2 groupXY_material0_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[0].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material0_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[0].w < 0.0 ? groupXY_material0_1_f2f2(_globals, xy) : xy;
float2 _56_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _57_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _56_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_57_e = saturate(_57_e + 0.5h);
float4 _58_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
return (((_57_e.x * _57_e.y) * _57_e.z) * _57_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _56_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _58_c.xy, _58_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
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
float3 _59_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _60_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].x;
float _61_luma = dot(_59_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_60_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].y - _60_black) * _61_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[0].z * (_59_c - float3(_61_luma));
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
float2 _65_xy = nativeSampleXY_material0_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material0_1_f2f2(_globals, _65_xy);
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
float2 _66_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].x) * nativeBevel_material0_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[0].z, radius);
rgb = float3(nativeGlassBleed_material0_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material0_1_h4f2f2f(_globals, xy, _66_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
float2 groupXY_material1_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[1].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material1_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[1].w < 0.0 ? groupXY_material1_1_f2f2(_globals, xy) : xy;
float2 _146_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_16.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_16;
half4 _147_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_17 - _146_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_147_e = saturate(_147_e + 0.5h);
float4 _148_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_20;
return (((_147_e.x * _147_e.y) * _147_e.z) * _147_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_18.zw) * _146_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_18, _148_c.xy, _148_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_20, _globals.image_20);
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
float3 _149_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _150_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].x;
float _151_luma = dot(_149_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_150_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].y - _150_black) * _151_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[1].z * (_149_c - float3(_151_luma));
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
float2 _155_xy = nativeSampleXY_material1_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material1_1_f2f2(_globals, _155_xy);
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
float2 _156_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].x) * nativeBevel_material1_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[1].z, radius);
rgb = float3(nativeGlassBleed_material1_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material1_1_h4f2f2f(_globals, xy, _156_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
float2 groupXY_material2_1_f2f2(thread Globals& _globals, float2 xy) {
float3 p = float3(xy / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0), 1.0);
return float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xyz, p), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xyz, p)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
half4 groupBackdrop_material2_1_h4f2(thread Globals& _globals, float2 xy) {
float2 p = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropPyramidInfo_1[2].w < 0.0 ? groupXY_material2_1_f2f2(_globals, xy) : xy;
float2 _236_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_23.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_23.zw) * p + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_23;
half4 _237_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_24 - _236_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_237_e = saturate(_237_e + 0.5h);
float4 _238_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_27;
return (((_237_e.x * _237_e.y) * _237_e.z) * _237_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_25.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_25.zw) * _236_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_25, _238_c.xy, _238_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_27, _globals.image_27);
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
float3 _239_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _240_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].x;
float _241_luma = dot(_239_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_240_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].y - _240_black) * _241_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedTransfer_1[2].z * (_239_c - float3(_241_luma));
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
float2 _245_xy = nativeSampleXY_material2_1_f2f2f2(_globals, xy, xy);
float2 world = groupXY_material2_1_f2f2(_globals, _245_xy);
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
float2 _246_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].x) * nativeBevel_material2_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBleedShape_1[2].z, radius);
rgb = float3(nativeGlassBleed_material2_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_material2_1_h4f2f2f(_globals, xy, _246_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 xy = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
bool covered = false;
covered = covered || ((xy.x >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].x && xy.y >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].y) && xy.x <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].z) && xy.y <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[0].w;
covered = covered || ((xy.x >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].x && xy.y >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].y) && xy.x <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].z) && xy.y <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[1].w;
covered = covered || ((xy.x >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[2].x && xy.y >= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[2].y) && xy.x <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[2].z) && xy.y <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].outputBounds_1[2].w;
if (!covered) {
bool blendPossible = false;
float2 bp0 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].xy * 0.5;
float bd0 = circularBoxSDF_1_ff2f2f(bp0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].xy * 0.5, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].z, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0].y) * 0.5)) * abs(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[0].x);
float2 bp1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].xy * 0.5;
float bd1 = circularBoxSDF_1_ff2f2f(bp1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].xy * 0.5, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].z, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1].y) * 0.5)) * abs(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[1].x);
blendPossible = blendPossible || abs(bd1 - bd0) <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * 3.0;
float2 bp2 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0))) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2].xy * 0.5;
float bd2 = circularBoxSDF_1_ff2f2f(bp2, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2].xy * 0.5, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2].z, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2].y) * 0.5)) * abs(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].x);
blendPossible = blendPossible || abs(bd2 - bd0) <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * 3.0;
blendPossible = blendPossible || abs(bd2 - bd1) <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mergeSpacing_1 * 3.0;
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
float _326_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _326_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _326_cover;
_globals.weights_1[1] *= 1.0 - _326_cover;
_globals.weights_1[2] *= 1.0 - _326_cover;
_globals.weights_1[0] += _326_cover;
}
{
float _327_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _327_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _327_cover;
_globals.weights_1[1] *= 1.0 - _327_cover;
_globals.weights_1[2] *= 1.0 - _327_cover;
_globals.weights_1[1] += _327_cover;
}
{
float _328_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5) _328_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _328_cover;
_globals.weights_1[1] *= 1.0 - _328_cover;
_globals.weights_1[2] *= 1.0 - _328_cover;
_globals.weights_1[2] += _328_cover;
}
{
float _329_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _329_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _329_cover;
_globals.weights_1[1] *= 1.0 - _329_cover;
_globals.weights_1[2] *= 1.0 - _329_cover;
_globals.weights_1[0] += _329_cover;
}
{
float _330_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _330_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _330_cover;
_globals.weights_1[1] *= 1.0 - _330_cover;
_globals.weights_1[2] *= 1.0 - _330_cover;
_globals.weights_1[1] += _330_cover;
}
{
float _331_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] <= 1.5) _331_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _331_cover;
_globals.weights_1[1] *= 1.0 - _331_cover;
_globals.weights_1[2] *= 1.0 - _331_cover;
_globals.weights_1[2] += _331_cover;
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
{
float intensity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionIntensity_1[2] * clamp(0.5 - _globals.distances_1[2] * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (intensity > groupInteraction) {
groupInteraction = intensity;
float3 touch = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_interactionLocation_1[2], 1.0);
groupTouch = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xyz, touch), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xyz, touch));
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
float _332_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[0], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 0.5);
half4 _333_field;
if (coverage >= 1.0) _333_field = nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _332_radius); else {
float _334_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].x * nativeBevel_material0_1_ffff(localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[0].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[0].z, _332_radius);
half4 _335_shadow = nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, localXY + _globals.materialNormal_1 * _334_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[0].w);
_333_field = nativeGlassExterior_material0_1_h4ffff2h4h4h4(_globals, localDistance, shadowDistance, ringDistance, _globals.materialNormal_1, original, _335_shadow, original);
if (coverage > 0.0) _333_field = mix(_333_field, nativeGlassBackgroundBody_material0_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _332_radius), half(coverage));
}
float _336_alpha = clamp(nativeLightingRingAlpha_material0_1_ffff2(_globals, localDistance, ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _337_foreground = half4(0.0h, 0.0h, 0.0h, half(_336_alpha));
_333_field = _337_foreground + _333_field * (1.0h - _337_foreground.w);
half4 _338_material = _333_field;
half3 _339_exteriorFace = half3(nativeFace_material0_1_f3f3f(_globals, nativeUnpremul_material0_1_f3h4(original), localDistance));
float _340_amount = nativeGlassFilterHighlight_material0_1_fff2f(_globals, localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * localMetric);
if (_340_amount > 1e-06) {
float3 _341_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _342_face = float3(_339_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].x > 0.5) _342_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y < 0.0 ? min(_342_face, _341_raw) : max(_342_face, _341_raw);
float _343_alpha = float(_338_material.w);
float3 _344_color = float3(_338_material.xyz) + (1.0 - _343_alpha) * mix(_341_raw, _342_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[0] * _340_amount);
_344_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[0].y * _340_amount) * (3.0 - 2.0 * _344_color);
float _345_resultAlpha = _343_alpha + ((1.0 - _343_alpha) * _340_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[0].y;
_338_material = half4(half3(_344_color - (1.0 - _345_resultAlpha) * _341_raw), half(_345_resultAlpha));
}
_333_field = nativeGlassHoldingTone_material0_1_h4fh4(_globals, localDistance, _338_material);
float3 _346_fieldColor = nativeLightingClampColor_material0_1_f3f3ff(nativeUnpremul_material0_1_f3h4(_333_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
_333_field = half4(half3(_346_fieldColor) * _333_field.w, _333_field.w);
half4 _347_composited = _333_field + original * (1.0h - _333_field.w);
float3 _348_clamped = clamp(nativeUnpremul_material0_1_f3h4(_347_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0])));
_347_composited = half4(half3(_348_clamped) * _347_composited.w, _347_composited.w);
float3 _349_rgb = nativeUnpremul_material0_1_f3h4(_347_composited);
float _350_materialAlpha = float(_347_composited.w);
float _351_inside = nativeBevel_material0_1_ffff(localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[0].y, _332_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[0] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w > 0.0) {
float4 _352_gradient = nativeGradientColor_material0_1_f4f(_globals, localDistance);
float4 _353_matrixInput = float4(_349_rgb, float(_347_composited.w));
float4 _354_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[0], _353_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[0], _353_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[0], _353_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[0], _353_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[0];
_354_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_354_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[0].y);
float4 _355_accent = _354_result;
_355_accent.w = clamp(_355_accent.w, 0.0, 1.0);
_355_accent.xyz = _355_accent.xyz * _355_accent.w;
float _356_mask = _352_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[0].y > 0.5) _356_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w; else {
_355_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _355_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
_355_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w + _355_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[0].w);
}
_355_accent *= clamp(_356_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w, 0.0, 1.0);
float _357_outAlpha = _355_accent.w + _350_materialAlpha * (1.0 - _355_accent.w);
_349_rgb = (_355_accent.xyz + (_349_rgb * _350_materialAlpha) * (1.0 - _355_accent.w)) / max(_357_outAlpha, 1e-06);
_350_materialAlpha = _357_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] >= 0.0) {
_349_rgb = mix(_349_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[0].w) * coverage, 0.0, 1.0));
}
_349_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[0] * _351_inside;
_349_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[0] * smoothstep(0.85, 0.98, dot(_349_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _358_unlit = _349_rgb;
float3 _359_color = float3(half3(_349_rgb));
float2 _360_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? sceneNormal : _globals.materialNormal_1;
float _361_norm = length(_360_normal);
if (_361_norm > 1e-05) _360_normal /= _361_norm;
float _362_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].y);
float _363_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].y);
float _364_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[0].w;
float _365_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[0].w;
float _366_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].x;
float _367_key = 0.0;
float _368_fill = 0.0;
float _369_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].w > 0.5 ? localDistance / max(_361_norm, 1e-06) : localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z > 0.0) {
float _370_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _369_localSDF, _362_keyHeight, _366_curvature, _360_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_360_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _364_keySpread);
float _371_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _369_localSDF, _362_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _360_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_360_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].x, _364_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_367_key = nativeLightingBias_material0_1_fff(_370_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_371_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z > 0.0) {
float _372_primary = nativeLightingSDFBand_material0_1_fffff2(_globals, _369_localSDF, _363_fillHeight, _366_curvature, _360_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_360_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _365_fillSpread);
float _373_diffuse = nativeLightingSDFBand_material0_1_fffff2(_globals, _369_localSDF, _363_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].w, 1.0, _360_normal) * nativeLightingSDFDirection_material0_1_ff2ff(_360_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].x, _365_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].z);
_368_fill = nativeLightingBias_material0_1_fff(_372_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z) + nativeLightingBias_material0_1_fff(_373_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[0].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[0].y);
}
float3 _374_unlitColor = _359_color;
float4 _375_c = float4(_374_unlitColor, 1.0);
float4 _376_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[0], _375_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[0], _375_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[0], _375_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[0], _375_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[0];
_376_result.xyz = nativeLightingClampColor_material0_1_f3f3ff(_376_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[0].y);
float4 _377_transformed = _376_result;
float _378_coverage = _367_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[0].w + _368_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[0].w;
_378_coverage *= clamp(_377_transformed.w, 0.0, 1.0);
_359_color = _374_unlitColor + _378_coverage * (_377_transformed.xyz - _374_unlitColor);
_349_rgb = float3(half3(nativeLightingClampColor_material0_1_f3f3ff(_359_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[0])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y)));
float _379_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[0], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[0], clamp(dot(_358_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_349_rgb = mix(_358_unlit, _349_rgb, _379_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0] > 0.0 && _351_inside > 0.0) {
float2 _380_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[0];
half4 _381_reflected = (nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) + _380_tangent, 0.0) + nativeAtRadius_material0_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[0]) - _380_tangent, 0.0)) * 0.5h;
_349_rgb = mix(_349_rgb, nativeUnpremul_material0_1_f3h4(_381_reflected), (_351_inside * _351_inside) * 0.1);
}
float _382_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * coverage;
if (_382_glow > 0.0) {
float _383_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y) * 1.6);
float _384_sheen = 1.0 - smoothstep(0.0, _383_glowRadius, length(localXY - _globals.interactionLocation_1));
float _385_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float3 _386_restingResult = _349_rgb;
_349_rgb = mix(_349_rgb, float3(1.0), _382_glow * _385_ambientPressOpacity);
_349_rgb = mix(_349_rgb, float3(1.0), (_382_glow * _384_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]));
float _387_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y));
float _388_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[0].y);
float _389_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[0] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] < 0.5) * smoothstep(160.0, 280.0, _388_longSide)) * smoothstep(2.5, 4.0, _388_longSide / _387_shortSide);
float _390_peak = max(_386_restingResult.x, max(_386_restingResult.y, _386_restingResult.z));
float _391_chromaticRange = _390_peak - min(_386_restingResult.x, min(_386_restingResult.y, _386_restingResult.z));
float _392_neutralPress = 1.0 - smoothstep(0.01, 0.1, _391_chromaticRange);
float _393_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[0]);
float _394_pressGain = mix(1.18 - 0.06 * _390_peak, 1.06, _393_lightAppearance);
float3 _395_colorPress = mix(float3(float(dot(half3(_386_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _386_restingResult, mix(1.03, 1.06, _393_lightAppearance)) * _394_pressGain;
_395_colorPress += float3(_392_neutralPress * mix(0.02745098, 0.04509804, _393_lightAppearance));
_349_rgb = mix(_349_rgb, mix(_386_restingResult, _395_colorPress, _382_glow), _389_platterPress);
}
_349_rgb = nativeLightingClampColor_material0_1_f3f3ff(_349_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[0]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[0].y);
float _396_alpha = clamp(_350_materialAlpha, 0.0, 1.0);
half4 color = half4(half3(_349_rgb * _396_alpha), half(_396_alpha));
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
float _397_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[1], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 0.5);
half4 _398_field;
if (coverage >= 1.0) _398_field = nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _397_radius); else {
float _399_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].x * nativeBevel_material1_1_ffff(localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[1].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[1].z, _397_radius);
half4 _400_shadow = nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, localXY + _globals.materialNormal_1 * _399_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[1].w);
_398_field = nativeGlassExterior_material1_1_h4ffff2h4h4h4(_globals, localDistance, shadowDistance, ringDistance, _globals.materialNormal_1, original, _400_shadow, original);
if (coverage > 0.0) _398_field = mix(_398_field, nativeGlassBackgroundBody_material1_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _397_radius), half(coverage));
}
float _401_alpha = clamp(nativeLightingRingAlpha_material1_1_ffff2(_globals, localDistance, ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _402_foreground = half4(0.0h, 0.0h, 0.0h, half(_401_alpha));
_398_field = _402_foreground + _398_field * (1.0h - _402_foreground.w);
half4 _403_material = _398_field;
half3 _404_exteriorFace = half3(nativeFace_material1_1_f3f3f(_globals, nativeUnpremul_material1_1_f3h4(original), localDistance));
float _405_amount = nativeGlassFilterHighlight_material1_1_fff2f(_globals, localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * localMetric);
if (_405_amount > 1e-06) {
float3 _406_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _407_face = float3(_404_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].x > 0.5) _407_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y < 0.0 ? min(_407_face, _406_raw) : max(_407_face, _406_raw);
float _408_alpha = float(_403_material.w);
float3 _409_color = float3(_403_material.xyz) + (1.0 - _408_alpha) * mix(_406_raw, _407_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[1] * _405_amount);
_409_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[1].y * _405_amount) * (3.0 - 2.0 * _409_color);
float _410_resultAlpha = _408_alpha + ((1.0 - _408_alpha) * _405_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[1].y;
_403_material = half4(half3(_409_color - (1.0 - _410_resultAlpha) * _406_raw), half(_410_resultAlpha));
}
_398_field = nativeGlassHoldingTone_material1_1_h4fh4(_globals, localDistance, _403_material);
float3 _411_fieldColor = nativeLightingClampColor_material1_1_f3f3ff(nativeUnpremul_material1_1_f3h4(_398_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
_398_field = half4(half3(_411_fieldColor) * _398_field.w, _398_field.w);
half4 _412_composited = _398_field + original * (1.0h - _398_field.w);
float3 _413_clamped = clamp(nativeUnpremul_material1_1_f3h4(_412_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1])));
_412_composited = half4(half3(_413_clamped) * _412_composited.w, _412_composited.w);
float3 _414_rgb = nativeUnpremul_material1_1_f3h4(_412_composited);
float _415_materialAlpha = float(_412_composited.w);
float _416_inside = nativeBevel_material1_1_ffff(localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[1].y, _397_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[1] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w > 0.0) {
float4 _417_gradient = nativeGradientColor_material1_1_f4f(_globals, localDistance);
float4 _418_matrixInput = float4(_414_rgb, float(_412_composited.w));
float4 _419_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[1], _418_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[1], _418_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[1], _418_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[1], _418_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[1];
_419_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_419_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[1].y);
float4 _420_accent = _419_result;
_420_accent.w = clamp(_420_accent.w, 0.0, 1.0);
_420_accent.xyz = _420_accent.xyz * _420_accent.w;
float _421_mask = _417_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[1].y > 0.5) _421_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w; else {
_420_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _420_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
_420_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w + _420_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[1].w);
}
_420_accent *= clamp(_421_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w, 0.0, 1.0);
float _422_outAlpha = _420_accent.w + _415_materialAlpha * (1.0 - _420_accent.w);
_414_rgb = (_420_accent.xyz + (_414_rgb * _415_materialAlpha) * (1.0 - _420_accent.w)) / max(_422_outAlpha, 1e-06);
_415_materialAlpha = _422_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] >= 0.0) {
_414_rgb = mix(_414_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[1].w) * coverage, 0.0, 1.0));
}
_414_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[1] * _416_inside;
_414_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[1] * smoothstep(0.85, 0.98, dot(_414_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _423_unlit = _414_rgb;
float3 _424_color = float3(half3(_414_rgb));
float2 _425_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? sceneNormal : _globals.materialNormal_1;
float _426_norm = length(_425_normal);
if (_426_norm > 1e-05) _425_normal /= _426_norm;
float _427_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].y);
float _428_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].y);
float _429_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[1].w;
float _430_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[1].w;
float _431_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].x;
float _432_key = 0.0;
float _433_fill = 0.0;
float _434_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].w > 0.5 ? localDistance / max(_426_norm, 1e-06) : localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z > 0.0) {
float _435_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _434_localSDF, _427_keyHeight, _431_curvature, _425_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_425_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _429_keySpread);
float _436_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _434_localSDF, _427_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _425_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_425_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].x, _429_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_432_key = nativeLightingBias_material1_1_fff(_435_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_436_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z > 0.0) {
float _437_primary = nativeLightingSDFBand_material1_1_fffff2(_globals, _434_localSDF, _428_fillHeight, _431_curvature, _425_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_425_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _430_fillSpread);
float _438_diffuse = nativeLightingSDFBand_material1_1_fffff2(_globals, _434_localSDF, _428_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].w, 1.0, _425_normal) * nativeLightingSDFDirection_material1_1_ff2ff(_425_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].x, _430_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].z);
_433_fill = nativeLightingBias_material1_1_fff(_437_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z) + nativeLightingBias_material1_1_fff(_438_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[1].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[1].y);
}
float3 _439_unlitColor = _424_color;
float4 _440_c = float4(_439_unlitColor, 1.0);
float4 _441_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[1], _440_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[1], _440_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[1], _440_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[1], _440_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[1];
_441_result.xyz = nativeLightingClampColor_material1_1_f3f3ff(_441_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[1].y);
float4 _442_transformed = _441_result;
float _443_coverage = _432_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[1].w + _433_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[1].w;
_443_coverage *= clamp(_442_transformed.w, 0.0, 1.0);
_424_color = _439_unlitColor + _443_coverage * (_442_transformed.xyz - _439_unlitColor);
_414_rgb = float3(half3(nativeLightingClampColor_material1_1_f3f3ff(_424_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[1])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y)));
float _444_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[1], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[1], clamp(dot(_423_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_414_rgb = mix(_423_unlit, _414_rgb, _444_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1] > 0.0 && _416_inside > 0.0) {
float2 _445_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[1];
half4 _446_reflected = (nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) + _445_tangent, 0.0) + nativeAtRadius_material1_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[1]) - _445_tangent, 0.0)) * 0.5h;
_414_rgb = mix(_414_rgb, nativeUnpremul_material1_1_f3h4(_446_reflected), (_416_inside * _416_inside) * 0.1);
}
float _447_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * coverage;
if (_447_glow > 0.0) {
float _448_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y) * 1.6);
float _449_sheen = 1.0 - smoothstep(0.0, _448_glowRadius, length(localXY - _globals.interactionLocation_1));
float _450_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float3 _451_restingResult = _414_rgb;
_414_rgb = mix(_414_rgb, float3(1.0), _447_glow * _450_ambientPressOpacity);
_414_rgb = mix(_414_rgb, float3(1.0), (_447_glow * _449_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]));
float _452_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y));
float _453_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[1].y);
float _454_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[1] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] < 0.5) * smoothstep(160.0, 280.0, _453_longSide)) * smoothstep(2.5, 4.0, _453_longSide / _452_shortSide);
float _455_peak = max(_451_restingResult.x, max(_451_restingResult.y, _451_restingResult.z));
float _456_chromaticRange = _455_peak - min(_451_restingResult.x, min(_451_restingResult.y, _451_restingResult.z));
float _457_neutralPress = 1.0 - smoothstep(0.01, 0.1, _456_chromaticRange);
float _458_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[1]);
float _459_pressGain = mix(1.18 - 0.06 * _455_peak, 1.06, _458_lightAppearance);
float3 _460_colorPress = mix(float3(float(dot(half3(_451_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _451_restingResult, mix(1.03, 1.06, _458_lightAppearance)) * _459_pressGain;
_460_colorPress += float3(_457_neutralPress * mix(0.02745098, 0.04509804, _458_lightAppearance));
_414_rgb = mix(_414_rgb, mix(_451_restingResult, _460_colorPress, _447_glow), _454_platterPress);
}
_414_rgb = nativeLightingClampColor_material1_1_f3f3ff(_414_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[1]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[1].y);
float _461_alpha = clamp(_415_materialAlpha, 0.0, 1.0);
half4 color = half4(half3(_414_rgb * _461_alpha), half(_461_alpha));
result += half(_globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1]) * (color - original);
}
}
if (_globals.weights_1[2] > 0.0) {
float2 localXY = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0)));
float2 localGradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].x * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].x * sceneNormal.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].y * sceneNormal.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].y * sceneNormal.y);
float localMetric = max(length(localGradient), 0.0001);
_globals.materialNormal_1 = localGradient / localMetric;
_globals.interactionIntensity_1 = groupInteraction;
_globals.interactionLocation_1 = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(groupTouch, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(groupTouch, 1.0)));
float localDistance = sceneDistance / localMetric;
if (nativeGlassOutputCoverage_material2_1_ff(_globals, localDistance) > 0.0 || coverage > 0.0) {
float ringDistance = localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].w > 0.0 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadow_1[2].y > 0.0) {
float2 ringOffset = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRingShadowOffset_1[2];
ringDistance = mergeGeometry_1_ff2b(_globals, xy - ringOffset, false) / localMetric;
}
float shadowDistance = localDistance;
if (coverage < 1.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].w > 0.0)) {
float2 shadowOffset = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardX_1[2].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[2]), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].forwardY_1[2].xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowOffset_1[2]));
shadowDistance = mergeGeometry_1_ff2b(_globals, xy - shadowOffset, false) / localMetric;
}
float _462_radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRadius_1[2], min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y) * 0.5);
half4 _463_field;
if (coverage >= 1.0) _463_field = nativeGlassBackgroundBody_material2_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _462_radius); else {
float _464_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].x * nativeBevel_material2_1_ffff(localDistance + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowResponse_1[2].y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeShadowShape_1[2].z, _462_radius);
half4 _465_shadow = nativeAtRadius_material2_1_h4f2f2f(_globals, localXY, localXY + _globals.materialNormal_1 * _464_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBlurRadii_1[2].w);
_463_field = nativeGlassExterior_material2_1_h4ffff2h4h4h4(_globals, localDistance, shadowDistance, ringDistance, _globals.materialNormal_1, original, _465_shadow, original);
if (coverage > 0.0) _463_field = mix(_463_field, nativeGlassBackgroundBody_material2_1_h4f2ff2f(_globals, localXY, localDistance, _globals.materialNormal_1, _462_radius), half(coverage));
}
float _466_alpha = clamp(nativeLightingRingAlpha_material2_1_ffff2(_globals, localDistance, ringDistance, _globals.materialNormal_1), 0.0, 1.0);
half4 _467_foreground = half4(0.0h, 0.0h, 0.0h, half(_466_alpha));
_463_field = _467_foreground + _463_field * (1.0h - _467_foreground.w);
half4 _468_material = _463_field;
half3 _469_exteriorFace = half3(nativeFace_material2_1_f3f3f(_globals, nativeUnpremul_material2_1_f3h4(original), localDistance));
float _470_amount = nativeGlassFilterHighlight_material2_1_fff2f(_globals, localDistance, _globals.materialNormal_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1 * localMetric);
if (_470_amount > 1e-06) {
float3 _471_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _472_face = float3(_469_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[2].x > 0.5) _472_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].y < 0.0 ? min(_472_face, _471_raw) : max(_472_face, _471_raw);
float _473_alpha = float(_468_material.w);
float3 _474_color = float3(_468_material.xyz) + (1.0 - _473_alpha) * mix(_471_raw, _472_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFaceOpacity_1[2] * _470_amount);
_474_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterHighlightColor_1[2].y * _470_amount) * (3.0 - 2.0 * _474_color);
float _475_resultAlpha = _473_alpha + ((1.0 - _473_alpha) * _470_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFilterOptions_1[2].y;
_468_material = half4(half3(_474_color - (1.0 - _475_resultAlpha) * _471_raw), half(_475_resultAlpha));
}
_463_field = nativeGlassHoldingTone_material2_1_h4fh4(_globals, localDistance, _468_material);
float3 _476_fieldColor = nativeLightingClampColor_material2_1_f3f3ff(nativeUnpremul_material2_1_f3h4(_463_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y);
_463_field = half4(half3(_476_fieldColor) * _463_field.w, _463_field.w);
half4 _477_composited = _463_field + original * (1.0h - _463_field.w);
float3 _478_clamped = clamp(nativeUnpremul_material2_1_f3h4(_477_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2])));
_477_composited = half4(half3(_478_clamped) * _477_composited.w, _477_composited.w);
float3 _479_rgb = nativeUnpremul_material2_1_f3h4(_477_composited);
float _480_materialAlpha = float(_477_composited.w);
float _481_inside = nativeBevel_material2_1_ffff(localDistance, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeRefraction_1[2].y, _462_radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentEnabled_1[2] > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w > 0.0) {
float4 _482_gradient = nativeGradientColor_material2_1_f4f(_globals, localDistance);
float4 _483_matrixInput = float4(_479_rgb, float(_477_composited.w));
float4 _484_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix0_1[2], _483_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix1_1[2], _483_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix2_1[2], _483_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrix3_1[2], _483_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixBias_1[2];
_484_result.xyz = nativeLightingClampColor_material2_1_f3f3ff(_484_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMatrixOptions_1[2].y);
float4 _485_accent = _484_result;
_485_accent.w = clamp(_485_accent.w, 0.0, 1.0);
_485_accent.xyz = _485_accent.xyz * _485_accent.w;
float _486_mask = _482_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentMaskOptions_1[2].y > 0.5) _486_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w; else {
_485_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w + _485_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w);
_485_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w + _485_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeAccentFill_1[2].w);
}
_485_accent *= clamp(_486_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w, 0.0, 1.0);
float _487_outAlpha = _485_accent.w + _480_materialAlpha * (1.0 - _485_accent.w);
_479_rgb = (_485_accent.xyz + (_479_rgb * _480_materialAlpha) * (1.0 - _485_accent.w)) / max(_487_outAlpha, 1e-06);
_480_materialAlpha = _487_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[2] >= 0.0) {
_479_rgb = mix(_479_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeTintOpacity_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tint_1[2].w) * coverage, 0.0, 1.0));
}
_479_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeEdgeAbsorption_1[2] * _481_inside;
_479_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightBackdropShade_1[2] * smoothstep(0.85, 0.98, dot(_479_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _488_unlit = _479_rgb;
float3 _489_color = float3(half3(_479_rgb));
float2 _490_normal = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].w > 0.5 ? sceneNormal : _globals.materialNormal_1;
float _491_norm = length(_490_normal);
if (_491_norm > 1e-05) _490_normal /= _491_norm;
float _492_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].y);
float _493_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].y);
float _494_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightShape_1[2].w;
float _495_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightShape_1[2].w;
float _496_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].x;
float _497_key = 0.0;
float _498_fill = 0.0;
float _499_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].w > 0.5 ? localDistance / max(_491_norm, 1e-06) : localDistance;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z > 0.0) {
float _500_primary = nativeLightingSDFBand_material2_1_fffff2(_globals, _499_localSDF, _492_keyHeight, _496_curvature, _490_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_490_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].x, _494_keySpread);
float _501_diffuse = nativeLightingSDFBand_material2_1_fffff2(_globals, _499_localSDF, _492_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].w, 1.0, _490_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_490_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].x, _494_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].z);
_497_key = nativeLightingBias_material2_1_fff(_500_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z) + nativeLightingBias_material2_1_fff(_501_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlight_1[2].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z > 0.0) {
float _502_primary = nativeLightingSDFBand_material2_1_fffff2(_globals, _499_localSDF, _493_fillHeight, _496_curvature, _490_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_490_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].x, _495_fillSpread);
float _503_diffuse = nativeLightingSDFBand_material2_1_fffff2(_globals, _499_localSDF, _493_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].w, 1.0, _490_normal) * nativeLightingSDFDirection_material2_1_ff2ff(_490_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].x, _495_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].z);
_498_fill = nativeLightingBias_material2_1_fff(_502_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z) + nativeLightingBias_material2_1_fff(_503_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlight_1[2].z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightResponse_1[2].y);
}
float3 _504_unlitColor = _489_color;
float4 _505_c = float4(_504_unlitColor, 1.0);
float4 _506_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix0_1[2], _505_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix1_1[2], _505_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix2_1[2], _505_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrix3_1[2], _505_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixBias_1[2];
_506_result.xyz = nativeLightingClampColor_material2_1_f3f3ff(_506_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeHighlightMatrixOptions_1[2].y);
float4 _507_transformed = _506_result;
float _508_coverage = _497_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeKeyHighlightColor_1[2].w + _498_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeFillHighlightColor_1[2].w;
_508_coverage *= clamp(_507_transformed.w, 0.0, 1.0);
_489_color = _504_unlitColor + _508_coverage * (_507_transformed.xyz - _504_unlitColor);
_479_rgb = float3(half3(nativeLightingClampColor_material2_1_f3f3ff(_489_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingDisplayHeadroom_1[2])), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y)));
float _509_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDarkSpecularGain_1[2], _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeBrightSpecularGain_1[2], clamp(dot(_488_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_479_rgb = mix(_488_unlit, _479_rgb, _509_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2] > 0.0 && _481_inside > 0.0) {
float2 _510_tangent = float2(-_globals.materialNormal_1.y, _globals.materialNormal_1.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionSpread_1[2];
half4 _511_reflected = (nativeAtRadius_material2_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2]) + _510_tangent, 0.0) + nativeAtRadius_material2_1_h4f2f2f(_globals, localXY, (localXY + _globals.materialNormal_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeReflectionDistance_1[2]) - _510_tangent, 0.0)) * 0.5h;
_479_rgb = mix(_479_rgb, nativeUnpremul_material2_1_f3h4(_511_reflected), (_481_inside * _481_inside) * 0.1);
}
float _512_glow = clamp(_globals.interactionIntensity_1, 0.0, 1.0) * coverage;
if (_512_glow > 0.0) {
float _513_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y) * 1.6);
float _514_sheen = 1.0 - smoothstep(0.0, _513_glowRadius, length(localXY - _globals.interactionLocation_1));
float _515_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[2] > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]));
float3 _516_restingResult = _479_rgb;
_479_rgb = mix(_479_rgb, float3(1.0), _512_glow * _515_ambientPressOpacity);
_479_rgb = mix(_479_rgb, float3(1.0), (_512_glow * _514_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]));
float _517_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y));
float _518_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_lensSize_1[2].y);
float _519_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_bodyMode_1[2] < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] < 0.5) * smoothstep(160.0, 280.0, _518_longSide)) * smoothstep(2.5, 4.0, _518_longSide / _517_shortSide);
float _520_peak = max(_516_restingResult.x, max(_516_restingResult.y, _516_restingResult.z));
float _521_chromaticRange = _520_peak - min(_516_restingResult.x, min(_516_restingResult.y, _516_restingResult.z));
float _522_neutralPress = 1.0 - smoothstep(0.01, 0.1, _521_chromaticRange);
float _523_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_backdropLuma_1[2]);
float _524_pressGain = mix(1.18 - 0.06 * _520_peak, 1.06, _523_lightAppearance);
float3 _525_colorPress = mix(float3(float(dot(half3(_516_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _516_restingResult, mix(1.03, 1.06, _523_lightAppearance)) * _524_pressGain;
_525_colorPress += float3(_522_neutralPress * mix(0.02745098, 0.04509804, _523_lightAppearance));
_479_rgb = mix(_479_rgb, mix(_516_restingResult, _525_colorPress, _512_glow), _519_platterPress);
}
_479_rgb = nativeLightingClampColor_material2_1_f3f3ff(_479_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeDisplayHeadroom_1[2]), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_nativeLightingClamp_1[2].y);
float _526_alpha = clamp(_480_materialAlpha, 0.0, 1.0);
half4 color = half4(half3(_479_rgb * _526_alpha), half(_526_alpha));
result += half(_globals.weights_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[2]) * (color - original);
}
}
return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], texture2d<half> image_20_Tex [[texture(2)]], sampler image_20_Smplr [[sampler(2)]], texture2d<half> image_27_Tex [[texture(3)]], sampler image_27_Smplr [[sampler(3)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}, {image_13_Tex, image_13_Smplr}, {image_20_Tex, image_20_Smplr}, {image_27_Tex, image_27_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
