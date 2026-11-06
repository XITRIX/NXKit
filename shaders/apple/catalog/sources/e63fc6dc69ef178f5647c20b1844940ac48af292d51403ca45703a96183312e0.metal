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
half4 LocalMatrix_9_h4f2(thread Globals& _globals, float2 pos) {
float2 _13_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _14_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _13_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_14_e = saturate(_14_e + 0.5h);
float4 _15_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
return (((_14_e.x * _14_e.y) * _14_e.z) * _14_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _13_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _15_c.xy, _15_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
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
float nativeGlassOutputCoverage_1_ff(thread Globals& _globals, float sdf);
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
float2 _16_q = (abs(p) - halfDim) + r;
return float3((length(max(_16_q, 0.0)) + min(max(_16_q.x, _16_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float nativeGlassOutputCoverage_1_ff(thread Globals& _globals, float sdf) {
float aa = 0.5 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0);
return smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeOutputDistance_1.x - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeOutputDistance_1.x + aa, sdf) * (1.0 - smoothstep(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeOutputDistance_1.y - aa, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeOutputDistance_1.y + aa, sdf));
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
float outputMask = nativeGlassOutputCoverage_1_ff(_globals, sdf);
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
float3 _17_c = float3(bleedBackdrop.xyz) / max(float(bleedBackdrop.w), 1e-06);
float _18_black = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.x;
float _19_luma = dot(_17_c, float3(0.2126, 0.7152, 0.0722));
float3 bleed = float3(_18_black + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.y - _18_black) * _19_luma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedTransfer_1.z * (_17_c - float3(_19_luma));
color = mix(color, bleed, clamp(gain * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.w, 0.0, 1.0));
}
return half3(color);
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
float2 _23_pos = nativePyramidPosition_1_f2f2f(_globals, worldXY, first);
float2 _24_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _23_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _25_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _24_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_25_e = saturate(_25_e + 0.5h);
float4 _26_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
half4 low = (((_25_e.x * _25_e.y) * _25_e.z) * _25_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _24_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _26_c.xy, _26_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
if (fract(lod) == 0.0 || first == second) return low;
float2 _27_pos = nativePyramidPosition_1_f2f2f(_globals, worldXY, second);
float2 _28_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _27_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _29_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _28_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_29_e = saturate(_29_e + 0.5h);
float4 _30_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
return mix(low, (((_29_e.x * _29_e.y) * _29_e.z) * _29_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _28_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _30_c.xy, _30_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6), half(fract(lod)));
}
half4 nativeAtRadius_1_h4f2f2f(thread Globals& _globals, float2 origin, float2 target, float radius) {
float2 sampleXY = nativeSampleXY_1_f2f2f2(_globals, origin, target);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.w < 0.0) return radius > 0.0 ? LocalMatrix_2_h4f2(_globals, sampleXY) : LocalMatrix_9_h4f2(_globals, sampleXY);
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
float inside = nativeBevel_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefraction_1.y, radius);
float outside = nativeBevel_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefraction_1.w, radius);
float displacement = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefraction_1.x * inside;
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
float2 _31_xy = nativeSampleXY_1_f2f2f2(_globals, xy, xy);
float2 world = _31_xy;
float2 offset = float2((lod * 0.25) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropPyramidInfo_1.z, 1e-06));
fill = (nativePyramidAt_1_h4f2f(_globals, world + offset, lod) + nativePyramidAt_1_h4f2f(_globals, world - offset, lod)) * 0.5h;
}
float3 fillRGB = nativeUnpremul_1_f3h4(fill);
rgb = (((1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.y) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.z) * rgb + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.y * max(rgb, fillRGB)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.z * min(rgb, fillRGB);
rgb = mix(rgb, fillRGB, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillOpacity_1.x);
}
float outerMask = nativeLinearRamp_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.y);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask > 0.0) {
float outerShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefraction_1.z * outside;
half4 outer = nativeAtRadius_1_h4f2f2f(_globals, xy, xy + normal * outerShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.x * max(nativeBlurCoverage_1_ff(_globals, sdf + outerShift), 0.0));
rgb = mix(rgb, float3(outer.xyz), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask);
bodyAlpha = mix(bodyAlpha, float(outer.w), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefractionMask_1.z * outerMask);
}
float3 processed = nativeFace_1_f3f3f(_globals, rgb, sdf);
rgb = mix(rgb, processed, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceOpacity_1);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.w > 0.0) {
float2 _32_target = xy + (normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.x) * nativeBevel_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBleedShape_1.z, radius);
rgb = float3(nativeGlassBleed_1_h3fh3h4(_globals, sdf, half3(rgb), nativeAtRadius_1_h4f2f2f(_globals, xy, _32_target, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.z)));
}
return half4(half3(rgb), half(bodyAlpha));
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 xy = pos / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float2 halfDim = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1 * 0.5;
float radius = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRadius_1, min(halfDim.x, halfDim.y));
float3 _0_sample = roundedBoxSample_1_f3f2f2f(_globals, xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1, halfDim, radius);
float sdf = _0_sample.x;
float2 normal = _0_sample.yz;
float2 _112_pos = nativeSampleXY_1_f2f2f2(_globals, xy, xy);
float2 _113_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_9.zw) * _112_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_9;
half4 _114_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_10 - _113_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_114_e = saturate(_114_e + 0.5h);
float4 _115_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_13;
half4 original = (((_114_e.x * _114_e.y) * _114_e.z) * _114_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * _113_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11, _115_c.xy, _115_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_13, _globals.image_13);
if (nativeGlassOutputCoverage_1_ff(_globals, sdf) <= 0.0) return original;
float shiftedShadow = boxRoundedSDF_1_ff2f2f(_globals, (xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowOffset_1) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1, halfDim, radius);
float shiftedRing = boxRoundedSDF_1_ff2f2f(_globals, (xy - float2(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRingShadowOffset_1)) - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensCenter_1, halfDim, radius);
float coverage = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].externalShapeCoverage_1 > 0.5 ? 1.0 : clamp(0.5 - sdf * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
half4 _116_field;
if (coverage >= 1.0) _116_field = nativeGlassBackgroundBody_1_h4f2ff2f(_globals, xy, sdf, normal, radius); else {
float _117_shadowShift = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.x * nativeBevel_1_ffff(sdf + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowResponse_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeShadowShape_1.z, radius);
half4 _118_shadow = nativeAtRadius_1_h4f2f2f(_globals, xy, xy + normal * _117_shadowShift, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBlurRadii_1.w);
_116_field = nativeGlassExterior_1_h4ffff2h4h4h4(_globals, sdf, shiftedShadow, shiftedRing, normal, original, _118_shadow, original);
if (coverage > 0.0) _116_field = mix(_116_field, nativeGlassBackgroundBody_1_h4f2ff2f(_globals, xy, sdf, normal, radius), half(coverage));
}
float _119_alpha = clamp(nativeLightingRingAlpha_1_ffff2(_globals, sdf, shiftedRing, normal), 0.0, 1.0);
half4 _120_foreground = half4(0.0h, 0.0h, 0.0h, half(_119_alpha));
_116_field = _120_foreground + _116_field * (1.0h - _120_foreground.w);
half4 _121_material = _116_field;
half3 _122_exteriorFace = half3(nativeFace_1_f3f3f(_globals, nativeUnpremul_1_f3h4(original), sdf));
float _123_amount = nativeGlassFilterHighlight_1_fff2f(_globals, sdf, normal, max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
if (_123_amount > 1e-06) {
float3 _124_raw = float3(original.xyz) / max(float(original.w), 1e-06);
float3 _125_face = float3(_122_exteriorFace);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterOptions_1.x > 0.5) _125_face = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.y < 0.0 ? min(_125_face, _124_raw) : max(_125_face, _124_raw);
float _126_alpha = float(_121_material.w);
float3 _127_color = float3(_121_material.xyz) + (1.0 - _126_alpha) * mix(_124_raw, _125_face, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFaceOpacity_1 * _123_amount);
_127_color *= 1.0 + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterHighlightColor_1.y * _123_amount) * (3.0 - 2.0 * _127_color);
float _128_resultAlpha = _126_alpha + ((1.0 - _126_alpha) * _123_amount) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFilterOptions_1.y;
_121_material = half4(half3(_127_color - (1.0 - _128_resultAlpha) * _124_raw), half(_128_resultAlpha));
}
_116_field = nativeGlassHoldingTone_1_h4fh4(_globals, sdf, _121_material);
float3 _129_fieldColor = nativeLightingClampColor_1_f3f3ff(nativeUnpremul_1_f3h4(_116_field), min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y);
_116_field = half4(half3(_129_fieldColor) * _116_field.w, _116_field.w);
half4 _130_composited = _116_field + original * (1.0h - _116_field.w);
float3 _131_clamped = clamp(nativeUnpremul_1_f3h4(_130_composited), float3(0.0), float3(min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1)));
_130_composited = half4(half3(_131_clamped) * _130_composited.w, _130_composited.w);
float3 _132_rgb = nativeUnpremul_1_f3h4(_130_composited);
float _133_materialAlpha = float(_130_composited.w);
float _134_inside = nativeBevel_1_ffff(sdf, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeRefraction_1.y, radius);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentEnabled_1 > 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w > 0.0) {
float4 _135_gradient = nativeGradientColor_1_f4f(_globals, sdf);
float4 _136_matrixInput = float4(_132_rgb, float(_130_composited.w));
float4 _137_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix0_1, _136_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix1_1, _136_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix2_1, _136_matrixInput), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrix3_1, _136_matrixInput)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixBias_1;
_137_result.xyz = nativeLightingClampColor_1_f3f3ff(_137_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixOptions_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMatrixOptions_1.y);
float4 _138_accent = _137_result;
_138_accent.w = clamp(_138_accent.w, 0.0, 1.0);
_138_accent.xyz = _138_accent.xyz * _138_accent.w;
float _139_mask = _135_gradient.w * coverage;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentMaskOptions_1.y > 0.5) _139_mask *= _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w; else {
_138_accent.xyz = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.xyz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w + _138_accent.xyz * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w);
_138_accent.w = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w + _138_accent.w * (1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeAccentFill_1.w);
}
_138_accent *= clamp(_139_mask * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w, 0.0, 1.0);
float _140_outAlpha = _138_accent.w + _133_materialAlpha * (1.0 - _138_accent.w);
_132_rgb = (_138_accent.xyz + (_132_rgb * _133_materialAlpha) * (1.0 - _138_accent.w)) / max(_140_outAlpha, 1e-06);
_133_materialAlpha = _140_outAlpha;
} else if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeTintOpacity_1 >= 0.0) {
_132_rgb = mix(_132_rgb, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.xyz, clamp((_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeTintOpacity_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tint_1.w) * coverage, 0.0, 1.0));
}
_132_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeEdgeAbsorption_1 * _134_inside;
_132_rgb *= 1.0 - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBrightBackdropShade_1 * smoothstep(0.85, 0.98, dot(_132_rgb, float3(0.2126, 0.7152, 0.0722)));
float3 _141_unlit = _132_rgb;
float3 _142_color = float3(half3(_132_rgb));
float2 _143_normal = normal;
float _144_norm = length(_143_normal);
if (_144_norm > 1e-05) _143_normal /= _144_norm;
float _145_keyHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.y);
float _146_fillHeight = max(0.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.y * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.y);
float _147_keySpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightShape_1.w;
float _148_fillSpread = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.w * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.z + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightShape_1.w;
float _149_curvature = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.x;
float _150_key = 0.0;
float _151_fill = 0.0;
float _152_localSDF = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.w > 0.5 ? sdf / max(_144_norm, 1e-06) : sdf;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z > 0.0) {
float _153_primary = nativeLightingSDFBand_1_fffff2(_globals, _152_localSDF, _145_keyHeight, _149_curvature, _143_normal) * nativeLightingSDFDirection_1_ff2ff(_143_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.x, _147_keySpread);
float _154_diffuse = nativeLightingSDFBand_1_fffff2(_globals, _152_localSDF, _145_keyHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.w, 1.0, _143_normal) * nativeLightingSDFDirection_1_ff2ff(_143_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.x, _147_keySpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.z);
_150_key = nativeLightingBias_1_fff(_153_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z) + nativeLightingBias_1_fff(_154_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlight_1.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.y);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z > 0.0) {
float _155_primary = nativeLightingSDFBand_1_fffff2(_globals, _152_localSDF, _146_fillHeight, _149_curvature, _143_normal) * nativeLightingSDFDirection_1_ff2ff(_143_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.x, _148_fillSpread);
float _156_diffuse = nativeLightingSDFBand_1_fffff2(_globals, _152_localSDF, _146_fillHeight * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.w, 1.0, _143_normal) * nativeLightingSDFDirection_1_ff2ff(_143_normal, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.x, _148_fillSpread * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.z);
_151_fill = nativeLightingBias_1_fff(_155_primary, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z) + nativeLightingBias_1_fff(_156_diffuse, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlight_1.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightResponse_1.y);
}
float3 _157_unlitColor = _142_color;
float4 _158_c = float4(_157_unlitColor, 1.0);
float4 _159_result = float4(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix0_1, _158_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix1_1, _158_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix2_1, _158_c), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrix3_1, _158_c)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixBias_1;
_159_result.xyz = nativeLightingClampColor_1_f3f3ff(_159_result.xyz, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeHighlightMatrixOptions_1.y);
float4 _160_transformed = _159_result;
float _161_coverage = _150_key * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeKeyHighlightColor_1.w + _151_fill * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeFillHighlightColor_1.w;
_161_coverage *= clamp(_160_transformed.w, 0.0, 1.0);
_142_color = _157_unlitColor + _161_coverage * (_160_transformed.xyz - _157_unlitColor);
_132_rgb = float3(half3(nativeLightingClampColor_1_f3f3ff(_142_color, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.z, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingDisplayHeadroom_1)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y)));
float _162_specularGain = mix(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDarkSpecularGain_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeBrightSpecularGain_1, clamp(dot(_141_unlit, float3(0.2126, 0.7152, 0.0722)), 0.0, 1.0));
_132_rgb = mix(_141_unlit, _132_rgb, _162_specularGain);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1 > 0.0 && _134_inside > 0.0) {
float2 _163_tangent = float2(-normal.y, normal.x) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionSpread_1;
half4 _164_reflected = (nativeAtRadius_1_h4f2f2f(_globals, xy, (xy + normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1) + _163_tangent, 0.0) + nativeAtRadius_1_h4f2f2f(_globals, xy, (xy + normal * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeReflectionDistance_1) - _163_tangent, 0.0)) * 0.5h;
_132_rgb = mix(_132_rgb, nativeUnpremul_1_f3h4(_164_reflected), (_134_inside * _134_inside) * 0.1);
}
float _165_glow = clamp(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionIntensity_1, 0.0, 1.0) * coverage;
if (_165_glow > 0.0) {
float _166_glowRadius = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y) * 1.6);
float _167_sheen = 1.0 - smoothstep(0.0, _166_glowRadius, length(xy - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].interactionLocation_1));
float _168_ambientPressOpacity = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 > 0.5 ? mix(0.14, 0.22, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1) : (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 > 1.5 ? 0.3 : mix(0.385, 0.47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1));
float3 _169_restingResult = _132_rgb;
_132_rgb = mix(_132_rgb, float3(1.0), _165_glow * _168_ambientPressOpacity);
_132_rgb = mix(_132_rgb, float3(1.0), (_165_glow * _167_sheen) * mix(0.07, 0.1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1));
float _170_shortSide = max(1.0, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y));
float _171_longSide = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].lensSize_1.y);
float _172_platterPress = (float(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bodyMode_1 < 0.5 && _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tintMode_1 < 0.5) * smoothstep(160.0, 280.0, _171_longSide)) * smoothstep(2.5, 4.0, _171_longSide / _170_shortSide);
float _173_peak = max(_169_restingResult.x, max(_169_restingResult.y, _169_restingResult.z));
float _174_chromaticRange = _173_peak - min(_169_restingResult.x, min(_169_restingResult.y, _169_restingResult.z));
float _175_neutralPress = 1.0 - smoothstep(0.01, 0.1, _174_chromaticRange);
float _176_lightAppearance = smoothstep(0.15, 0.85, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].backdropLuma_1);
float _177_pressGain = mix(1.18 - 0.06 * _173_peak, 1.06, _176_lightAppearance);
float3 _178_colorPress = mix(float3(float(dot(half3(_169_restingResult), half3(0.2126h, 0.7152h, 0.0722h)))), _169_restingResult, mix(1.03, 1.06, _176_lightAppearance)) * _177_pressGain;
_178_colorPress += float3(_175_neutralPress * mix(0.02745098, 0.04509804, _176_lightAppearance));
_132_rgb = mix(_132_rgb, mix(_169_restingResult, _178_colorPress, _165_glow), _172_platterPress);
}
_132_rgb = nativeLightingClampColor_1_f3f3ff(_132_rgb, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeDisplayHeadroom_1), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].nativeLightingClamp_1.y);
float _179_alpha = clamp(_133_materialAlpha, 0.0, 1.0);
return half4(half3(_132_rgb * _179_alpha), half(_179_alpha));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}, {image_13_Tex, image_13_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
