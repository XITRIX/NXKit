#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 upper2x2_1;
float2 translation_1;
float2 invDimensions_3;
half mode_6;
float4 xform_10;
float4 rect_10;
float4 radiiWithInverse_10;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 CoordNormalize_3_Var [[user(locn1)]];
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
    sampler2D image_4;
};
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _10_sk_csxform_prealpha;
half4 _11_a = sample(_globals.image_4, _in.CoordNormalize_3_Var);
half _12_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6;
if (_12_b < 0.0h) {
_10_sk_csxform_prealpha = half4(_11_a.xyz / max(_11_a.w, 0.0001h), _11_a.w);
} else {
half _13_c = max(_11_a.w, _12_b - 1.0h);
half _14_d = max(_13_c, 1.0h - _12_b);
_10_sk_csxform_prealpha = half4(_11_a.xyz * _14_d, _13_c);
}
half4 outColor_1 = _10_sk_csxform_prealpha;
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
float2 _15_a = devCoord;
float4 _16_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].xform_10;
float4 _17_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].rect_10;
float4 _18_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].radiiWithInverse_10;
_15_a = float2x2_from_float4(_16_b) * _15_a;
half4 _19_e = half4(sign(_18_d));
half _20_f = _19_e.x;
half _21_g = 0.5h * (_19_e.y + _19_e.z);
float4 _22_h = abs(_18_d) - 1.0;
float4 _23_i = (_15_a.x + float4(-1.0, 1.0, 1.0, -1.0) * _22_h) - _17_c.xzzx;
float4 _24_j = (_15_a.y + float4(-1.0, -1.0, 1.0, 1.0) * _22_h) - _17_c.yyww;
bool4 _25_k = bool4(_23_i.x >= 0.0 || _24_j.x >= 0.0, _23_i.y <= 0.0 || _24_j.y >= 0.0, _23_i.z <= 0.0 || _24_j.z <= 0.0, _23_i.w >= 0.0 || _24_j.w <= 0.0);
_23_i = select(_23_i, float4(-0.5, 0.0, 0.5, 0.0), _25_k);
_24_j = select(_24_j, float4(0.0, -0.5, 0.0, -0.5), _25_k);
float4 _26_l = _23_i * _16_b.x + _24_j * _16_b.y;
float4 _27_m = _23_i * _16_b.z + _24_j * _16_b.w;
float4 _28_n = 0.5 / (abs(_26_l) + abs(_27_m));
float4 _29_o = select((_22_h * _22_h - _23_i * _23_i) - _24_j * _24_j, float4(_15_a, _17_c.zw) - float4(_17_c.xy, _15_a), _25_k);
half4 _30_p = saturate(0.5h + half4(_28_n * _29_o));
half _31_q = ((_30_p.x * _30_p.y) * _30_p.z) * _30_p.w;
half4 outColor_10 = half4(_21_g + _20_f * _31_q);
outputCoverage *= outColor_10.w;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
