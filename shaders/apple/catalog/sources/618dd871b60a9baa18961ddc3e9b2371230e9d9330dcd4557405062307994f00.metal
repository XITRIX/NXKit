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
float4 subset_3;
int tilemodeX_3;
int tilemodeY_3;
int filterMode_3;
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
    half4 sk_SecondaryFragColor [[color(0), index(1)]];
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
float p_Qfifff(int a, float b, float c, float d);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g);
half4 r_Qh4f2f2f4iiif2Z(float2 a, float2 b, float4 c, int d, int e, int f, float2 g, sampler2D h);
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
float p_Qfifff(int a, float b, float c, float d) {
    float _skTemp0;
    float _skTemp1;
    float _skTemp2;
    float _skTemp3;
switch (a) {
case 0:
return clamp(b, c, d);
case 1:
{
float e = d - c;
return (_skTemp0 = b - c, _skTemp1 = e, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1)) + c;
}
case 2:
{
float e = d - c;
float g = 2.0 * e;
float h = (_skTemp2 = b - c, _skTemp3 = g, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3));
return mix(h, g - h, step(e, h)) + c;
}
default:
return b;
}
}
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c) {
return sample(c, a * b);
}
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g) {
    float2 _skTemp4;
    float2 _skTemp5;
    float2 _skTemp6;
    float2 _skTemp7;
((K_Qvf2f22f2f2f2f2((_skTemp4 = a), b, c, e.xy, float2(-1.0), float2(f.x, g.x))), (a = _skTemp4));
((K_Qvf2f22f2f2f2f2((_skTemp5 = a), b, c, e.zy, float2(1.0, -1.0), float2(f.y, g.y))), (a = _skTemp5));
((K_Qvf2f22f2f2f2f2((_skTemp6 = a), b, c, e.zw, float2(1.0), float2(f.z, g.z))), (a = _skTemp6));
((K_Qvf2f22f2f2f2f2((_skTemp7 = a), b, c, e.xw, float2(-1.0, 1.0), float2(f.w, g.w))), (a = _skTemp7));
}
half4 r_Qh4f2f2f4iiif2Z(float2 a, float2 b, float4 c, int d, int e, int f, float2 g, sampler2D h) {
if (d == 3 && f == 0) {
float i = floor(a.x) + 0.5;
if (i < c.x || i > c.z) return half4(0.0h);
}
if (e == 3 && f == 0) {
float i = floor(a.y) + 0.5;
if (i < c.y || i > c.w) return half4(0.0h);
}
a.x = p_Qfifff(d, a.x, c.x, c.z);
a.y = p_Qfifff(e, a.y, c.y, c.w);
float4 i;
if (f == 0) i = float4(floor(c.xy) + 0.50001, ceil(c.zw) - 0.50001); else i = float4(c.xy + g, c.zw - g);
float2 j = clamp(a, i.xy, i.zw);
half4 k = q_Qh4f2f2Z(j, b, h);
if (f == 1) {
half2 l = half2(a - j);
half2 m = abs(l);
bool n = d == 1;
bool o = e == 1;
if (n || o) {
float p;
float q;
half4 r;
half4 t;
if (n) {
p = l.x > 0.0h ? i.x : i.z;
r = q_Qh4f2f2Z(float2(p, j.y), b, h);
}
if (o) {
q = l.y > 0.0h ? i.y : i.w;
t = q_Qh4f2f2Z(float2(j.x, q), b, h);
}
if (n && o) {
half4 u = q_Qh4f2f2Z(float2(p, q), b, h);
k = mix(mix(k, r, m.x), mix(t, u, m.x), m.y);
} else if (n) k = mix(k, r, m.x); else if (o) k = mix(k, t, m.y);
}
if (d == 3) k *= max(1.0h - m.x, 0.0h);
if (e == 3) k *= max(1.0h - m.y, 0.0h);
}
return k;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_3_Tex [[texture(0)]], sampler image_3_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_3_Tex, image_3_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
    float2 _skTemp8;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _10_sk_csxform_prealpha;
half4 _11_a = r_Qh4f2f2f4iiif2Z(_in.LocalMatrix_1_Var, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_3, float2(0.50001), _globals.image_3);
half _12_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_5;
if (_12_b < 0.0h) {
_10_sk_csxform_prealpha = half4(_11_a.xyz / max(_11_a.w, 0.0001h), _11_a.w);
} else {
half _13_c = max(_11_a.w, _12_b - 1.0h);
half _14_d = max(_13_c, 1.0h - _12_b);
_10_sk_csxform_prealpha = half4(_11_a.xyz * _14_d, _13_c);
}
half4 outColor_1 = _10_sk_csxform_prealpha;
half4 outputCoverage = half4(1.0h);
half4 _15_analytic_rrect_coverage_fn;
if (_in.perPixelControl.x > 0.0) _15_analytic_rrect_coverage_fn = half4(1.0h); else if (_in.perPixelControl.y > 1.0) {
float2 _16_h = min(_in.edgeDistances.xy, _in.edgeDistances.zw);
float _17_i = min(_16_h.x, _16_h.y) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _18_j = (_in.perPixelControl.y - 1.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _19_k = 1.0 - 0.5 * _18_j;
_15_analytic_rrect_coverage_fn = half4(half(saturate(_18_j * (_17_i + _19_k))));
} else {
float2x2 _20_h = float2x2_from_float4(_in.jacobian) * (1.0 / float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w);
float2 _21_c = float2(1.0, 0.0) * _20_h;
float2 _22_c = float2(0.0, 1.0) * _20_h;
float2 _23_i = float2(rsqrt(dot(_21_c, _21_c)), rsqrt(dot(_22_c, _22_c)));
float2 _24_j = _23_i * (_in.strokeParams.x + min(_in.edgeDistances.xy, _in.edgeDistances.zw));
float2 _25_k = float2(min(_24_j.x, _24_j.y), -1.0);
float _26_l;
float _27_m;
if (_in.perPixelControl.x > -0.95) {
float2 _28_n = _23_i * ((_in.edgeDistances.xy + _in.edgeDistances.zw) + 2.0 * _in.strokeParams.xx);
_26_l = min(min(_28_n.x, _28_n.y), 1.0);
_27_m = 1.0 - 0.5 * _26_l;
} else {
float2 _29_n = (2.0 * _in.strokeParams.x) * _23_i;
float2 _30_o = _29_n - _24_j;
_25_k.y = -max(_30_o.x, _30_o.y);
if (_in.strokeParams.x > 0.0) {
float _31_p = min(_29_n.x, _29_n.y);
float2 _32_q = select(float2(_31_p), _29_n, (_30_o >= float2(-0.5)));
_26_l = saturate(max(_32_q.x, _32_q.y));
_27_m = 1.0 - 0.5 * _26_l;
} else _26_l = (_27_m = 1.0);
}
((L_Qvf2f22f2f4f4f4((_skTemp8 = _25_k), _20_h, _in.strokeParams, _in.edgeDistances, _in.xRadii, _in.yRadii)), (_25_k = _skTemp8));
float _33_n = min(_in.perPixelControl.y, 0.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _34_o = _26_l * (min(_25_k.x + _33_n, -_25_k.y) + _27_m);
_15_analytic_rrect_coverage_fn = half4(half(saturate(_34_o)));
}
outputCoverage = _15_analytic_rrect_coverage_fn;
_out.sk_FragColor = outColor_1 * outputCoverage;
_out.sk_SecondaryFragColor = outputCoverage;
return _out;
}
