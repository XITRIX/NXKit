#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
float4 color_1;
float4x4 maskToDevice;
float4x4 localToDevice;
float2 atlasSizeInv;
};
struct Inputs {
    ushort2 size  [[attribute(0)]];
    ushort2 uvPos  [[attribute(1)]];
    float2 xyPos  [[attribute(2)]];
    ushort2 indexAndFlags  [[attribute(3)]];
    float strikeToSourceScale  [[attribute(4)]];
    float depth  [[attribute(5)]];
    uint ssboIndex  [[attribute(6)]];
};
struct Outputs {
    float4 sk_Position [[position]];
    half4 SolidColor_1_Var [[user(locn0) center_no_perspective]];
    float2 textureCoords [[user(locn1)]];
    half texIndex [[user(locn2) flat]];
    half maskFormat [[user(locn3) flat]];
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
float4 text_vertex_fn_f4f2f44f44f2f2f2f2fff2f2f2(float2 a, float4x4 b, float4x4 c, float2 d, float2 e, float2 f, float2 g, float h, float i, thread float2& j, thread float2& k, thread float2& l);
template <typename T>matrix<T, 4, 4> mat4_inverse(matrix<T, 4, 4> m) {T a00 = m[0].x, a01 = m[0].y, a02 = m[0].z, a03 = m[0].w,a10 = m[1].x, a11 = m[1].y, a12 = m[1].z, a13 = m[1].w,a20 = m[2].x, a21 = m[2].y, a22 = m[2].z, a23 = m[2].w,a30 = m[3].x, a31 = m[3].y, a32 = m[3].z, a33 = m[3].w,b00 = a00*a11 - a01*a10,b01 = a00*a12 - a02*a10,b02 = a00*a13 - a03*a10,b03 = a01*a12 - a02*a11,b04 = a01*a13 - a03*a11,b05 = a02*a13 - a03*a12,b06 = a20*a31 - a21*a30,b07 = a20*a32 - a22*a30,b08 = a20*a33 - a23*a30,b09 = a21*a32 - a22*a31,b10 = a21*a33 - a23*a31,b11 = a22*a33 - a23*a32,det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;return matrix<T, 4, 4>(a11*b11 - a12*b10 + a13*b09,a02*b10 - a01*b11 - a03*b09,a31*b05 - a32*b04 + a33*b03,a22*b04 - a21*b05 - a23*b03,a12*b08 - a10*b11 - a13*b07,a00*b11 - a02*b08 + a03*b07,a32*b02 - a30*b05 - a33*b01,a20*b05 - a22*b02 + a23*b01,a10*b10 - a11*b08 + a13*b06,a01*b08 - a00*b10 - a03*b06,a30*b04 - a31*b02 + a33*b00,a21*b02 - a20*b04 - a23*b00,a11*b07 - a10*b09 - a12*b06,a00*b09 - a01*b07 + a02*b06,a31*b01 - a30*b03 - a32*b00,a20*b03 - a21*b01 + a22*b00) * (1/det);}float4 text_vertex_fn_f4f2f44f44f2f2f2f2fff2f2f2(float2 a, float4x4 b, float4x4 c, float2 d, float2 e, float2 f, float2 g, float h, float i, thread float2& j, thread float2& k, thread float2& l) {
a *= e;
float2 m = h * a + g;
float4 n = b * float4(m, 0.0, 1.0);
l = (mat4_inverse(c) * n).xy;
k = a + f;
j = k * d;
return float4(n.xy, i * n.w, n.w);
}
vertex Outputs vertexMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1};
(void)_globals;
Outputs _out;
(void)_out;
    float2 _skTemp0;
    float2 _skTemp1;
    float2 _skTemp2;
    float4 _skTemp3;
float2 stepLocalCoords = float2(0.0);
_globals.uniformSsboIndex = _in.ssboIndex;
float4x4 maskToDevice = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].maskToDevice;
float4x4 localToDevice = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].localToDevice;
float2 atlasSizeInv = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].atlasSizeInv;
_out.texIndex = half(_in.indexAndFlags.x);
_out.maskFormat = half(_in.indexAndFlags.y);
float2 unormTexCoords;
float4 devPosition = ((_skTemp3 = text_vertex_fn_f4f2f44f44f2f2f2f2fff2f2f2(float2(float(sk_VertexID >> 1), float(sk_VertexID & 1)), maskToDevice, localToDevice, atlasSizeInv, float2(_in.size), float2(_in.uvPos), _in.xyPos, _in.strikeToSourceScale, _in.depth, _skTemp0, _skTemp1, _skTemp2)), (_out.textureCoords = _skTemp0), (unormTexCoords = _skTemp1), (stepLocalCoords = _skTemp2), _skTemp3);
_out.sk_Position = float4(_globals._anonInterface1->viewport.zw * devPosition.xy - sign(_globals._anonInterface1->viewport.zw) * devPosition.ww, devPosition.zw);
_out.SolidColor_1_Var = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].color_1);
return _out;
}
