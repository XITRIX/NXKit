#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
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
    float4 bounds  [[attribute(0)]];
    float depth  [[attribute(1)]];
    uint ssboIndex  [[attribute(2)]];
    float3 mat0  [[attribute(3)]];
    float3 mat1  [[attribute(4)]];
    float3 mat2  [[attribute(5)]];
};
struct Outputs {
    float4 sk_Position [[position]];
    uint ssboIndexVar [[user(locn0) flat]];
    float2 localCoordsVar [[user(locn1)]];
    float sk_PointSize [[point_size]];
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
    constant IntrinsicUniforms* _anonInterface1;
};
float4 cover_bounds_vertex_fn_f4f2f4ff33f2(float2 a, float4 b, float c, float3x3 d, thread float2& e);
template <typename T>matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {T a00 = m[0].x, a01 = m[0].y, a02 = m[0].z,a10 = m[1].x, a11 = m[1].y, a12 = m[1].z,a20 = m[2].x, a21 = m[2].y, a22 = m[2].z,b01 =  a22*a11 - a12*a21,b11 = -a22*a10 + a12*a20,b21 =  a21*a10 - a11*a20,det = a00*b01 + a01*b11 + a02*b21;return matrix<T, 3, 3>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);}float4 cover_bounds_vertex_fn_f4f2f4ff33f2(float2 a, float4 b, float c, float3x3 d, thread float2& e) {
if (all((b.xy <= b.zw))) {
a = mix(b.xy, b.zw, a);
float3 f = d * float3(a, 1.0);
e = a;
return float4(f.xy, c * f.z, f.z);
} else {
a = mix(b.zw, b.xy, a);
float3 f = mat3_inverse(d) * float3(a, 1.0);
float g = 1.0 / f.z;
e = f.xy * g;
return float4(a * g, c * g, g);
}
}
vertex Outputs vertexMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
Globals _globals{&_anonInterface0, &_anonInterface1};
(void)_globals;
Outputs _out;
(void)_out;
    float2 _skTemp0;
    float4 _skTemp1;
float2 stepLocalCoords = float2(0.0);
float4 devPosition = ((_skTemp1 = cover_bounds_vertex_fn_f4f2f4ff33f2(float2(float(sk_VertexID / 2), float(sk_VertexID % 2)), _in.bounds, _in.depth, float3x3(_in.mat0, _in.mat1, _in.mat2), _skTemp0)), (stepLocalCoords = _skTemp0), _skTemp1);
_out.sk_Position = float4(_globals._anonInterface1->viewport.zw * devPosition.xy - sign(_globals._anonInterface1->viewport.zw) * devPosition.ww, devPosition.zw);
_out.localCoordsVar = stepLocalCoords;
_out.ssboIndexVar = _in.ssboIndex;
return _out;
}
