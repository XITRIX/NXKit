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
float4 upper2x2_3;
float2 translation_3;
float2 invImgSize_5;
float4 subset_5;
int tilemodeX_5;
int tilemodeY_5;
int filterMode_5;
float4 upper2x2_8;
float2 translation_8;
float2 invImgSize_10;
float4 subset_10;
int tilemodeX_10;
int tilemodeY_10;
int filterMode_10;
float4 color_13;
float4 xform_17;
float4 rect_17;
float4 radiiWithInverse_17;
};
struct VibrancyProfile_1 {
float3 color;
float3 red;
float3 green;
float3 blue;
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
    sampler2D image_10;
    sampler2D dstSampler;
};
float p_Qfifff(int a, float b, float c, float d);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
half4 r_Qh4f2f2f4iiif2Z(float2 a, float2 b, float4 c, int d, int e, int f, float2 g, sampler2D h);
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
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
float3 lensShape_1_f3f2f2(thread Globals& _globals, float2 p, float2 size);
float lensShoulder_1_ffff(float depth, float height, float offset);
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization);
float lensEdgeOpacity_1_fff4(float sdf, float4 edge);
float2 lensAberration_1_f2f2ff2(float2 normal, float angle, float2 sourceExtent);
float2 lensForegroundExtent_1_f2f2f(float2 size, float margin);
float2 magnifiedPosition_1_f2f2(thread Globals& _globals, float2 p);
float2 sourcePosition_1_f2f2(float2 p);
half4 sourceAt_1_h4f2(float2 p);
half4 displacedSourceAt_1_h4f2(thread Globals& _globals, float2 p);
void spectralSource_1_vf2h3h3(thread Globals& _globals, float2 p, thread half3& ink, thread half3& coverage);
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
float3 lensShape_1_f3f2f2(thread Globals& _globals, float2 p, float2 size) {
return roundedBoxSample_1_f3f2f2f(_globals, p - size * 0.5, size * 0.5, min(size.x, size.y) * 0.5);
}
float lensShoulder_1_ffff(float depth, float height, float offset) {
float t = height > 0.0 ? clamp((depth - offset) / height, 0.0, 1.0) : step(offset, depth);
return 1.0 - sqrt(max(t * (2.0 - t), 0.0));
}
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization) {
if (amount == 0.0 || height <= 0.0) return p;
float3 field = lensShape_1_f3f2f2(_globals, p, size);
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
float2 magnifiedPosition_1_f2f2(thread Globals& _globals, float2 p) {
return _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceMagnification_1 == 1.0 ? p : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5 + (p - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5) / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceMagnification_1;
}
half4 displacedSourceAt_1_h4f2(thread Globals& _globals, float2 p) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bufferedOptics_1 > 0.5) {
return r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_8.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_8.zw) * (p * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_8, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_10, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_10, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_10, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_10, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_10, float2(0.50001), _globals.image_10);
}
float mask = clamp(0.5 - lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float2 _2_p = lensDisplaced_1_f2f2f2ffff(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceDisplacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceDisplacement_1.y, 1.0, 0.0);
float _3_mask = clamp(0.5 - lensShape_1_f3f2f2(_globals, _2_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float2 _4_pos = magnifiedPosition_1_f2f2(_globals, lensDisplaced_1_f2f2f2ffff(_globals, _2_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].wrapperDisplacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].wrapperDisplacement_1.y, 1.0, 0.5)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1;
return (r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _4_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_5, float2(0.50001), _globals.image_5) * half(_3_mask)) * half(mask);
}
void spectralSource_1_vf2h3h3(thread Globals& _globals, float2 p, thread half3& ink, thread half3& coverage) {
float3 field = lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1);
float2 center = p + (field.yz * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundRefraction_1.x) * lensShoulder_1_ffff(-field.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundRefraction_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundRefraction_1.z);
float _5_angle = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.w;
float2 _6_sourceExtent = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 + float2(1.41421354 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundCaptureMargin_1);
float _7_c = cos(_5_angle);
float _8_s = sin(_5_angle);
float2 spread = (((float2(field.z * _7_c + field.y * _8_s, field.y * _7_c - field.z * _8_s) * _6_sourceExtent) / max(_6_sourceExtent.yx, float2(0.0001))) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.x) * lensShoulder_1_ffff(-field.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.y, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.z);
float3 rgb = float3(0.0);
float3 alpha = float3(0.0);
for (int i = 0;i < 7; ++i) {
float position = 1.0 - float(i) * 0.333333343;
half4 _0_sample = displacedSourceAt_1_h4f2(_globals, center + spread * position);
float r = max(position, 0.0);
float b = max(-position, 0.0);
float g = 1.0 - abs(position);
float3 weight = float3(r, g, b) * float3(0.5, 0.333333343, 0.5);
rgb += float3(_0_sample.xyz) * weight;
alpha += float(_0_sample.w) * weight;
}
ink = half3(rgb);
coverage = half3(alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_5_Tex [[texture(0)]], sampler image_5_Smplr [[sampler(0)]], texture2d<half> image_10_Tex [[texture(1)]], sampler image_10_Smplr [[sampler(1)]], texture2d<half> dstSampler_Tex [[texture(2)]], sampler dstSampler_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_5_Tex, image_5_Smplr}, {image_10_Tex, image_10_Smplr}, {dstSampler_Tex, dstSampler_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
    half3 _skTemp4;
    half3 _skTemp5;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _13_p = _in.localCoordsVar;
float2 _14_pos = magnifiedPosition_1_f2f2(_globals, _in.localCoordsVar) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1;
half4 _15_original = r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _14_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_5, float2(0.50001), _globals.image_5);
half3 _16_ink = _15_original.xyz;
half3 _17_coverage = _15_original.www;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1 > 0.0) {
half4 _18_base = displacedSourceAt_1_h4f2(_globals, _13_p);
half3 _19_spectralInk = _18_base.xyz;
half3 _20_spectralCoverage = _18_base.www;
half _21_edge = 0.0h;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.x != 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundRefraction_1.x != 0.0) {
((spectralSource_1_vf2h3h3(_globals, _13_p, _skTemp4, _skTemp5)), (_19_spectralInk = _skTemp4), (_20_spectralCoverage = _skTemp5));
float _22_sdf = lensShape_1_f3f2f2(_globals, _13_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x;
float4 _23_edge = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundEdge_1;
float _24_t = abs(_23_edge.y - _23_edge.x) > 1e-06 ? clamp((_22_sdf - _23_edge.x) / (_23_edge.y - _23_edge.x), 0.0, 1.0) : step(_23_edge.x, _22_sdf);
_21_edge = half(1.0 - mix(_23_edge.z, _23_edge.w, _24_t));
}
half3 _25_color = mix(_18_base.xyz, _19_spectralInk, _21_edge);
half3 _26_alpha = mix(_18_base.www, _20_spectralCoverage, _21_edge);
_16_ink = mix(_15_original.xyz, _25_color, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1));
_17_coverage = mix(_15_original.www, _26_alpha, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1));
}
_16_ink *= half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].opacity_1);
_17_coverage *= half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].opacity_1);
half _27_alpha = max(_17_coverage.x, max(_17_coverage.y, _17_coverage.z));
half4 outColor_1 = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].pass_1 < 0.5 ? half4(_17_coverage, 1.0h) : half4(_16_ink, _27_alpha);
half4 dstColor = sample(_globals.dstSampler, _globals._anonInterface1->dstReadBounds.zw * (float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->dstReadBounds.xy));
half4 outColor_15 = half4(outColor_1.xyz + dstColor.xyz, outColor_1.w + dstColor.w * (1.0h - outColor_1.w));
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
float2 _28_a = devCoord;
float4 _29_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].xform_17;
float4 _30_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].rect_17;
float4 _31_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].radiiWithInverse_17;
_28_a = float2x2_from_float4(_29_b) * _28_a;
half4 _32_e = half4(sign(_31_d));
half _33_f = _32_e.x;
half _34_g = 0.5h * (_32_e.y + _32_e.z);
float4 _35_h = abs(_31_d) - 1.0;
float4 _36_i = (_28_a.x + float4(-1.0, 1.0, 1.0, -1.0) * _35_h) - _30_c.xzzx;
float4 _37_j = (_28_a.y + float4(-1.0, -1.0, 1.0, 1.0) * _35_h) - _30_c.yyww;
bool4 _38_k = bool4(_36_i.x >= 0.0 || _37_j.x >= 0.0, _36_i.y <= 0.0 || _37_j.y >= 0.0, _36_i.z <= 0.0 || _37_j.z <= 0.0, _36_i.w >= 0.0 || _37_j.w <= 0.0);
_36_i = select(_36_i, float4(-0.5, 0.0, 0.5, 0.0), _38_k);
_37_j = select(_37_j, float4(0.0, -0.5, 0.0, -0.5), _38_k);
float4 _39_l = _36_i * _29_b.x + _37_j * _29_b.y;
float4 _40_m = _36_i * _29_b.z + _37_j * _29_b.w;
float4 _41_n = 0.5 / (abs(_39_l) + abs(_40_m));
float4 _42_o = select((_35_h * _35_h - _36_i * _36_i) - _37_j * _37_j, float4(_28_a, _30_c.zw) - float4(_30_c.xy, _28_a), _38_k);
half4 _43_p = saturate(0.5h + half4(_41_n * _42_o));
half _44_q = ((_43_p.x * _43_p.y) * _43_p.z) * _43_p.w;
half4 outColor_17 = half4(_34_g + _33_f * _44_q);
outputCoverage *= outColor_17.w;
if (all((outputCoverage.xyz <= half3(0.0h)))) {
discard_fragment();
}
_out.sk_FragColor = outColor_15 * outputCoverage + dstColor * (1.0h - outputCoverage);
return _out;
}
