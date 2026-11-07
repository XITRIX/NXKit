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
    sampler2D image_4;
    sampler2D image_9;
    sampler2D image_14;
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
float3 lensShape_1_f3f2f2(thread Globals& _globals, float2 p, float2 size);
float lensShoulder_1_ffff(float depth, float height, float offset);
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization);
float lensEdgeOpacity_1_fff4(float sdf, float4 edge);
float2 lensAberration_1_f2f2ff2(float2 normal, float angle, float2 sourceExtent);
float2 lensForegroundExtent_1_f2f2f(float2 size, float margin);
VibrancyProfile_1 glassVibrancyProfile_1_Sif(int index, float dark);
float3 glassVibrantColor_1_f3f3f3f(float3 source, float3 backdrop, float dark);
float2 magnifiedPosition_1_f2f2(thread Globals& _globals, float2 p);
float2 sourcePosition_1_f2f2(float2 p);
half4 sourceAt_1_h4f2(float2 p);
half4 displacedSourceAt_1_h4f2(thread Globals& _globals, float2 p);
void spectralSource_1_vf2h3h3(thread Globals& _globals, float2 p, thread half3& ink, thread half3& coverage);
half4 vibrantSample_1_h4h4h4(thread Globals& _globals, half4 source, half4 background);
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
VibrancyProfile_1 glassVibrancyProfile_1_Sif(int index, float dark) {
if (dark > 0.5) {
if (index == 0) return VibrancyProfile_1{float3(0.0, 0.5686275, 1.0), float3(0.0, 0.2107248, 0.3800659), float3(0.0, 0.2107248, 0.3800659), float3(0.0)};
if (index == 1) return VibrancyProfile_1{float3(1.0, 0.258823544, 0.270588249), float3(0.0), float3(0.32347694, 0.211944342, 0.09062167), float3(0.323005259, 0.08458066, 0.2186681)};
if (index == 2) return VibrancyProfile_1{float3(0.188235283, 0.819607854, 0.345098048), float3(0.1994913, 0.390331775, 0.1566663), float3(0.233333334), float3(0.08631088, 0.388334274, 0.272034436)};
if (index == 3) return VibrancyProfile_1{float3(1.0, 0.572549045, 0.188235283), float3(0.0), float3(0.299836248, 0.258912325, 0.05683626), float3(0.3011428, 0.161486253, 0.15295577)};
if (index == 4) return VibrancyProfile_1{float3(0.858823538, 0.203921571, 0.9490196), float3(0.2), float3(0.19874233, 0.160554364, 0.219681889), float3(0.2)};
if (index == 5) return VibrancyProfile_1{float3(0.0, 0.854901969, 0.7647059), float3(0.00172205, 0.321627378, 0.292235434), float3(0.00619937, 0.287332952, 0.306467682), float3(0.00619937, 0.287332952, 0.306467682)};
if (index == 6) return VibrancyProfile_1{float3(1.0, 0.8392157, 0.0), float3(0.0), float3(0.2906894, 0.18384549, 0.02546509), float3(0.2927568, 0.242898285, 0.00167605)};
if (index == 7) return VibrancyProfile_1{float3(0.427450985, 0.4862745, 1.0), float3(0.297529042, 0.101149991, 0.200958535), float3(0.08821759, 0.308806449, 0.2026135), float3(0.0)};
if (index == 8) return VibrancyProfile_1{float3(0.5568628, 0.5568628, 0.5764706), float3(0.476666331, 0.03529412, 0.0), float3(0.01117033, 0.4894184, 0.01137174), float3(0.01278176, 0.00259291, 0.4965858)};
if (index == 9) return VibrancyProfile_1{float3(0.0, 0.8235294, 0.8784314), float3(0.0, 0.283472449, 0.3030032), float3(0.01470588, 0.0, 0.5852941), float3(0.01470588, 0.0, 0.5852941)};
if (index == 10) return VibrancyProfile_1{float3(0.235294119, 0.827451, 0.996078432), float3(0.168287843, 0.1877903, 0.218188733), float3(0.03246073, 0.5675393, 0.0), float3(0.0)};
if (index == 11) return VibrancyProfile_1{float3(1.0, 0.215686277, 0.372549027), float3(0.0), float3(0.330855876, 0.186006814, 0.119381137), float3(0.327807218, 0.08000098, 0.228435621)};
} else {
if (index == 0) return VibrancyProfile_1{float3(0.0, 0.5333333, 1.0), float3(0.0), float3(0.188537076, 0.08752048, 0.00067591), float3(0.248925239, 0.111727111, 0.0)};
if (index == 1) return VibrancyProfile_1{float3(1.0, 0.219607845, 0.235294119), float3(0.111066379, 0.152225479, 0.148758426), float3(0.001323, 0.196158469, 0.09649339), float3(0.0, 0.09550075, 0.202062517)};
if (index == 2) return VibrancyProfile_1{float3(0.203921571, 0.78039217, 0.349019617), float3(0.23107639, 0.01368712, 0.09310206), float3(0.144280955, 0.169322267, 0.116310887), float3(0.109381847, 0.02611154, 0.223712385)};
if (index == 3) return VibrancyProfile_1{float3(1.0, 0.5529412, 0.156862751), float3(0.07934714, 0.126527935, 0.231570467), float3(0.00531327, 0.168690011, 0.1852025), float3(0.0, 0.07393069, 0.217244267)};
if (index == 4) return VibrancyProfile_1{float3(0.796078444, 0.188235283, 0.8784314), float3(0.164866433, 0.244075343, 0.03335873), float3(0.03591966, 0.276895016, 0.01401958), float3(0.061134, 0.258695364, 0.139712527)};
if (index == 5) return VibrancyProfile_1{float3(0.0, 0.784313738, 0.7019608), float3(0.0), float3(0.236050814, 0.04724378, 0.07591119), float3(0.221293181, 0.05389692, 0.06409784)};
if (index == 6) return VibrancyProfile_1{float3(1.0, 0.8, 0.0), float3(0.00255841, 0.05035659, 0.293904364), float3(0.00564985, 0.03789729, 0.270345539), float3(0.0)};
if (index == 7) return VibrancyProfile_1{float3(0.380392164, 0.333333343, 0.9607843), float3(0.27021715, 0.117750026, 0.00148882), float3(0.0985833, 0.280795932, 0.00522178), float3(0.146758646, 0.146640778, 0.186682731)};
if (index == 8) return VibrancyProfile_1{float3(0.5568628, 0.5568628, 0.5764706), float3(0.483724773, 0.01236094, 0.0039143), float3(0.00906469, 0.483621746, 0.00731356), float3(0.0065925, 0.01081582, 0.4825917)};
if (index == 9) return VibrancyProfile_1{float3(0.0, 0.7647059, 0.8156863), float3(0.0), float3(0.2444208, 0.05653706, 0.04586151), float3(0.2531273, 0.0511426, 0.0500809)};
if (index == 10) return VibrancyProfile_1{float3(0.0, 0.7529412, 0.9098039), float3(0.0), float3(0.245320365, 0.05762778, 0.02395339), float3(0.2727216, 0.06053179, 0.02595239)};
if (index == 11) return VibrancyProfile_1{float3(1.0, 0.1764706, 0.333333343), float3(0.09013781, 0.178013921, 0.148753539), float3(0.0, 0.200364247, 0.08010181), float3(0.0, 0.116937637, 0.190211356)};
}
return VibrancyProfile_1{float3(0.0), float3(0.0), float3(0.0), float3(0.0)};
}
float3 glassVibrantColor_1_f3f3f3f(float3 source, float3 backdrop, float dark) {
source = clamp(source, 0.0, 1.0);
backdrop = clamp(backdrop, 0.0, 1.0);
float3 change = float3(0.0);
float total = 0.0;
float3 underlay = dark > 0.5 ? backdrop : 1.0 - backdrop;
for (int i = 0;i < 12; ++i) {
VibrancyProfile_1 profile = glassVibrancyProfile_1_Sif(i, dark);
float3 delta = source - profile.color;
float _0_distance = dot(delta, delta);
float3 value = float3(dot(profile.red, underlay), dot(profile.green, underlay), dot(profile.blue, underlay));
if (_0_distance < 1e-06) return clamp(source + (dark > 0.5 ? value : -value), 0.0, 1.0);
float weight = 1.0 / (_0_distance * _0_distance);
change += value * weight;
total += weight;
}
change /= max(total, 0.0001);
return clamp(source + (dark > 0.5 ? change : -change), 0.0, 1.0);
}
float2 magnifiedPosition_1_f2f2(thread Globals& _globals, float2 p) {
return _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceMagnification_1 == 1.0 ? p : _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5 + (p - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5) / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceMagnification_1;
}
half4 displacedSourceAt_1_h4f2(thread Globals& _globals, float2 p) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bufferedOptics_1 > 0.5) {
return r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_7.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_7.zw) * (p * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_9, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_9, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_9, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_9, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_9, float2(0.50001), _globals.image_9);
}
float mask = clamp(0.5 - lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float2 _2_p = lensDisplaced_1_f2f2f2ffff(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceDisplacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceDisplacement_1.y, 1.0, 0.0);
float _3_mask = clamp(0.5 - lensShape_1_f3f2f2(_globals, _2_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0, 1.0);
float2 _4_pos = magnifiedPosition_1_f2f2(_globals, lensDisplaced_1_f2f2f2ffff(_globals, _2_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].wrapperDisplacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].wrapperDisplacement_1.y, 1.0, 0.5)) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1;
return (r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _4_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_4, float2(0.50001), _globals.image_4) * half(_3_mask)) * half(mask);
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
half4 vibrantSample_1_h4h4h4(thread Globals& _globals, half4 source, half4 background) {
if (source.w <= 0.0h) return half4(0.0h);
half3 color = half3(glassVibrantColor_1_f3f3f3f(float3(source.xyz / max(source.w, 0.0001h)), float3(background.xyz / max(background.w, 0.0001h)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dark_1));
return half4(mix(source.xyz, color * source.w, background.w), source.w);
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
    half3 _skTemp4;
    half3 _skTemp5;
half4 background = half4(0.0h);
{
background = r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_14, float2(0.50001), _globals.image_14);
pos = (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].samplingToOwner_1 * float3(pos, 1.0)).xy;
}
float2 p = pos;
float2 _9_pos = magnifiedPosition_1_f2f2(_globals, pos) * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].sourceScale_1;
half4 original = r_Qh4f2f2f4iiif2Z(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _9_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subset_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeX_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemodeY_4, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterMode_4, float2(0.50001), _globals.image_4);
original = vibrantSample_1_h4h4h4(_globals, original, background);
half3 ink = original.xyz;
half3 coverage = original.www;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1 > 0.0) {
half4 base = displacedSourceAt_1_h4f2(_globals, p);
half3 spectralInk = base.xyz;
half3 spectralCoverage = base.www;
half edge = 0.0h;
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundAberration_1.x != 0.0 || _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundRefraction_1.x != 0.0) {
((spectralSource_1_vf2h3h3(_globals, p, _skTemp4, _skTemp5)), (spectralInk = _skTemp4), (spectralCoverage = _skTemp5));
float _10_sdf = lensShape_1_f3f2f2(_globals, p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1).x;
float4 _11_edge = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].foregroundEdge_1;
float _12_t = abs(_11_edge.y - _11_edge.x) > 1e-06 ? clamp((_10_sdf - _11_edge.x) / (_11_edge.y - _11_edge.x), 0.0, 1.0) : step(_11_edge.x, _10_sdf);
edge = half(1.0 - mix(_11_edge.z, _11_edge.w, _12_t));
}
half3 color = mix(base.xyz, spectralInk, edge);
half3 alpha = mix(base.www, spectralCoverage, edge);
{
half3 straight = color / max(alpha, half3(0.0001h));
color = mix(color, half3(glassVibrantColor_1_f3f3f3f(float3(straight), float3(background.xyz / max(background.w, 0.0001h)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dark_1)) * alpha, background.w);
}
ink = mix(original.xyz, color, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1));
coverage = mix(original.www, alpha, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].progress_1));
}
ink *= half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].opacity_1);
coverage *= half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].opacity_1);
half alpha = max(coverage.x, max(coverage.y, coverage.z));
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].pass_1 > 1.5) return half4(ink + background.xyz * (half3(1.0h) - coverage), alpha + background.w * (1.0h - alpha));
return _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].pass_1 < 0.5 ? half4(coverage, 1.0h) : half4(ink, alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], texture2d<half> image_9_Tex [[texture(1)]], sampler image_9_Smplr [[sampler(1)]], texture2d<half> image_14_Tex [[texture(2)]], sampler image_14_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}, {image_9_Tex, image_9_Smplr}, {image_14_Tex, image_14_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
