#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float2 axis_1;
float radius_1;
float4 upper2x2_2;
float2 translation_2;
float4 decalBounds_3;
float4 upper2x2_4;
float2 translation_4;
float2 invDimensions_6;
float4 upper2x2_10;
float2 translation_10;
array<float4, 8> colors_12;
array<float4, 2> offsets_12;
int tilemode_12;
int colorSpace_12;
int doUnPremul_12;
half mode_13;
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
    sampler2D image_7;
};
half3 a_Qh3h3(half3 a);
half3 css_hsl_to_srgb_Qh3h3(half3 b);
half3 css_lab_to_xyz_Qh3h3(half3 a);
half3 css_oklab_gamut_map_to_linear_srgb_Qh3h3(half3 a);
half3 css_oklab_to_linear_srgb_Qh3h3(half3 a);
half4 interpolated_to_rgb_unpremul_Qh4h4ii(half4 a, int b, int c);
float2 v_Qf2if2(int a, float2 b);
half4 x_Qh4f4f4f2(array<float4, 8> a, array<float4, 2> b, float2 c);
half3 a_Qh3h3(half3 a) {
return half3(a.z, a.y * cos(((a.x) * 0.0174532925)), a.y * sin(((a.x) * 0.0174532925)));
}
half3 css_hsl_to_srgb_Qh3h3(half3 b) {
    half _skTemp0;
    half _skTemp1;
    half3 _skTemp2;
    half _skTemp3;
b.x = (_skTemp0 = b.x, _skTemp1 = 360.0h, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1));
if (b.x < 0.0h) b.x = b.x + 360.0h;
b.yz = b.yz * 0.01h;
half3 c = (_skTemp2 = half3(0.0h, 8.0h, 4.0h) + b.x * 0.0333333351h, _skTemp3 = 12.0h, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3));
half d = b.y * min(b.z, 1.0h - b.z);
return b.z - d * clamp(min(c - 3.0h, 9.0h - c), -1.0h, 1.0h);
}
half3 css_lab_to_xyz_Qh3h3(half3 a) {
half3 d;
d.y = (a.x + 16.0h) * 0.00862069h;
d.x = a.y * 0.002h + d.y;
d.z = d.y - a.z * 0.005h;
half3 g = powr(d, half3(3.0h));
half3 h = half3(g.x > 0.008856452h ? g.x : (116.0h * d.x - 16.0h) * 0.00110705639h, a.x > 8.000001h ? g.y : a.x * 0.00110705639h, g.z > 0.008856452h ? g.z : (116.0h * d.z - 16.0h) * 0.00110705639h);
return h * half3(0.9642956h, 1.0h, 0.825104535h);
}
half3 css_oklab_gamut_map_to_linear_srgb_Qh3h3(half3 a) {
const half2 r = half2(-0.014804h, -0.162608h);
const half2 s = half2(-0.276786h, 0.004193h);
half2 z = a.yz;
half A;
half2 B;
half2 C;
if (dot(z, half2(0.409702h, -0.912219h)) < 0.0h) if (dot(z, half2(0.460276h, 0.887776h)) < 0.0h) if (dot(z, half2(-0.171122h, 0.98525h)) < 0.0h) {
A = 0.102047h;
B = r;
C = s;
} else {
A = 0.092029h;
B = half2(-0.038533h, -0.00165h);
C = half2(-0.232572h, -0.094331h);
} else if (dot(z, half2(0.947925h, 0.318495h)) < 0.0h) {
A = 0.081709h;
B = half2(-0.034601h, -0.002215h);
C = half2(0.012185h, 0.338031h);
} else {
A = 0.091132h;
B = half2(0.07037h, 0.034139h);
C = half2(0.01817h, 0.37855h);
} else if (dot(z, half2(-0.9068h, 0.421562h)) < 0.0h) if (dot(z, half2(-0.397919h, -0.917421h)) < 0.0h) {
A = 0.113902h;
B = half2(0.090836h, 0.036251h);
C = half2(0.226781h, 0.018764h);
} else {
A = 0.161739h;
B = half2(-0.008202h, -0.264819h);
C = half2(0.187156h, -0.284304h);
} else {
A = 0.102047h;
B = r;
C = s;
}
half D = 1.0h;
half E = dot(B, z);
if (E > 0.0h) {
half F = 1.0h - a.x;
half G = A * F;
if (G < E) D = min(D, G / E);
}
half F = dot(C, z);
if (F > 0.0h) {
half G = a.x;
half H = A * G;
if (H < F) D = min(D, H / F);
}
a.yz = a.yz * D;
return css_oklab_to_linear_srgb_Qh3h3(a);
}
half3 css_oklab_to_linear_srgb_Qh3h3(half3 a) {
half b = (a.x + 0.396337777h * a.y) + 0.215803757h * a.z;
half c = (a.x - 0.105561346h * a.y) - 0.06385417h * a.z;
half d = (a.x - 0.08948418h * a.y) - 1.29148555h * a.z;
half e = (b * b) * b;
half f = (c * c) * c;
half g = (d * d) * d;
return half3((4.0767417h * e - 3.3077116h * f) + 0.230969936h * g, (-1.268438h * e + 2.60975742h * f) - 0.341319382h * g, (-0.00419608643h * e - 0.7034186h * f) + 1.70761466h * g);
}
half4 interpolated_to_rgb_unpremul_Qh4h4ii(half4 a, int b, int c) {
if (bool(c)) switch (b) {
case 2:
case 3:
case 4:
a = half4(a.xyz / max(a.w, 0.0001h), a.w);
break;
case 5:
case 6:
case 7:
case 9:
case 10:
a = half4(a.x, a.yz / max(a.w, 0.0001h), a.w);
break;
}
switch (b) {
case 2:
a.xyz = css_lab_to_xyz_Qh3h3(a.xyz);
break;
case 3:
a.xyz = css_oklab_to_linear_srgb_Qh3h3(a.xyz);
break;
case 4:
a.xyz = css_oklab_gamut_map_to_linear_srgb_Qh3h3(a.xyz);
break;
case 5:
a.xyz = css_lab_to_xyz_Qh3h3(a_Qh3h3(a.xyz));
break;
case 6:
a.xyz = css_oklab_to_linear_srgb_Qh3h3(a_Qh3h3(a.xyz));
break;
case 7:
a.xyz = css_oklab_gamut_map_to_linear_srgb_Qh3h3(a_Qh3h3(a.xyz));
break;
case 9:
a.xyz = css_hsl_to_srgb_Qh3h3(a.xyz);
break;
case 10:
half3 _3_a = a.xyz;
half3 _4_b;
_3_a.yz = _3_a.yz * 0.01h;
if (_3_a.y + _3_a.z >= 1.0h) _4_b = half3(_3_a.y / (_3_a.y + _3_a.z)); else {
_4_b = css_hsl_to_srgb_Qh3h3(half3(_3_a.x, 100.0h, 50.0h));
_4_b *= (1.0h - _3_a.y) - _3_a.z;
_4_b += _3_a.y;
}
a.xyz = _4_b;
break;
}
return a;
}
float2 v_Qf2if2(int a, float2 b) {
switch (a) {
case 0:
b.x = saturate(b.x);
break;
case 1:
b.x = fract(b.x);
break;
case 2:
{
float c = b.x - 1.0;
b.x = (c - 2.0 * floor(c * 0.5)) - 1.0;
if (false) b.x = clamp(b.x, -1.0, 1.0);
b.x = abs(b.x);
break;
}
case 3:
if (b.x < 0.0 || b.x > 1.0) return float2(0.0, -1.0);
break;
}
return b;
}
half4 x_Qh4f4f4f2(array<float4, 8> a, array<float4, 2> b, float2 c) {
if (c.y < 0.0) return half4(0.0h); else if (c.x < b[1].x) if (c.x < b[0].z) if (c.x <= b[0].x) return half4(a[0]); else if (c.x < b[0].y) return half4(mix(a[0], a[1], (c.x - b[0].x) / (b[0].y - b[0].x))); else return half4(mix(a[1], a[2], (c.x - b[0].y) / (b[0].z - b[0].y))); else if (c.x < b[0].w) return half4(mix(a[2], a[3], (c.x - b[0].z) / (b[0].w - b[0].z))); else return half4(mix(a[3], a[4], (c.x - b[0].w) / (b[1].x - b[0].w))); else if (c.x < b[1].z) if (c.x < b[1].y) return half4(mix(a[4], a[5], (c.x - b[1].x) / (b[1].y - b[1].x))); else return half4(mix(a[5], a[6], (c.x - b[1].y) / (b[1].z - b[1].y))); else if (c.x < b[1].w) return half4(mix(a[6], a[7], (c.x - b[1].z) / (b[1].w - b[1].z))); else return half4(a[7]);
}
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 _17_g = float2((float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10).x + 1e-05, 1.0);
_17_g = v_Qf2if2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemode_12, _17_g);
half4 _18_h = x_Qh4f4f4f2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colors_12, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].offsets_12, _17_g);
half4 _19_sk_csxform_prealpha;
half4 _20_a = interpolated_to_rgb_unpremul_Qh4h4ii(_18_h, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorSpace_12, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].doUnPremul_12);
half _21_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_13;
if (_21_b < 0.0h) {
_19_sk_csxform_prealpha = half4(_20_a.xyz / max(_20_a.w, 0.0001h), _20_a.w);
} else {
half _22_c = max(_20_a.w, _21_b - 1.0h);
half _23_d = max(_22_c, 1.0h - _21_b);
_19_sk_csxform_prealpha = half4(_20_a.xyz * _23_d, _22_c);
}
float sigma = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].radius_1 * float(clamp(_19_sk_csxform_prealpha.w, 0.0h, 1.0h));
if (sigma < 0.01) {
float2 _24_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _25_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _24_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_25_e = saturate(_25_e + 0.5h);
return (((_25_e.x * _25_e.y) * _25_e.z) * _25_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _24_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
}
half4 sum = half4(0.0h);
float total = 0.0;
for (int i = -16;i <= 16; i++) {
float d = float(i) * 0.1875;
float w = exp((-0.5 * d) * d);
float2 _26_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].axis_1 * (d * sigma)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _27_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _26_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_27_e = saturate(_27_e + 0.5h);
sum += ((((_27_e.x * _27_e.y) * _27_e.z) * _27_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _26_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4))) * half(w);
total += w;
}
return sum / half(total);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
