#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
float4 color_1;
float4 xform_5;
float4 rect_5;
float4 radiiWithInverse_5;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    half4 SolidColor_1_Var [[user(locn1) center_no_perspective]];
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
};
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = _in.SolidColor_1_Var;
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
float2 _0_a = devCoord;
float4 _1_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].xform_5;
float4 _2_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].rect_5;
float4 _3_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].radiiWithInverse_5;
_0_a = float2x2_from_float4(_1_b) * _0_a;
half4 _4_e = half4(sign(_3_d));
half _5_f = _4_e.x;
half _6_g = 0.5h * (_4_e.y + _4_e.z);
float4 _7_h = abs(_3_d) - 1.0;
float4 _8_i = (_0_a.x + float4(-1.0, 1.0, 1.0, -1.0) * _7_h) - _2_c.xzzx;
float4 _9_j = (_0_a.y + float4(-1.0, -1.0, 1.0, 1.0) * _7_h) - _2_c.yyww;
bool4 _10_k = bool4(_8_i.x >= 0.0 || _9_j.x >= 0.0, _8_i.y <= 0.0 || _9_j.y >= 0.0, _8_i.z <= 0.0 || _9_j.z <= 0.0, _8_i.w >= 0.0 || _9_j.w <= 0.0);
_8_i = select(_8_i, float4(-0.5, 0.0, 0.5, 0.0), _10_k);
_9_j = select(_9_j, float4(0.0, -0.5, 0.0, -0.5), _10_k);
float4 _11_l = _8_i * _1_b.x + _9_j * _1_b.y;
float4 _12_m = _8_i * _1_b.z + _9_j * _1_b.w;
float4 _13_n = 0.5 / (abs(_11_l) + abs(_12_m));
float4 _14_o = select((_7_h * _7_h - _8_i * _8_i) - _9_j * _9_j, float4(_0_a, _2_c.zw) - float4(_2_c.xy, _0_a), _10_k);
half4 _15_p = saturate(0.5h + half4(_13_n * _14_o));
half _16_q = ((_15_p.x * _15_p.y) * _15_p.z) * _15_p.w;
half4 outColor_5 = half4(_6_g + _5_f * _16_q);
outputCoverage *= outColor_5.w;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
