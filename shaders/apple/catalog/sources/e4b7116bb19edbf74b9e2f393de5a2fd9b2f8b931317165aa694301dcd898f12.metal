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
    float2 LocalMatrix_2_Var [[user(locn2)]];
    float2 LocalMatrix_10_Var [[user(locn3)]];
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
    float cornerCurve_1;
    array<float, 4> weights_1;
    array<float, 4> distances_1;
    float2 mergedNormal_1;
    bool geometryBlended_1;
    float4 cornerTop_1;
    float4 cornerTopCircle_1;
    float4 cornerRight_1;
    float4 cornerRightCircle_1;
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_11_h4f2f4(float2 b, float4 c);
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
float2 _4_q = (abs(p) - halfDim) + r;
return float3((length(max(_4_q, 0.0)) + min(max(_4_q.x, _4_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float mergeGeometry_1_ff2b(thread Globals& _globals, float2 xy, bool record) {
float4 _5_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[0];
float2 _6_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].xyz, float3(xy, 1.0))) - _5_g.xy * 0.5;
_globals.cornerCurve_1 = _5_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[0];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[0];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[0];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[0];
float _7_r = min(_5_g.z, min(_5_g.x, _5_g.y) * 0.5);
float3 _8_surface = roundedBoxSample_1_f3f2f2f(_globals, _6_p, _5_g.xy * 0.5, _7_r);
float _9_d = _8_surface.x;
float2 _10_n = _8_surface.yz;
float2 _11_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].x * _10_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].x * _10_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[0].y * _10_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[0].y * _10_n.y);
float _12_magnitude = max(length(_11_gradient), 0.0001);
float3 first = float3(_9_d / _12_magnitude, _11_gradient / _12_magnitude);
float d = first.x;
_globals.mergedNormal_1 = first.yz;
if (record) {
_globals.weights_1[0] = 1.0;
_globals.distances_1[0] = d;
_globals.geometryBlended_1 = false;
}
{
float4 _13_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[1];
float2 _14_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].xyz, float3(xy, 1.0))) - _13_g.xy * 0.5;
_globals.cornerCurve_1 = _13_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[1];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[1];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[1];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[1];
float _15_r = min(_13_g.z, min(_13_g.x, _13_g.y) * 0.5);
float3 _16_surface = roundedBoxSample_1_f3f2f2f(_globals, _14_p, _13_g.xy * 0.5, _15_r);
float _17_d = _16_surface.x;
float2 _18_n = _16_surface.yz;
float2 _19_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].x * _18_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].x * _18_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[1].y * _18_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[1].y * _18_n.y);
float _20_magnitude = max(length(_19_gradient), 0.0001);
float3 item = float3(_17_d / _20_magnitude, _19_gradient / _20_magnitude);
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
float4 _21_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[2];
float2 _22_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].xyz, float3(xy, 1.0))) - _21_g.xy * 0.5;
_globals.cornerCurve_1 = _21_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[2];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[2];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[2];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[2];
float _23_r = min(_21_g.z, min(_21_g.x, _21_g.y) * 0.5);
float3 _24_surface = roundedBoxSample_1_f3f2f2f(_globals, _22_p, _21_g.xy * 0.5, _23_r);
float _25_d = _24_surface.x;
float2 _26_n = _24_surface.yz;
float2 _27_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].x * _26_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].x * _26_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[2].y * _26_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[2].y * _26_n.y);
float _28_magnitude = max(length(_27_gradient), 0.0001);
float3 item = float3(_25_d / _28_magnitude, _27_gradient / _28_magnitude);
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
float4 _29_g = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].geometry_1[3];
float2 _30_p = float2(dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].xyz, float3(xy, 1.0)), dot(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].xyz, float3(xy, 1.0))) - _29_g.xy * 0.5;
_globals.cornerCurve_1 = _29_g.w;
_globals.cornerTop_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTop_1[3];
_globals.cornerTopCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerTopCircle_1[3];
_globals.cornerRight_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRight_1[3];
_globals.cornerRightCircle_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_cornerRightCircle_1[3];
float _31_r = min(_29_g.z, min(_29_g.x, _29_g.y) * 0.5);
float3 _32_surface = roundedBoxSample_1_f3f2f2f(_globals, _30_p, _29_g.xy * 0.5, _31_r);
float _33_d = _32_surface.x;
float2 _34_n = _32_surface.yz;
float2 _35_gradient = float2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].x * _34_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].x * _34_n.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseX_1[3].y * _34_n.x + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].inverseY_1[3].y * _34_n.y);
float _36_magnitude = max(length(_35_gradient), 0.0001);
float3 item = float3(_33_d / _36_magnitude, _35_gradient / _36_magnitude);
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_15_Tex [[texture(1)]], sampler image_15_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_15_Tex, image_15_Smplr}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _47_xy = _in.localCoordsVar / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
mergeGeometry_1_ff2b(_globals, _47_xy, true);
if (_globals.geometryBlended_1) {
float _48_epsilon = max(0.05, 0.25 / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0));
mergeGeometry_1_ff2b(_globals, _47_xy + float2(_48_epsilon, 0.0), false) - mergeGeometry_1_ff2b(_globals, _47_xy - float2(_48_epsilon, 0.0), false);
mergeGeometry_1_ff2b(_globals, _47_xy + float2(0.0, _48_epsilon), false) - mergeGeometry_1_ff2b(_globals, _47_xy - float2(0.0, _48_epsilon), false);
}
{
float _53_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] > 1.5) _53_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _53_cover;
_globals.weights_1[1] *= 1.0 - _53_cover;
_globals.weights_1[2] *= 1.0 - _53_cover;
_globals.weights_1[3] *= 1.0 - _53_cover;
_globals.weights_1[0] += _53_cover;
}
{
float _54_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] > 1.5) _54_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _54_cover;
_globals.weights_1[1] *= 1.0 - _54_cover;
_globals.weights_1[2] *= 1.0 - _54_cover;
_globals.weights_1[3] *= 1.0 - _54_cover;
_globals.weights_1[1] += _54_cover;
}
{
float _55_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] > 1.5) _55_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _55_cover;
_globals.weights_1[1] *= 1.0 - _55_cover;
_globals.weights_1[2] *= 1.0 - _55_cover;
_globals.weights_1[3] *= 1.0 - _55_cover;
_globals.weights_1[2] += _55_cover;
}
{
float _56_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] > 1.5) _56_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _56_cover;
_globals.weights_1[1] *= 1.0 - _56_cover;
_globals.weights_1[2] *= 1.0 - _56_cover;
_globals.weights_1[3] *= 1.0 - _56_cover;
_globals.weights_1[3] += _56_cover;
}
{
float _57_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[0] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[0] <= 1.5) _57_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _57_cover;
_globals.weights_1[1] *= 1.0 - _57_cover;
_globals.weights_1[2] *= 1.0 - _57_cover;
_globals.weights_1[3] *= 1.0 - _57_cover;
_globals.weights_1[0] += _57_cover;
}
{
float _58_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[1] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[1] <= 1.5) _58_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _58_cover;
_globals.weights_1[1] *= 1.0 - _58_cover;
_globals.weights_1[2] *= 1.0 - _58_cover;
_globals.weights_1[3] *= 1.0 - _58_cover;
_globals.weights_1[1] += _58_cover;
}
{
float _59_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[2] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[2] <= 1.5) _59_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _59_cover;
_globals.weights_1[1] *= 1.0 - _59_cover;
_globals.weights_1[2] *= 1.0 - _59_cover;
_globals.weights_1[3] *= 1.0 - _59_cover;
_globals.weights_1[2] += _59_cover;
}
{
float _60_cover = smoothstep(0.0, 1.0, 1.1875 - _globals.distances_1[3] * 0.125);
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].material_tintMode_1[3] <= 1.5) _60_cover = 0.0;
_globals.weights_1[0] *= 1.0 - _60_cover;
_globals.weights_1[1] *= 1.0 - _60_cover;
_globals.weights_1[2] *= 1.0 - _60_cover;
_globals.weights_1[3] *= 1.0 - _60_cover;
_globals.weights_1[3] += _60_cover;
}
half4 _61_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _in.LocalMatrix_2_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_61_e = saturate(_61_e + 0.5h);
half4 _62_original = (((_61_e.x * _61_e.y) * _61_e.z) * _61_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _in.LocalMatrix_2_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
float _63_weight = 0.0;
_63_weight += _globals.weights_1[0] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[0];
_63_weight += _globals.weights_1[1] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[1];
_63_weight += _globals.weights_1[2] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[2];
_63_weight += _globals.weights_1[3] * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].itemAlpha_1[3];
half4 _64_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _in.LocalMatrix_10_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_64_e = saturate(_64_e + 0.5h);
half4 outColor_1 = _62_original * half(1.0 - _63_weight) + (((_64_e.x * _64_e.y) * _64_e.z) * _64_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _in.LocalMatrix_10_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12));
_out.sk_FragColor = outColor_1;
return _out;
}
