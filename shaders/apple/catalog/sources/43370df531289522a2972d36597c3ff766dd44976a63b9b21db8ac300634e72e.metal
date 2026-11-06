#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 upper2x2_1;
float2 translation_1;
float2 invImgSize_3;
float4 subsetInsetClamp_3;
half mode_5;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 LocalMatrix_1_Var [[user(locn1)]];
    float4 jacobian [[user(locn2)]];
    float4 edgeDistances [[user(locn3)]];
    float4 xRadii [[user(locn4) flat]];
    float4 yRadii [[user(locn5) flat]];
    float2 strokeParams [[user(locn6) flat]];
    float2 perPixelControl [[user(locn7)]];
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
    sampler2D image_3;
};
void K_Qvf2f22f2f2f2f2(thread float2& a, float2x2 b, float2 c, float2 d, float2 e, float2 f);
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
}
void K_Qvf2f22f2f2f2f2(thread float2& a, float2x2 b, float2 c, float2 d, float2 e, float2 f) {
float2 g = f - d;
if (all((g > float2(0.0)))) if (all((f > float2(0.0))) || c.x > 0.0 && c.y < 0.0) {
float2 _83_a = g * e;
float2 _84_e = 1.0 / (f * f + c.x * c.x);
float2 _85_g = _84_e * _83_a;
float2 _86_c = _85_g * b;
float _87_h = rsqrt(dot(_86_c, _86_c));
float _88_i = (0.5 * _87_h) * (dot(_83_a, _85_g) - 1.0);
float _89_j = ((f.x * c.x) * _84_e.x) * _87_h;
float2 h = float2(_89_j - _88_i, _89_j + _88_i);
h.y = f.x - c.x <= 0.0 ? 1.0 : -h.y;
a = min(a, h);
} else if (c.y == 0.0) {
float2 _90_c = e * b;
float h = ((c.x - g.x) - g.y) * rsqrt(dot(_90_c, _90_c));
a.x = min(a.x, h);
}
}
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g) {
    float2 _skTemp0;
    float2 _skTemp1;
    float2 _skTemp2;
    float2 _skTemp3;
((K_Qvf2f22f2f2f2f2((_skTemp0 = a), b, c, e.xy, float2(-1.0), float2(f.x, g.x))), (a = _skTemp0));
((K_Qvf2f22f2f2f2f2((_skTemp1 = a), b, c, e.zy, float2(1.0, -1.0), float2(f.y, g.y))), (a = _skTemp1));
((K_Qvf2f22f2f2f2f2((_skTemp2 = a), b, c, e.zw, float2(1.0), float2(f.z, g.z))), (a = _skTemp2));
((K_Qvf2f22f2f2f2f2((_skTemp3 = a), b, c, e.xw, float2(-1.0, 1.0), float2(f.w, g.w))), (a = _skTemp3));
}
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c) {
return sample(c, a * b);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_3_Tex [[texture(0)]], sampler image_3_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_3_Tex, image_3_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
    float2 _skTemp4;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float4 _12_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_3;
half4 _13_sk_csxform_prealpha;
half4 _14_a = q_Qh4f2f2Z(clamp(_in.LocalMatrix_1_Var, _12_c.xy, _12_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_3, _globals.image_3);
half _15_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_5;
if (_15_b < 0.0h) {
_13_sk_csxform_prealpha = half4(_14_a.xyz / max(_14_a.w, 0.0001h), _14_a.w);
} else {
half _16_c = max(_14_a.w, _15_b - 1.0h);
half _17_d = max(_16_c, 1.0h - _15_b);
_13_sk_csxform_prealpha = half4(_14_a.xyz * _17_d, _16_c);
}
half4 outColor_1 = _13_sk_csxform_prealpha;
half4 outputCoverage = half4(1.0h);
half4 _18_analytic_rrect_coverage_fn;
if (_in.perPixelControl.x > 0.0) _18_analytic_rrect_coverage_fn = half4(1.0h); else if (_in.perPixelControl.y > 1.0) {
float2 _19_h = min(_in.edgeDistances.xy, _in.edgeDistances.zw);
float _20_i = min(_19_h.x, _19_h.y) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _21_j = (_in.perPixelControl.y - 1.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _22_k = 1.0 - 0.5 * _21_j;
_18_analytic_rrect_coverage_fn = half4(half(saturate(_21_j * (_20_i + _22_k))));
} else {
float2x2 _23_h = float2x2_from_float4(_in.jacobian) * (1.0 / float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w);
float2 _24_c = float2(1.0, 0.0) * _23_h;
float2 _25_c = float2(0.0, 1.0) * _23_h;
float2 _26_i = float2(rsqrt(dot(_24_c, _24_c)), rsqrt(dot(_25_c, _25_c)));
float2 _27_j = _26_i * (_in.strokeParams.x + min(_in.edgeDistances.xy, _in.edgeDistances.zw));
float2 _28_k = float2(min(_27_j.x, _27_j.y), -1.0);
float _29_l;
float _30_m;
if (_in.perPixelControl.x > -0.95) {
float2 _31_n = _26_i * ((_in.edgeDistances.xy + _in.edgeDistances.zw) + 2.0 * _in.strokeParams.xx);
_29_l = min(min(_31_n.x, _31_n.y), 1.0);
_30_m = 1.0 - 0.5 * _29_l;
} else {
float2 _32_n = (2.0 * _in.strokeParams.x) * _26_i;
float2 _33_o = _32_n - _27_j;
_28_k.y = -max(_33_o.x, _33_o.y);
if (_in.strokeParams.x > 0.0) {
float _34_p = min(_32_n.x, _32_n.y);
float2 _35_q = select(float2(_34_p), _32_n, (_33_o >= float2(-0.5)));
_29_l = saturate(max(_35_q.x, _35_q.y));
_30_m = 1.0 - 0.5 * _29_l;
} else _29_l = (_30_m = 1.0);
}
((L_Qvf2f22f2f4f4f4((_skTemp4 = _28_k), _23_h, _in.strokeParams, _in.edgeDistances, _in.xRadii, _in.yRadii)), (_28_k = _skTemp4));
float _36_n = min(_in.perPixelControl.y, 0.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _37_o = _29_l * (min(_28_k.x + _36_n, -_28_k.y) + _30_m);
_18_analytic_rrect_coverage_fn = half4(half(saturate(_37_o)));
}
outputCoverage = _18_analytic_rrect_coverage_fn;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
