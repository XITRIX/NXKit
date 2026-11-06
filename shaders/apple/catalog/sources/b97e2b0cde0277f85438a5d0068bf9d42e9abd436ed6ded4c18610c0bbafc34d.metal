#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const int kMaxLoopLimit_1 = 2;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<half4, 14> offsetsAndKernel_1;
half2 dir_1;
float4 upper2x2_2;
float2 translation_2;
float2 invImgSize_4;
float4 subset_4;
int tilemodeX_4;
int tilemodeY_4;
int filterMode_4;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 localCoordsVar [[user(locn1)]];
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
float p_Qfifff(int a, float b, float c, float d);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
half4 r_Qh4f2f2f4iiif2Z(float2 a, float2 b, float4 c, int d, int e, int f, float2 g, sampler2D h);
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _0_sum = half4(0.0h);
for (int _1_i = 0;_1_i < kMaxLoopLimit_1; ++_1_i) {
half4 _2_s = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].offsetsAndKernel_1[_1_i];
_0_sum += _2_s.y * r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_in.localCoordsVar + float2(_2_s.x * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dir_1)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_4, float2(0.50001), _globals.image_4);
_0_sum += _2_s.w * r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_in.localCoordsVar + float2(_2_s.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dir_1)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_4, float2(0.50001), _globals.image_4);
}
half4 outColor_1 = _0_sum;
_out.sk_FragColor = outColor_1;
return _out;
}
