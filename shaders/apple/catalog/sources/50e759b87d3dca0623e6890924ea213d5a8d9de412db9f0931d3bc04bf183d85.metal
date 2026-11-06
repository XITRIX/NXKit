#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float2 resolution_1;
float2 lensCenter_1;
float2 lensSize_1;
float cornerRadius_1;
float cornerCurve_1;
float externalShapeCoverage_1;
float maximumSampleRadius_1;
float contentScale_1;
float filterScale_1;
float interactionIntensity_1;
float2 interactionLocation_1;
float bodyMode_1;
float tintMode_1;
float referenceAppearance_1;
float backdropLuma_1;
float4 tint_1;
float nativeFaceBlack_1;
float3 nativeFaceTransfer_1;
float4 nativeFaceFill_1;
float2 nativeFaceLuma_1;
float nativeFaceOpacity_1;
float3 nativeFillOpacity_1;
float4 nativeBlurDistances_1;
float4 nativeBlurOpacities_1;
float4 nativeRefraction_1;
float3 nativeRefractionMask_1;
float2 nativeSDRGradient_1;
float nativeDisplayHeadroom_1;
float nativeAccentEnabled_1;
float4 nativeAccent0_1;
float4 nativeAccent1_1;
float4 nativeAccent2_1;
float3 nativeAccentDistances_1;
float4 nativeAccentTiming0_1;
float4 nativeAccentTiming1_1;
float2 nativeAccentTimingKnown_1;
float2 nativeAccentMaskOptions_1;
float4 nativeAccentFill_1;
float4 nativeAccentMatrix0_1;
float4 nativeAccentMatrix1_1;
float4 nativeAccentMatrix2_1;
float4 nativeAccentMatrix3_1;
float4 nativeAccentMatrixBias_1;
float3 nativeAccentMatrixOptions_1;
float nativeDispersion_1;
float nativeEdgeAbsorption_1;
float nativeTintOpacity_1;
float nativeBrightBackdropShade_1;
float nativeDarkSpecularGain_1;
float nativeBrightSpecularGain_1;
float nativeReflectionDistance_1;
float nativeReflectionSpread_1;
float4 backdropPyramidRect_1;
float4 backdropPyramidInfo_1;
float nativePyramidStretch_1;
float nativeMinimumBlurRadius_1;
float4 nativeBlurRadii_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
float2 nativeOutputDistance_1;
float4 nativeShadowShape_1;
float2 nativeShadowOffset_1;
float4 nativeShadowTransfer_1;
float4 nativeShadowFill_1;
float4 nativeShadowResponse_1;
float4 nativeRingShadow_1;
float nativeRingShadowOffset_1;
float4 nativeBleedShape_1;
float4 nativeBleedTransfer_1;
float2 nativeBleedDistance_1;
float4 nativeFilterHighlight_1;
float3 nativeFilterHighlightColor_1;
float4 nativeKeyHighlight_1;
float4 nativeFillHighlight_1;
float4 nativeKeyHighlightShape_1;
float4 nativeFillHighlightShape_1;
float4 nativeKeyHighlightColor_1;
float4 nativeFillHighlightColor_1;
float4 nativeHighlightResponse_1;
float4 nativeHighlightMatrix0_1;
float4 nativeHighlightMatrix1_1;
float4 nativeHighlightMatrix2_1;
float4 nativeHighlightMatrix3_1;
float4 nativeHighlightMatrixBias_1;
float4 nativeHighlightMatrixOptions_1;
float3 nativeLightingClamp_1;
float nativeLightingDisplayHeadroom_1;
float4 nativeHoldingTone_1;
float3 nativeFilterOptions_1;
float2 lensBackdropDisplacement_1;
float4 lensForegroundAberration_1;
float4 lensForegroundEdge_1;
float lensForegroundCaptureMargin_1;
float2 lensInnerRefractionFraction_1;
float3 lensControlRefraction_1;
float lensProgress_1;
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
half4 LocalMatrix_10_h4f2(thread Globals& _globals, float2 pos) {
float2 _4_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10;
half4 _5_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _4_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_5_e = saturate(_5_e + 0.5h);
return (((_5_e.x * _5_e.y) * _5_e.z) * _5_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _4_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12));
}
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float boxRoundedSDF_1_ff2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float toBrightness_1_fh3(half3 c);
float nativeLightingAA_1_f();
float nativeLightingHeadroom_1_f();
float nativeLightingLuma_1_ff3(float3 c);
float3 nativeLightingUnpremul_1_f3h4(half4 c);
float nativeGlassOutputCoverage_1_ff(float sdf);
float3 nativeLightingTransfer_1_f3f3fff(float3 c, float black, float white, float saturation);
float3 nativeLightingClampColor_1_f3f3ff(float3 c, float limit, float preserveHue);
float nativeLightingErfc_1_ff(float valueInput);
float nativeLightingRingAlpha_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal);
half4 nativeLightingOver_1_h4h4h4(half4 foreground, half4 background);
float nativeLightingBias_1_fff(float coverage, float amount);
float nativeLightingSDFBand_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal);
float nativeLightingSDFDirection_1_ff2ff(float2 normal, float angle, float spread);
float nativeGlassFilterHighlight_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale);
half4 nativeGlassHoldingTone_1_h4fh4(thread Globals& _globals, float sdf, half4 material);
half4 nativeGlassBackgroundFinalize_1_h4fh4h4h3ff2f(float sdf, half4 material, half4 original, half3 exteriorFace, float faceOpacity, float2 normal, float localScale);
half4 nativeGlassExterior_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop);
half3 nativeGlassBleed_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop);
half4 nativeGlassRingComposite_1_h4fff2fh4(float sdf, float shiftedRingSDF, float2 normal, float fieldAlpha, half4 material);
float4 nativeLightingHighlightMatrix_1_f4f4(float4 c);
half3 nativeGlassHighlight_1_h3ff2f2h3(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop);
float3 lensShape_1_f3f2f2(thread Globals& _globals, float2 p, float2 size);
float lensShoulder_1_ffff(float depth, float height, float offset);
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization);
float lensEdgeOpacity_1_fff4(float sdf, float4 edge);
float2 lensAberration_1_f2f2ff2(float2 normal, float angle, float2 sourceExtent);
float2 lensForegroundExtent_1_f2f2f(float2 size, float margin);
float4 lensRefraction_1_f4(thread Globals& _globals);
half4 lensBackdropAt_1_h4f2(thread Globals& _globals, float2 coordinate);
float2 nativeSampleXY_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target);
float2 nativeSamplingWorld_1_f2f2(float2 xy);
float nativeRadiusLOD_1_ff(thread Globals& _globals, float radius);
float2 nativePyramidPosition_1_f2f2f(thread Globals& _globals, float2 worldXY, float level);
half4 nativePyramidAt_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod);
half4 nativeAtRadius_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius);
half4 nativeSharp_1_h4f2f2(float2 origin, float2 target);
half4 nativeBleed_1_h4f2f2(float2 origin, float2 target);
half4 nativeShadow_1_h4f2f2(float2 origin, float2 target);
float3 nativeUnpremul_1_f3h4(half4 p);
float nativeLinearRamp_1_ffff(float x, float a, float b);
float nativeBlurCoverage_1_ff(thread Globals& _globals, float d);
float3 nativeFace_1_f3f3f(thread Globals& _globals, float3 color, float sdf);
float4 nativeAccentMatrix_1_f4f4(float4 color);
float nativeCubic_1_ffff(float t, float p1, float p2);
float nativeTiming_1_fff4f(float x, float4 control, float known);
float4 nativeGradientColor_1_f4f(thread Globals& _globals, float d);
float nativeBevel_1_ffff(float _0_distance, float height, float radius);
half4 nativeGlassBackgroundBody_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius);
half4 nativeGlassDecorate_1_h4f2ff2f2fh4f(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, half4 material, float surfaceCoverage);
half4 nativeGlassSurface_1_h4f2ff2f2fffh4ff(float2 xy, float sdf, float2 normal, float2 worldNormal, float radius, float shadowSDF, float ringSDF, half4 original, float coverage, float localScale);
half4 lensMaterialAt_1_h4f2(thread Globals& _globals, float2 p);
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
float boxRoundedSDF_1_ff2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) return length(p) - r;
return _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0 ? circularBoxSDF_1_ff2f2f(p, halfDim, r) : continuousCornerSample_1_f3f2f2(_globals, p, halfDim).x;
}
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
return len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
}
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
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
float2 normal = len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
return float3(len - r, normal);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _6_q = (abs(p) - halfDim) + r;
return float3((length(max(_6_q, 0.0)) + min(max(_6_q.x, _6_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float3 nativeLightingClampColor_1_f3f3ff(float3 c, float limit, float preserveHue) {
if (limit <= 0.0) return c;
if (preserveHue > 0.5) {
float maximum = max(max(c.x, c.y), c.z);
return maximum > limit ? c * (limit / maximum) : c;
}
return clamp(c, float3(-0.75), float3(limit));
}
float nativeLightingErfc_1_ff(float valueInput) {
half x = half(clamp(valueInput, -2.0, 2.0));
half square = x * x;
half value = 0.002954483h * square + -0.0344543457h;
value = value * square + 0.168212891h;
value = value * square + -0.5605469h;
return clamp(float(value * x + 0.5h), 0.0, 1.0);
}
float nativeLightingRingAlpha_1_ffff2(thread Globals& _globals, float sdf, float shiftedRingSDF, float2 normal) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.w <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.y <= 0.0) return 0.0;
float inverseRadius = 1.0 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.x, 1e-06);
float start = shiftedRingSDF * inverseRadius;
float finish = (shiftedRingSDF + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.y) * inverseRadius;
float ring = clamp(nativeLightingErfc_1_ff(start * 0.707106769) - nativeLightingErfc_1_ff(finish * 0.707106769), 0.0, 1.0);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
return (ring * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.w) * mix(1.0, coverage, clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadow_1.z, 0.0, 1.0));
}
float nativeLightingBias_1_fff(float coverage, float amount) {
amount = clamp(amount, 0.0, 1.0);
if (amount <= 0.0 || coverage <= 0.0) return 0.0;
float coefficient = 1.0 / max(amount, 1e-06) - 2.0;
return coverage / max(1.0 + coefficient * (1.0 - coverage), 1e-06);
}
float nativeLightingSDFBand_1_fffff2(thread Globals& _globals, float sdf, float height, float curvature, float2 normal) {
if (height <= 0.0) return 0.0;
float depth = -sdf;
if (depth < -5.0) return 0.0;
float t = clamp(depth / height, 0.0, 1.0);
float shape = mix(float(t < 1.0), 1.0 - t, curvature);
float width = max((abs(normal.x) + abs(normal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
return (shape * clamp(depth / width + 0.5, 0.0, 1.0)) * clamp((height - depth) / width + 0.5, 0.0, 1.0);
}
float nativeLightingSDFDirection_1_ff2ff(float2 normal, float angle, float spread) {
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(cos(angle - 1.57079637), sin(angle - 1.57079637)));
return clamp((cosine - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
}
float nativeGlassFilterHighlight_1_fff2f(thread Globals& _globals, float sdf, float2 normal, float localScale) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.x <= 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.z <= 0.0) return 0.0;
float spread = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingDisplayHeadroom_1) <= 1.0 ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.z : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.w;
float threshold = cos(clamp(abs(spread), 0.0001, 3.14159274));
float cosine = dot(normal, float2(sin(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.y), -cos(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.y)));
float2 angular = clamp((float2(cosine, -cosine) - threshold) / max(1.0 - threshold, 1e-06), 0.0, 1.0);
float2 localNormal = (normal * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) / max(localScale, 1.0);
float band;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterOptions_1.z > 0.5) {
float width = max((abs(localNormal.x) + abs(localNormal.y)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float coverage = clamp(0.5 - sdf / width, 0.0, 1.0);
float depth = -(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.x);
band = (1.0 - coverage) * clamp(depth / width + 0.5, 0.0, 1.0);
} else {
band = nativeLightingSDFBand_1_fffff2(_globals, sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.z, 0.75, localNormal);
}
return nativeLightingBias_1_fff(band * angular.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.x) + nativeLightingBias_1_fff(band * angular.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlight_1.x);
}
half4 nativeGlassHoldingTone_1_h4fh4(thread Globals& _globals, float sdf, half4 material) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.x <= 0.0) return material;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.z;
float progress = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.z) / span, 0.0, 1.0) : 0.0;
float weight = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.x * (1.0 - progress);
float alpha = clamp(float(material.w), 0.0, 1.0);
float3 target = ((float3(material.xyz) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHoldingTone_1.y) * alpha) / max(float(material.w), 1e-06);
return mix(material, half4(half3(target), half(alpha)), half(weight));
}
half4 nativeGlassExterior_1_h4ffff2h4h4h4(thread Globals& _globals, float sdf, float shiftedShadowSDF, float shiftedRingSDF, float2 normal, half4 backdrop, half4 blurredBackdrop, half4 bleedBackdrop) {
float outputMask = 1.0;
if (outputMask <= 0.0) return half4(0.0h);
float3 shadowSource = float3(blurredBackdrop.xyz) / max(float(blurredBackdrop.w), 1e-06);
half4 result = half4(0.0h);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowShape_1.w > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.w > 0.0) {
float coverage = nativeLightingErfc_1_ff(shiftedShadowSDF / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowShape_1.x, 1e-06));
float sourceLuma = dot(shadowSource, float3(0.2126, 0.7152, 0.0722));
float3 linear = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowTransfer_1.y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowTransfer_1.x) * float3(sourceLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowTransfer_1.z * (shadowSource - float3(sourceLuma));
float fillAlpha = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowFill_1.w, 0.0, 1.0);
float3 bias = float3(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowTransfer_1.x) * (1.0 - fillAlpha) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowFill_1.xyz * fillAlpha;
float contribution = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.z, 0.0, 1.0);
float3 transformed = (linear * (1.0 - fillAlpha)) * contribution + bias;
float nativeAlpha = mix(fillAlpha, 1.0, contribution);
float opacity = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowShape_1.w, 0.0, 1.0);
float alpha = (coverage * opacity) * nativeAlpha;
transformed /= max(nativeAlpha, 1e-06);
result = half4(half3(transformed * alpha), half(alpha));
}
return result * half(outputMask);
}
half3 nativeGlassBleed_1_h3fh3h4(thread Globals& _globals, float sdf, half3 body, half4 bleedBackdrop) {
float3 color = float3(body);
if (sdf > 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0)) return body;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.w > 0.0) {
float luminance = clamp(dot(color, float3(0.2125, 0.7154, 0.0721)), 0.0, 1.0);
float tone = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.w > 0.5 ? luminance : 1.0 - luminance;
float span = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedDistance_1.y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedDistance_1.x;
float edge = abs(span) > 1e-06 ? clamp((sdf - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedDistance_1.x) / span, 0.0, 1.0) : float(sdf <= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedDistance_1.x);
float gain = (tone * tone) * edge;
gain *= gain;
float3 _7_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _8_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.x;
float _9_luma = dot(_7_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_8_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.y - _8_black) * _9_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.z * (_7_c - float3(_9_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.w, 0.0, 1.0));
}
return half3(color);
}
float3 lensShape_1_f3f2f2(thread Globals& _globals, float2 p, float2 size) {
return roundedBoxSample_1_f3f2f2f(_globals, p - size * 0.5, size * 0.5, min(size.x, size.y) * 0.5);
}
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization) {
if (amount == 0.0 || height <= 0.0) return p;
float3 field = lensShape_1_f3f2f2(_globals, p, size);
float depth = -field.x;
float width = max((abs(field.y) + abs(field.z)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float t = clamp(depth / height, 0.0, 1.0);
float flatBand = float(t < 1.0 ? 0.2929 : 0.0);
float strength = mix(flatBand, 1.0 - sqrt(max(t * (2.0 - t), 0.0)), curvature);
float endpoint = clamp((height - depth) / width + 0.5, 0.0, 1.0);
float2 delta = p - size * 0.5;
float2 radial = float2(delta.x, (delta.y * size.x) / max(size.y, 0.0001));
float2 normal = field.yz;
if (length(radial) > 1e-06) {
float2 blended = mix(normal, normalize(radial), ovalization);
if (length(blended) > 1e-06) normal = normalize(blended);
}
return p + normal * ((amount * strength) * endpoint);
}
float4 lensRefraction_1_f4(thread Globals& _globals) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensControlRefraction_1.z > 0.5) return float4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensControlRefraction_1.xy, 0.0, 0.0);
float shortSide = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.y);
return float4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensInnerRefractionFraction_1.x * shortSide, min(20.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensInnerRefractionFraction_1.y * shortSide), 0.0, 0.0);
}
half4 lensBackdropAt_1_h4f2(thread Globals& _globals, float2 coordinate) {
float2 p = coordinate / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float mask = clamp(0.5 - lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (mask <= 0.0) return LocalMatrix_10_h4f2(_globals, coordinate);
p = lensDisplaced_1_f2f2f2ffff(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensBackdropDisplacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensBackdropDisplacement_1.y, 1.0, 0.5);
return mix(LocalMatrix_10_h4f2(_globals, coordinate), LocalMatrix_10_h4f2(_globals, p * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)), half(mask));
}
float2 nativeSampleXY_1_f2f2f2(thread Globals& _globals, float2 origin, float2 target) {
float2 offset = target - origin;
return (origin + offset * min(1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].maximumSampleRadius_1 / max(length(offset), 0.0001))) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
}
float nativeRadiusLOD_1_ff(thread Globals& _globals, float radius) {
float r = (((max(radius, 0.0) * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativePyramidStretch_1;
float minRadius = (((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeMinimumBlurRadius_1 * 1.6) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.z) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativePyramidStretch_1;
float base = float(minRadius >= 2.0);
r *= float(base > 0.5 ? 0.5 : 1.0);
return base + max(0.0, log2(r < 2.0 ? 1.0 + r * 0.5 : r));
}
float2 nativePyramidPosition_1_f2f2f(thread Globals& _globals, float2 worldXY, float level) {
float divisor = exp2(level);
float width = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.x;
float height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.y;
float2 tile = float2(1.0);
if (level > 0.5) {
tile.x = width + 3.0;
tile.y = tile.y + (height * (1.0 - 2.0 / divisor) + 2.0 * (level - 1.0));
}
float2 size = float2(width, height) / divisor;
float2 p = (worldXY * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidRect_1.xy) / divisor;
return tile + clamp(p, float2(0.5), max(size - 0.5, float2(0.5)));
}
half4 nativePyramidAt_1_h4f2f(thread Globals& _globals, float2 worldXY, float lod) {
lod = clamp(lod, 0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.w);
float first = floor(lod);
float second = min(first + 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.w);
float2 _13_pos = nativePyramidPosition_1_f2f2f(_globals, worldXY, first);
float2 _14_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _13_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _15_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _14_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_15_e = saturate(_15_e + 0.5h);
half4 low = (((_15_e.x * _15_e.y) * _15_e.z) * _15_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _14_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
if (fract(lod) == 0.0 || first == second) return low;
float2 _16_pos = nativePyramidPosition_1_f2f2f(_globals, worldXY, second);
float2 _17_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _16_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _18_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _17_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_18_e = saturate(_18_e + 0.5h);
return mix(low, (((_18_e.x * _18_e.y) * _18_e.z) * _18_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _17_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4)), half(fract(lod)));
}
half4 nativeAtRadius_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
if (radius <= 0.0) return lensBackdropAt_1_h4f2(_globals, nativeSampleXY_1_f2f2f2(_globals, origin, target));
float2 sampleXY = nativeSampleXY_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.w < 0.0) return radius > 0.0 ? LocalMatrix_2_h4f2(_globals, sampleXY) : lensBackdropAt_1_h4f2(_globals, sampleXY);
return nativePyramidAt_1_h4f2f(_globals, sampleXY, nativeRadiusLOD_1_ff(_globals, radius));
}
float3 nativeUnpremul_1_f3h4(half4 p) {
return float3(p.xyz) / max(float(p.w), 1e-06);
}
float nativeLinearRamp_1_ffff(float x, float a, float b) {
return abs(b - a) < 1e-05 ? step(a, x) : clamp((x - a) / (b - a), 0.0, 1.0);
}
float nativeBlurCoverage_1_ff(thread Globals& _globals, float d) {
return ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.x + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.y - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.x) * nativeLinearRamp_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.y)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.z - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.y) * nativeLinearRamp_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.z)) + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.w - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurOpacities_1.z) * nativeLinearRamp_1_ffff(d, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurDistances_1.w);
}
float3 nativeFace_1_f3f3f(thread Globals& _globals, float3 color, float sdf) {
float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
float sdr = 1.0 - step(1.00001, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1);
float maxLuma = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceLuma_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceLuma_1.y, sdr);
float compression = clamp(1.0 + (maxLuma - 1.0) * luma, 0.0, 1.0);
float qLuma = luma * compression;
float3 q = float3(qLuma) + (color * compression - float3(qLuma)) * (1.0 + 0.3 * (1.0 - compression));
float3 result = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceBlack_1 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceTransfer_1.x - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceBlack_1) * qLuma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceTransfer_1.y * (q - float3(qLuma));
result = mix(result, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceFill_1.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceFill_1.w);
return result;
}
float nativeCubic_1_ffff(float t, float p1, float p2) {
float u = 1.0 - t;
return ((((3.0 * u) * u) * t) * p1 + (((3.0 * u) * t) * t) * p2) + (t * t) * t;
}
float nativeTiming_1_fff4f(float x, float4 control, float known) {
if (x <= 0.0 || x >= 1.0) return clamp(x, 0.0, 1.0);
if (known < 0.5) return x;
float low = 0.0;
float high = 1.0;
float t = x;
for (int i = 0;i < 14; ++i) {
float value = nativeCubic_1_ffff(t, control.x, control.z);
if (value < x) low = t; else high = t;
t = (low + high) * 0.5;
}
return nativeCubic_1_ffff(t, control.y, control.w);
}
float4 nativeGradientColor_1_f4f(thread Globals& _globals, float d) {
float4 a = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccent0_1 : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccent1_1;
float4 b = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccent1_1 : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccent2_1;
float lo = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.x : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y;
float hi = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y ? _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.z;
float raw = abs(hi - lo) > 1e-05 ? clamp((d - lo) / (hi - lo), 0.0, 1.0) : step(lo, d);
float t = d < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentDistances_1.y ? nativeTiming_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentTiming0_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentTimingKnown_1.x) : nativeTiming_1_fff4f(raw, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentTiming1_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentTimingKnown_1.y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMaskOptions_1.x > 0.5) {
a.xyz = a.xyz * a.w;
b.xyz = b.xyz * b.w;
float4 c = mix(a, b, t);
c.xyz = c.xyz / max(c.w, 1e-06);
return c;
}
return mix(a, b, t);
}
float nativeBevel_1_ffff(float _0_distance, float height, float radius) {
float t = clamp(-_0_distance / max(height, 1e-06), 0.0, 1.0);
return height > 0.0 ? 1.0 - sqrt(max(t * (2.0 - t), 0.0)) : 0.0;
}
half4 nativeGlassBackgroundBody_1_h4f2ff2f(thread Globals& _globals, float2 xy, float sdf, float2 normal, float radius) {
float inside = nativeBevel_1_ffff(sdf, lensRefraction_1_f4(_globals).y, radius);
float outside = nativeBevel_1_ffff(sdf, lensRefraction_1_f4(_globals).w, radius);
float displacement = lensRefraction_1_f4(_globals).x * inside;
float2 target = xy + normal * displacement;
float effectiveRadius = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.x * max(nativeBlurCoverage_1_ff(_globals, sdf + displacement), 0.0);
nativeAtRadius_1_h4f2f2f(_globals, xy, target, 0.0);
half4 pixel = nativeAtRadius_1_h4f2f2f(_globals, xy, target, effectiveRadius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDispersion_1 > 0.0 && abs(displacement) > 0.001) {
float2 shift = ((normal * abs(displacement)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDispersion_1) * 0.3;
half4 red = nativeAtRadius_1_h4f2f2f(_globals, xy, target + shift, effectiveRadius);
half4 blue = nativeAtRadius_1_h4f2f2f(_globals, xy, target - shift, effectiveRadius);
pixel = half4(red.x, pixel.y, blue.z, pixel.w);
}
float3 rgb = nativeUnpremul_1_f3h4(pixel);
float bodyAlpha = 1.0;
if ((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.x > 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.y > 0.0) || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.z > 0.0) {
half4 fill;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.w < 0.0) {
fill = nativeAtRadius_1_h4f2f2f(_globals, xy, xy, 0.0);
} else {
float lod = nativeRadiusLOD_1_ff(_globals, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.y);
float2 _19_xy = nativeSampleXY_1_f2f2f2(_globals, xy, xy);
float2 world = _19_xy;
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.z, 1e-06));
fill = (nativePyramidAt_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.x);
}
float outerMask = nativeLinearRamp_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask > 0.0) {
float outerShift = lensRefraction_1_f4(_globals).z * outside;
half4 outer = nativeAtRadius_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.x * max(nativeBlurCoverage_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask);
}
float3 processed = nativeFace_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceOpacity_1);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.w > 0.0) {
float2 _20_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.x) * nativeBevel_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.z, radius);
rgb = float3(nativeGlassBleed_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_1_h4f2f2f(_globals, xy, _20_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
half4 lensMaterialAt_1_h4f2(thread Globals& _globals, float2 p) {
float3 field = lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1);
float radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.y) * 0.5;
float shadow = boxRoundedSDF_1_ff2f2f(_globals, (p - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowOffset_1) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1 * 0.5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1 * 0.5, radius);
float ring = boxRoundedSDF_1_ff2f2f(_globals, (p - float2(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadowOffset_1)) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1 * 0.5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1 * 0.5, radius);
half4 background = lensBackdropAt_1_h4f2(_globals, p * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0));
float coverage = clamp(0.5 - field.x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
half4 _60_field;
if (coverage >= 1.0) _60_field = nativeGlassBackgroundBody_1_h4f2ff2f(_globals, p, field.x, field.yz, radius); else {
float _61_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.x * nativeBevel_1_ffff(field.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowShape_1.z, radius);
half4 _62_shadow = nativeAtRadius_1_h4f2f2f(_globals, p, p + field.yz * _61_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.w);
_60_field = nativeGlassExterior_1_h4ffff2h4h4h4(_globals, field.x, shadow, ring, field.yz, background, _62_shadow, background);
if (coverage > 0.0) _60_field = mix(_60_field, nativeGlassBackgroundBody_1_h4f2ff2f(_globals, p, field.x, field.yz, radius), half(coverage));
}
float _63_alpha = clamp(nativeLightingRingAlpha_1_ffff2(_globals, field.x, ring, field.yz), 0.0, 1.0);
half4 _64_foreground = half4(0.0h, 0.0h, 0.0h, half(_63_alpha));
_60_field = _64_foreground + _60_field * (1.0h - _64_foreground.w);
half4 _65_material = _60_field;
half3 _66_exteriorFace = half3(nativeFace_1_f3f3f(_globals, nativeUnpremul_1_f3h4(background), field.x));
float _67_amount = nativeGlassFilterHighlight_1_fff2f(_globals, field.x, field.yz, max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
if (_67_amount > 1e-06) {
float3 _68_raw = float3(background.xyz) / max(float(background.w), 1e-06);
float3 _69_face = float3(_66_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterOptions_1.x > 0.5) _69_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.y < 0.0 ? min(_69_face, _68_raw) : max(_69_face, _68_raw);
float _70_alpha = float(_65_material.w);
float3 _71_color = float3(_65_material.xyz) + (1.0 - _70_alpha) * mix(_68_raw, _69_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceOpacity_1 * _67_amount);
_71_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.y * _67_amount) * (3.0 - 2.0 * _71_color);
float _72_resultAlpha = _70_alpha + ((1.0 - _70_alpha) * _67_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterOptions_1.y;
_65_material = half4(half3(_71_color - (1.0 - _72_resultAlpha) * _68_raw), half(_72_resultAlpha));
}
_60_field = nativeGlassHoldingTone_1_h4fh4(_globals, field.x, _65_material);
float3 _73_fieldColor = nativeLightingClampColor_1_f3f3ff(nativeUnpremul_1_f3h4(_60_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y);
_60_field = half4(half3(_73_fieldColor) * _60_field.w, _60_field.w);
half4 _74_composited = _60_field + background * (1.0h - _60_field.w);
float3 _75_clamped = clamp(nativeUnpremul_1_f3h4(_74_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1)));
_74_composited = half4(half3(_75_clamped) * _74_composited.w, _74_composited.w);
float3 _76_rgb = nativeUnpremul_1_f3h4(_74_composited);
float _77_materialAlpha = float(_74_composited.w);
float _78_inside = nativeBevel_1_ffff(field.x, lensRefraction_1_f4(_globals).y, radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentEnabled_1 > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w > 0.0) {
float4 _79_gradient = nativeGradientColor_1_f4f(_globals, field.x);
float4 _80_matrixInput = float4(_76_rgb, float(_74_composited.w));
float4 _81_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix0_1, _80_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix1_1, _80_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix2_1, _80_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix3_1, _80_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixBias_1;
_81_result.xyz = nativeLightingClampColor_1_f3f3ff(_81_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixOptions_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixOptions_1.y);
float4 _82_accent = _81_result;
_82_accent.w = clamp(_82_accent.w, 0.0, 1.0);
_82_accent.xyz = _82_accent.xyz * _82_accent.w;
float _83_mask = _79_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMaskOptions_1.y > 0.5) _83_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w; else {
_82_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w + _82_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w);
_82_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w + _82_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w);
}
_82_accent *= clamp(_83_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w, 0.0, 1.0);
float _84_outAlpha = _82_accent.w + _77_materialAlpha * (1.0 - _82_accent.w);
_76_rgb = (_82_accent.xyz + (_76_rgb * _77_materialAlpha) * (1.0 - _82_accent.w)) / max(_84_outAlpha, 1e-06);
_77_materialAlpha = _84_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeTintOpacity_1 >= 0.0) {
_76_rgb = mix(_76_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeTintOpacity_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w) * coverage, 0.0, 1.0));
}
_76_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeEdgeAbsorption_1 * _78_inside;
_76_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBrightBackdropShade_1 * smoothstep(0.85, 0.98, dot(_76_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _85_unlit = _76_rgb;
_76_rgb = _85_unlit;
float _86_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDarkSpecularGain_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBrightSpecularGain_1, clamp(dot(_85_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_76_rgb = mix(_85_unlit, _76_rgb, _86_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1 > 0.0 && _78_inside > 0.0) {
float2 _87_tangent = float2(-field.z, field.y) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionSpread_1;
half4 _88_reflected = (nativeAtRadius_1_h4f2f2f(_globals, p, (p + field.yz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1) + _87_tangent, 0.0) + nativeAtRadius_1_h4f2f2f(_globals, p, (p + field.yz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1) - _87_tangent, 0.0)) * 0.5h;
_76_rgb = mix(_76_rgb, nativeUnpremul_1_f3h4(_88_reflected), (_78_inside * _78_inside) * 0.1);
}
float _89_glow = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionIntensity_1, 0.0, 1.0) * coverage;
if (_89_glow > 0.0) {
float _90_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y) * 1.6);
float _91_sheen = 1.0 - smoothstep(0.0, _90_glowRadius, length(p - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionLocation_1));
float _92_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1));
float3 _93_restingResult = _76_rgb;
_76_rgb = mix(_76_rgb, float3(1.0), _89_glow * _92_ambientPressOpacity);
_76_rgb = mix(_76_rgb, float3(1.0), (_89_glow * _91_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1));
float _94_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y));
float _95_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y);
float _96_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 < 0.5) * smoothstep(160.0, 280.0, _95_longSide)) * smoothstep(2.5, 4.0, _95_longSide / _94_shortSide);
float _97_peak = max(_93_restingResult.x, max(_93_restingResult.y, _93_restingResult.z));
float _98_chromaticRange = _97_peak - min(_93_restingResult.x, min(_93_restingResult.y, _93_restingResult.z));
float _99_neutralPress = 1.0 - smoothstep(0.01, 0.1, _98_chromaticRange);
float _100_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1);
float _101_pressGain = mix(1.18 - 0.06 * _97_peak, 1.06, _100_lightAppearance);
float3 _102_colorPress = mix(float3(float(dot(half3(_93_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _93_restingResult, mix(1.03, 1.06, _100_lightAppearance)) * _101_pressGain;
_102_colorPress += float3(_99_neutralPress * mix(0.02745098, 0.04509804, _100_lightAppearance));
_76_rgb = mix(_76_rgb, mix(_93_restingResult, _102_colorPress, _89_glow), _96_platterPress);
}
_76_rgb = nativeLightingClampColor_1_f3f3ff(_76_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y);
float _103_alpha = clamp(_77_materialAlpha, 0.0, 1.0);
return half4(half3(_76_rgb * _103_alpha), half(_103_alpha));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 p = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float3 field = lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1);
half4 original = LocalMatrix_10_h4f2(_globals, pos);
if (1.0 <= 0.0) return original;
half4 base = lensMaterialAt_1_h4f2(_globals, p);
float shortSide = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1.y);
float bodyCoverage = clamp(0.5 - field.x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float4 edgeInputs = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundEdge_1;
edgeInputs.x = edgeInputs.x * shortSide;
float _104_t = abs(edgeInputs.y - edgeInputs.x) > 1e-06 ? clamp((field.x - edgeInputs.x) / (edgeInputs.y - edgeInputs.x), 0.0, 1.0) : step(edgeInputs.x, field.x);
float edge = (1.0 - mix(edgeInputs.z, edgeInputs.w, _104_t)) * bodyCoverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundAberration_1.x > 0.0 && edge > 0.0) {
float _105_angle = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundAberration_1.w;
float2 _106_sourceExtent = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1 + float2(1.41421354 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundCaptureMargin_1);
float _107_c = cos(_105_angle);
float _108_s = sin(_105_angle);
float _109_height = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundAberration_1.y;
float _110_offset = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundAberration_1.z * shortSide;
float _111_t = _109_height > 0.0 ? clamp((-field.x - _110_offset) / _109_height, 0.0, 1.0) : step(_110_offset, -field.x);
float2 spread = ((((float2(field.z * _107_c + field.y * _108_s, field.y * _107_c - field.z * _108_s) * _106_sourceExtent) / max(_106_sourceExtent.yx, float2(0.0001))) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensForegroundAberration_1.x) * shortSide) * (1.0 - sqrt(max(_111_t * (2.0 - _111_t), 0.0)));
float3 rgb = float3(0.0);
float alpha = 0.0;
for (int i = 0;i < 7; ++i) {
float position = 1.0 - float(i) * 0.333333343;
half4 _0_sample = lensMaterialAt_1_h4f2(_globals, p + spread * position);
float3 color = float3(_0_sample.xyz) / max(float(_0_sample.w), 1e-06);
rgb += color * float3(max(position, 0.0), 1.0 - abs(position), max(-position, 0.0));
alpha += float(_0_sample.w) * 0.142857149;
}
rgb *= float3(0.5, 0.333333343, 0.5);
alpha *= edge;
base = half4(half3(rgb * alpha), half(alpha)) + base * (1.0h - half(alpha));
}
half3 _112_backdrop = half3(nativeUnpremul_1_f3h4(base));
float3 _113_color = float3(_112_backdrop);
float2 _114_normal = field.yz;
float _115_norm = length(_114_normal);
if (_115_norm > 1e-05) _114_normal /= _115_norm;
float _116_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.y);
float _117_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.y);
float _118_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.w;
float _119_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.w;
float _120_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.x;
float _121_key = 0.0;
float _122_fill = 0.0;
float _123_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.w > 0.5 ? field.x / max(_115_norm, 1e-06) : field.x;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z > 0.0) {
float _124_primary = nativeLightingSDFBand_1_fffff2(_globals, _123_localSDF, _116_keyHeight, _120_curvature, _114_normal) * nativeLightingSDFDirection_1_ff2ff(_114_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.x, _118_keySpread);
float _125_diffuse = nativeLightingSDFBand_1_fffff2(_globals, _123_localSDF, _116_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.w, 1.0, _114_normal) * nativeLightingSDFDirection_1_ff2ff(_114_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.x, _118_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.z);
_121_key = nativeLightingBias_1_fff(_124_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z) + nativeLightingBias_1_fff(_125_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z > 0.0) {
float _126_primary = nativeLightingSDFBand_1_fffff2(_globals, _123_localSDF, _117_fillHeight, _120_curvature, _114_normal) * nativeLightingSDFDirection_1_ff2ff(_114_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.x, _119_fillSpread);
float _127_diffuse = nativeLightingSDFBand_1_fffff2(_globals, _123_localSDF, _117_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.w, 1.0, _114_normal) * nativeLightingSDFDirection_1_ff2ff(_114_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.x, _119_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.z);
_122_fill = nativeLightingBias_1_fff(_126_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z) + nativeLightingBias_1_fff(_127_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.y);
}
float3 _128_unlitColor = _113_color;
float4 _129_c = float4(_128_unlitColor, 1.0);
float4 _130_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix0_1, _129_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix1_1, _129_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix2_1, _129_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix3_1, _129_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixBias_1;
_130_result.xyz = nativeLightingClampColor_1_f3f3ff(_130_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.y);
float4 _131_transformed = _130_result;
float _132_coverage = _121_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightColor_1.w + _122_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightColor_1.w;
_132_coverage *= clamp(_131_transformed.w, 0.0, 1.0);
_113_color = _128_unlitColor + _132_coverage * (_131_transformed.xyz - _128_unlitColor);
half3 lit = half3(nativeLightingClampColor_1_f3f3ff(_113_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingDisplayHeadroom_1)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y)) * base.w;
base.xyz = mix(base.xyz, lit, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensProgress_1));
float coverage = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 > 0.5 ? 1.0 : clamp(0.5 - field.x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
return field.x > 0.0 ? base : mix(original, base, half(coverage));
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
