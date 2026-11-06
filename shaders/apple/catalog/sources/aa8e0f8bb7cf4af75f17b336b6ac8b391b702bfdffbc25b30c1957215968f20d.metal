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
float dark_7;
};
struct VibrancyProfile_7 {
float3 color;
float3 red;
float3 green;
float3 blue;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 LocalMatrix_1_Var [[user(locn1)]];
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
    sampler2D dstSampler;
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
VibrancyProfile_7 glassVibrancyProfile_7_Sif(int index, float dark);
float3 glassVibrantColor_7_f3f3f3f(float3 source, float3 backdrop, float dark);
VibrancyProfile_7 glassVibrancyProfile_7_Sif(int index, float dark) {
if (dark > 0.5) {
if (index == 0) return VibrancyProfile_7{float3(0.0, 0.5686275, 1.0), float3(0.0, 0.2107248, 0.3800659), float3(0.0, 0.2107248, 0.3800659), float3(0.0)};
if (index == 1) return VibrancyProfile_7{float3(1.0, 0.258823544, 0.270588249), float3(0.0), float3(0.32347694, 0.211944342, 0.09062167), float3(0.323005259, 0.08458066, 0.2186681)};
if (index == 2) return VibrancyProfile_7{float3(0.188235283, 0.819607854, 0.345098048), float3(0.1994913, 0.390331775, 0.1566663), float3(0.233333334), float3(0.08631088, 0.388334274, 0.272034436)};
if (index == 3) return VibrancyProfile_7{float3(1.0, 0.572549045, 0.188235283), float3(0.0), float3(0.299836248, 0.258912325, 0.05683626), float3(0.3011428, 0.161486253, 0.15295577)};
if (index == 4) return VibrancyProfile_7{float3(0.858823538, 0.203921571, 0.9490196), float3(0.2), float3(0.19874233, 0.160554364, 0.219681889), float3(0.2)};
if (index == 5) return VibrancyProfile_7{float3(0.0, 0.854901969, 0.7647059), float3(0.00172205, 0.321627378, 0.292235434), float3(0.00619937, 0.287332952, 0.306467682), float3(0.00619937, 0.287332952, 0.306467682)};
if (index == 6) return VibrancyProfile_7{float3(1.0, 0.8392157, 0.0), float3(0.0), float3(0.2906894, 0.18384549, 0.02546509), float3(0.2927568, 0.242898285, 0.00167605)};
if (index == 7) return VibrancyProfile_7{float3(0.427450985, 0.4862745, 1.0), float3(0.297529042, 0.101149991, 0.200958535), float3(0.08821759, 0.308806449, 0.2026135), float3(0.0)};
if (index == 8) return VibrancyProfile_7{float3(0.5568628, 0.5568628, 0.5764706), float3(0.476666331, 0.03529412, 0.0), float3(0.01117033, 0.4894184, 0.01137174), float3(0.01278176, 0.00259291, 0.4965858)};
if (index == 9) return VibrancyProfile_7{float3(0.0, 0.8235294, 0.8784314), float3(0.0, 0.283472449, 0.3030032), float3(0.01470588, 0.0, 0.5852941), float3(0.01470588, 0.0, 0.5852941)};
if (index == 10) return VibrancyProfile_7{float3(0.235294119, 0.827451, 0.996078432), float3(0.168287843, 0.1877903, 0.218188733), float3(0.03246073, 0.5675393, 0.0), float3(0.0)};
if (index == 11) return VibrancyProfile_7{float3(1.0, 0.215686277, 0.372549027), float3(0.0), float3(0.330855876, 0.186006814, 0.119381137), float3(0.327807218, 0.08000098, 0.228435621)};
} else {
if (index == 0) return VibrancyProfile_7{float3(0.0, 0.5333333, 1.0), float3(0.0), float3(0.188537076, 0.08752048, 0.00067591), float3(0.248925239, 0.111727111, 0.0)};
if (index == 1) return VibrancyProfile_7{float3(1.0, 0.219607845, 0.235294119), float3(0.111066379, 0.152225479, 0.148758426), float3(0.001323, 0.196158469, 0.09649339), float3(0.0, 0.09550075, 0.202062517)};
if (index == 2) return VibrancyProfile_7{float3(0.203921571, 0.78039217, 0.349019617), float3(0.23107639, 0.01368712, 0.09310206), float3(0.144280955, 0.169322267, 0.116310887), float3(0.109381847, 0.02611154, 0.223712385)};
if (index == 3) return VibrancyProfile_7{float3(1.0, 0.5529412, 0.156862751), float3(0.07934714, 0.126527935, 0.231570467), float3(0.00531327, 0.168690011, 0.1852025), float3(0.0, 0.07393069, 0.217244267)};
if (index == 4) return VibrancyProfile_7{float3(0.796078444, 0.188235283, 0.8784314), float3(0.164866433, 0.244075343, 0.03335873), float3(0.03591966, 0.276895016, 0.01401958), float3(0.061134, 0.258695364, 0.139712527)};
if (index == 5) return VibrancyProfile_7{float3(0.0, 0.784313738, 0.7019608), float3(0.0), float3(0.236050814, 0.04724378, 0.07591119), float3(0.221293181, 0.05389692, 0.06409784)};
if (index == 6) return VibrancyProfile_7{float3(1.0, 0.8, 0.0), float3(0.00255841, 0.05035659, 0.293904364), float3(0.00564985, 0.03789729, 0.270345539), float3(0.0)};
if (index == 7) return VibrancyProfile_7{float3(0.380392164, 0.333333343, 0.9607843), float3(0.27021715, 0.117750026, 0.00148882), float3(0.0985833, 0.280795932, 0.00522178), float3(0.146758646, 0.146640778, 0.186682731)};
if (index == 8) return VibrancyProfile_7{float3(0.5568628, 0.5568628, 0.5764706), float3(0.483724773, 0.01236094, 0.0039143), float3(0.00906469, 0.483621746, 0.00731356), float3(0.0065925, 0.01081582, 0.4825917)};
if (index == 9) return VibrancyProfile_7{float3(0.0, 0.7647059, 0.8156863), float3(0.0), float3(0.2444208, 0.05653706, 0.04586151), float3(0.2531273, 0.0511426, 0.0500809)};
if (index == 10) return VibrancyProfile_7{float3(0.0, 0.7529412, 0.9098039), float3(0.0), float3(0.245320365, 0.05762778, 0.02395339), float3(0.2727216, 0.06053179, 0.02595239)};
if (index == 11) return VibrancyProfile_7{float3(1.0, 0.1764706, 0.333333343), float3(0.09013781, 0.178013921, 0.148753539), float3(0.0, 0.200364247, 0.08010181), float3(0.0, 0.116937637, 0.190211356)};
}
return VibrancyProfile_7{float3(0.0), float3(0.0), float3(0.0), float3(0.0)};
}
float3 glassVibrantColor_7_f3f3f3f(float3 source, float3 backdrop, float dark) {
source = clamp(source, 0.0, 1.0);
backdrop = clamp(backdrop, 0.0, 1.0);
float3 change = float3(0.0);
float total = 0.0;
float3 underlay = dark > 0.5 ? backdrop : 1.0 - backdrop;
for (int i = 0;i < 12; ++i) {
VibrancyProfile_7 profile = glassVibrancyProfile_7_Sif(i, dark);
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
half4 RuntimeEffect_7_h4h4h4(thread Globals& _globals, half4 inColor, half4 destColor) {
if (inColor.w <= 0.0h) return destColor;
half3 color = half3(glassVibrantColor_7_f3f3f3f(float3(inColor.xyz / max(inColor.w, 0.0001h)), float3(destColor.xyz / max(destColor.w, 0.0001h)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dark_7));
half3 ink = mix(inColor.xyz, color * inColor.w, destColor.w);
return half4(ink + destColor.xyz * (1.0h - inColor.w), inColor.w + destColor.w * (1.0h - inColor.w));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_3_Tex [[texture(0)]], sampler image_3_Smplr [[sampler(0)]], texture2d<half> dstSampler_Tex [[texture(1)]], sampler dstSampler_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_3_Tex, image_3_Smplr}, {dstSampler_Tex, dstSampler_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
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
half4 dstColor = sample(_globals.dstSampler, _globals._anonInterface1->dstReadBounds.zw * (float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->dstReadBounds.xy));
half4 outColor_7 = RuntimeEffect_7_h4h4h4(_globals, outColor_1, dstColor);
_out.sk_FragColor = outColor_7;
return _out;
}
