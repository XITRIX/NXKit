#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
float cornerCurve_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
float contentScale_1;
float bufferedOptics_1;
float3x3 samplingToOwner_1;
float sourceScale_1;
float dark_1;
float2 size_1;
float pass_1;
float progress_1;
float selection_1;
float opacity_1;
float sourceMagnification_1;
float2 wrapperDisplacement_1;
float2 sourceDisplacement_1;
float4 foregroundRefraction_1;
float4 foregroundAberration_1;
float4 foregroundEdge_1;
float foregroundCaptureMargin_1;
float4 upper2x2_2;
float2 translation_2;
float2 invImgSize_4;
float4 subset_4;
int tilemodeX_4;
int tilemodeY_4;
int filterMode_4;
float4 upper2x2_7;
float2 translation_7;
float2 invImgSize_9;
float4 subset_9;
int tilemodeX_9;
int tilemodeY_9;
int filterMode_9;
float4 upper2x2_12;
float2 translation_12;
float2 invImgSize_14;
float4 subset_14;
int tilemodeX_14;
int tilemodeY_14;
int filterMode_14;
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
