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
float4 upper2x2_10;
float2 translation_10;
float4 upper2x2_12;
float2 translation_12;
int numStops_14;
int bufferOffset_14;
int tilemode_14;
int colorSpace_14;
int doUnPremul_14;
half mode_15;
float4 upper2x2_17;
float2 translation_17;
int numStops_19;
int bufferOffset_19;
int tilemode_19;
int colorSpace_19;
int doUnPremul_19;
half mode_20;
float4 upper2x2_21;
float2 translation_21;
int numStops_23;
int bufferOffset_23;
int tilemode_23;
int colorSpace_23;
int doUnPremul_23;
half mode_24;
half4 coeffs_25;
half4 coeffs_26;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 CoordNormalize_3_Var [[user(locn1)]];
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
struct FSStorageBuffer {
float fsStorageBuffer[1];
};
struct Globals {
    const device CombinedUniforms* _anonInterface0;
    uint uniformSsboIndex;
    constant IntrinsicUniforms* _anonInterface1;
    const device FSStorageBuffer* _anonInterface2;
    sampler2D image_4;
};
half3 a_Qh3h3(half3 a);
half3 css_hsl_to_srgb_Qh3h3(half3 b);
half3 css_lab_to_xyz_Qh3h3(half3 a);
half3 css_oklab_gamut_map_to_linear_srgb_Qh3h3(half3 a);
half3 css_oklab_to_linear_srgb_Qh3h3(half3 a);
half4 z_Qh4fi(const device float* a, int b);
half4 A_Qh4fiif2(const device float* a, int b, int c, float2 d);
half4 interpolated_to_rgb_unpremul_Qh4h4ii(half4 a, int b, int c);
float2 v_Qf2if2(int a, float2 b);
half4 sk_csxform_prealpha_h4h4h(half4 a, half b);
half4 sk_linear_grad_buf_shader_h4f2iiiiif(float2 a, int b, int c, int d, int e, int f, const device float* g);
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
half4 z_Qh4fi(const device float* a, int b) {
return half4(half(a[b]), half(a[b + 1]), half(a[b + 2]), half(a[b + 3]));
}
half4 A_Qh4fiif2(const device float* a, int b, int c, float2 d) {
int e = b + c;
if (d.y < 0.0) return half4(0.0h); else if (d.x == 0.0) return z_Qh4fi(a, e); else if (d.x == 1.0) {
int f = e + (c - 1) * 4;
return z_Qh4fi(a, f);
} else {
int f = b;
int g = (f + c) - 1;
for (int h = 1;h < c; h += h) {
int j = (f + g) / 2;
if (d.x < a[j]) g = j; else f = j;
}
int h = e + (f - b) * 4;
float i = a[f];
half4 j = z_Qh4fi(a, h);
int k = e + (g - b) * 4;
float l = a[g];
if (l == i) return j; else {
half4 m = z_Qh4fi(a, k);
return half4(mix(float4(j), float4(m), (d.x - i) / (l - i)));
}
}
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
half4 sk_csxform_prealpha_h4h4h(half4 a, half b) {
if (b < 0.0h) {
return half4(a.xyz / max(a.w, 0.0001h), a.w);
} else {
half c = max(a.w, b - 1.0h);
half d = max(c, 1.0h - b);
return half4(a.xyz * d, c);
}
}
half4 sk_linear_grad_buf_shader_h4f2iiiiif(float2 a, int b, int c, int d, int e, int f, const device float* g) {
float2 h = float2(a.x + 1e-05, 1.0);
h = v_Qf2if2(d, h);
half4 i = A_Qh4fiif2(g, c, b, h);
return interpolated_to_rgb_unpremul_Qh4h4ii(i, e, f);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], const device FSStorageBuffer& _anonInterface2 [[buffer(4)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, &_anonInterface2, {image_4_Tex, image_4_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = sk_csxform_prealpha_h4h4h(sample(_globals.image_4, _in.CoordNormalize_3_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6);
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
float2 _21_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * devCoord + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10;
half4 _22_a = sk_csxform_prealpha_h4h4h(sk_linear_grad_buf_shader_h4f2iiiiif(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_17.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_17.zw) * _21_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_17, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].numStops_19, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bufferOffset_19, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemode_19, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorSpace_19, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].doUnPremul_19, _globals._anonInterface2->fsStorageBuffer), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_20);
half4 _23_b = sk_csxform_prealpha_h4h4h(sk_linear_grad_buf_shader_h4f2iiiiif(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_21.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_21.zw) * _21_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_21, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].numStops_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bufferOffset_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemode_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorSpace_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].doUnPremul_23, _globals._anonInterface2->fsStorageBuffer), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_24);
half4 _24_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].coeffs_25;
half2 _25_d = _24_c.xy + _24_c.zw * half2(_23_b.w, _22_a.w);
half4 _26_a = sk_csxform_prealpha_h4h4h(sk_linear_grad_buf_shader_h4f2iiiiif(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _21_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].numStops_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].bufferOffset_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].tilemode_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorSpace_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].doUnPremul_14, _globals._anonInterface2->fsStorageBuffer), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_15);
half4 _27_b = _22_a * _25_d.x + _23_b * _25_d.y;
half4 _28_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].coeffs_26;
half2 _29_d = _28_c.xy + _28_c.zw * half2(_27_b.w, _26_a.w);
half4 outColor_10 = _26_a * _29_d.x + _27_b * _29_d.y;
outputCoverage *= outColor_10.w;
_out.sk_FragColor = outColor_1 * outputCoverage;
_out.sk_SecondaryFragColor = outputCoverage;
return _out;
}
