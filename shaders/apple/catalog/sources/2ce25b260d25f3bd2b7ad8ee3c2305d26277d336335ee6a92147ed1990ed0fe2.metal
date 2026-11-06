#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
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
