#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
float4 color_1;
float3x3 maskToDeviceRemainder;
};
struct Inputs {
    float4 drawBounds  [[attribute(0)]];
    float4 maskBoundsIn  [[attribute(1)]];
    float2 deviceOrigin  [[attribute(2)]];
    float depth  [[attribute(3)]];
    uint ssboIndex  [[attribute(4)]];
    float3 mat0  [[attribute(5)]];
    float3 mat1  [[attribute(6)]];
    float3 mat2  [[attribute(7)]];
};
struct Outputs {
    float4 sk_Position [[position]];
    half4 SolidColor_1_Var [[user(locn0) center_no_perspective]];
    float4 maskBounds [[user(locn1)]];
    float2 textureCoords [[user(locn2)]];
    half invert [[user(locn3)]];
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
    uint uniformSsboIndex;
    constant IntrinsicUniforms* _anonInterface1;
};
float4 coverage_mask_vertex_fn_f4f2f33f4f4f2ff33f4f2hf2(float2 a, float3x3 b, float4 c, float4 d, float2 e, float f, float3x3 g, thread float4& h, thread float2& i, thread half& j, thread float2& k);
template <typename T>matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {T a00 = m[0].x, a01 = m[0].y, a02 = m[0].z,a10 = m[1].x, a11 = m[1].y, a12 = m[1].z,a20 = m[2].x, a21 = m[2].y, a22 = m[2].z,b01 =  a22*a11 - a12*a21,b11 = -a22*a10 + a12*a20,b21 =  a21*a10 - a11*a20,det = a00*b01 + a01*b11 + a02*b21;return matrix<T, 3, 3>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);}float4 coverage_mask_vertex_fn_f4f2f33f4f4f2ff33f4f2hf2(float2 a, float3x3 b, float4 c, float4 d, float2 e, float f, float3x3 g, thread float4& h, thread float2& i, thread half& j, thread float2& k) {
i = mix(c.xy, c.zw, a);
float3 l = b * float3(i + e, 1.0);
float3 m = mat3_inverse(g) * l;
k = m.xy / m.z;
if (all((d.xy <= d.zw))) {
h = d;
j = 0.0h;
} else {
h = d.zwxy;
j = 1.0h;
}
return float4(l.xy, f * l.z, l.z);
}
vertex Outputs vertexMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1};
(void)_globals;
Outputs _out;
(void)_out;
    float4 _skTemp0;
    float2 _skTemp1;
    half _skTemp2;
    float2 _skTemp3;
    float4 _skTemp4;
float2 stepLocalCoords = float2(0.0);
_globals.uniformSsboIndex = _in.ssboIndex;
float3x3 maskToDeviceRemainder = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].maskToDeviceRemainder;
float4 devPosition = ((_skTemp4 = coverage_mask_vertex_fn_f4f2f33f4f4f2ff33f4f2hf2(float2(float(sk_VertexID >> 1), float(sk_VertexID & 1)), maskToDeviceRemainder, _in.drawBounds, _in.maskBoundsIn, _in.deviceOrigin, _in.depth, float3x3(_in.mat0, _in.mat1, _in.mat2), _skTemp0, _skTemp1, _skTemp2, _skTemp3)), (_out.maskBounds = _skTemp0), (_out.textureCoords = _skTemp1), (_out.invert = _skTemp2), (stepLocalCoords = _skTemp3), _skTemp4);
_out.sk_Position = float4(_globals._anonInterface1->viewport.zw * devPosition.xy - sign(_globals._anonInterface1->viewport.zw) * devPosition.ww, devPosition.zw);
_out.SolidColor_1_Var = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].color_1);
return _out;
}
