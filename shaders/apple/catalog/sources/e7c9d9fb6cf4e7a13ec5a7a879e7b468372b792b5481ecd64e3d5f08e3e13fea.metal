#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float cornerCurve_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
float contentScale_1;
float2 size_1;
float2 displacement_1;
float ovalization_1;
float sourceScale_1;
float magnification_1;
float4 upper2x2_2;
float2 translation_2;
float4 upper2x2_3;
float2 translation_3;
float2 invImgSize_5;
float4 subset_5;
int tilemodeX_5;
int tilemodeY_5;
int filterMode_5;
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
    sampler2D image_5;
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
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 lensShape_1_f3f2f2(float2 p, float2 size);
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle) {
float2 radial = p - circle.xy;
float2 join = se.zw - circle.xy;
if (radial.x * join.y - radial.y * join.x > 0.0) {
float len = length(radial);
return float3(len - circle.z, radial / max(len, 1e-06));
}
float m = max(max(p.x, p.y), 1e-06);
float norm = m * powr(powr(p.x / m, se.y) + powr(p.y / m, se.y), 1.0 / se.y);
float2 gradient = powr(p / max(norm, 1e-06), float2(se.y - 1.0));
float magnitude = max(length(gradient), 1e-06);
return float3((norm - se.x) / magnitude, gradient / magnitude);
}
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim) {
float2 q = abs(p);
float c = halfDim.x - halfDim.y;
float3 result;
if (q.y + c >= q.x) result = continuousOctant_1_f3f2f4f4(float2(q.x, q.y + c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTop_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTopCircle_1); else {
result = continuousOctant_1_f3f2f4f4(float2(q.y, q.x - c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRight_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRightCircle_1);
result.yz = result.zy;
}
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
result.yz = result.yz * direction;
return result;
}
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
return len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim).yz;
float2 q = (abs(p) - halfDim) + r;
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
if (max(q.x, q.y) > 0.0) {
float2 n = max(q, 0.0);
return (direction * n) / max(length(n), 1e-06);
}
return q.x > q.y ? float2(direction.x, 0.0) : float2(0.0, direction.y);
}
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
float2 normal = len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
return float3(len - r, normal);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _0_q = (abs(p) - halfDim) + r;
return float3((length(max(_0_q, 0.0)) + min(max(_0_q.x, _0_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization) {
if (amount == 0.0 || height <= 0.0) return p;
float3 field = roundedBoxSample_1_f3f2f2f(_globals, p - size * 0.5, size * 0.5, min(size.x, size.y) * 0.5);
float depth = -field.x;
float width = max((abs(field.y) + abs(field.z)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float t = clamp(depth / height, 0.0, 1.0);
float flatBand = float(t < 1.0 ? 0.2929 : 0.0);
float strength = mix(flatBand, 1.0 - sqrt(max(t * (2.0 - t), 0.0)), curvature);
float endpoint = clamp((height - depth) / width + 0.5, 0.0, 1.0);
float2 delta = p - size * 0.5;
float2 radial = float2(delta.x, (delta.y * size.x) / max(size.y, 0.0001));
float2 normal = field.yz;
if (length(radial) > 1e-06) {
float2 blended = mix(normal, normalize(radial), ovalization);
if (length(blended) > 1e-06) normal = normalize(blended);
}
return p + normal * ((amount * strength) * endpoint);
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 _1_size = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1;
float mask = clamp(0.5 - roundedBoxSample_1_f3f2f2f(_globals, pos - _1_size * 0.5, _1_size * 0.5, min(_1_size.x, _1_size.y) * 0.5).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
if (mask <= 0.0) return half4(0.0h);
float2 _0_sample = lensDisplaced_1_f2f2f2ffff(_globals, pos, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].displacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].displacement_1.y, 1.0, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].ovalization_1);
_0_sample = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5 + (_0_sample - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5) / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].magnification_1;
return r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_0_sample * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_5, float2(0.50001), _globals.image_5) * half(mask);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_5_Tex [[texture(0)]], sampler image_5_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_5_Tex, image_5_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
