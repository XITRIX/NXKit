#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 upper2x2_2;
float2 translation_2;
float4 decalBounds_3;
float4 upper2x2_4;
float2 translation_4;
float2 invDimensions_6;
float4 upper2x2_10;
float2 translation_10;
float4 decalBounds_11;
float4 upper2x2_12;
float2 translation_12;
float2 invDimensions_14;
float4 upper2x2_18;
float2 translation_18;
float4 decalBounds_19;
float4 upper2x2_20;
float2 translation_20;
float2 invDimensions_22;
float4 upper2x2_26;
float2 translation_26;
float4 decalBounds_27;
float4 upper2x2_28;
float2 translation_28;
float2 invDimensions_30;
float4 upper2x2_34;
float2 translation_34;
float4 decalBounds_35;
float4 upper2x2_36;
float2 translation_36;
float2 invDimensions_38;
float4 upper2x2_42;
float2 translation_42;
float4 decalBounds_43;
float4 upper2x2_44;
float2 translation_44;
float2 invDimensions_46;
float4 upper2x2_50;
float2 translation_50;
float2 invDimensions_52;
half mode_55;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 localCoordsVar [[user(locn1)]];
    float2 LocalMatrix_2_Var [[user(locn2)]];
    float2 LocalMatrix_10_Var [[user(locn3)]];
    float2 LocalMatrix_18_Var [[user(locn4)]];
    float2 LocalMatrix_26_Var [[user(locn5)]];
    float2 LocalMatrix_34_Var [[user(locn6)]];
    float2 LocalMatrix_42_Var [[user(locn7)]];
    float2 CoordNormalize_52_Var [[user(locn8)]];
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
    sampler2D image_15;
    sampler2D image_23;
    sampler2D image_31;
    sampler2D image_39;
    sampler2D image_47;
    sampler2D image_53;
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_11_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_19_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_27_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_35_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_43_h4f2f4(float2 b, float4 c);
half4 RuntimeEffect_1_h4f2(Inputs _in, thread Globals& _globals, float2 pos) {
half4 _22_sk_csxform_prealpha;
half4 _23_a = sample(_globals.image_53, _in.CoordNormalize_52_Var);
half _24_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_55;
if (_24_b < 0.0h) {
_22_sk_csxform_prealpha = half4(_23_a.xyz / max(_23_a.w, 0.0001h), _23_a.w);
} else {
half _25_c = max(_23_a.w, _24_b - 1.0h);
half _26_d = max(_25_c, 1.0h - _24_b);
_22_sk_csxform_prealpha = half4(_23_a.xyz * _26_d, _25_c);
}
float v = float(powr(clamp(_22_sk_csxform_prealpha.w, 0.0h, 1.0h), 2.0h));
if (v < 0.00390625) {
half4 _27_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _in.LocalMatrix_2_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_27_e = saturate(_27_e + 0.5h);
{
half4 _28_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _in.LocalMatrix_10_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_28_e = saturate(_28_e + 0.5h);
return mix((((_27_e.x * _27_e.y) * _27_e.z) * _27_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _in.LocalMatrix_2_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4)), (((_28_e.x * _28_e.y) * _28_e.z) * _28_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _in.LocalMatrix_10_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12)), half(v * 256.0));
}
}
if (v < 0.015625) {
half4 _29_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _in.LocalMatrix_10_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_29_e = saturate(_29_e + 0.5h);
{
half4 _30_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_19 - _in.LocalMatrix_18_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_30_e = saturate(_30_e + 0.5h);
return mix((((_29_e.x * _29_e.y) * _29_e.z) * _29_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _in.LocalMatrix_10_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12)), (((_30_e.x * _30_e.y) * _30_e.z) * _30_e.w) * sample(_globals.image_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_22 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.zw) * _in.LocalMatrix_18_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_20)), half((v - 0.00390625) * 85.3333359));
}
}
if (v < 0.0625) {
half4 _31_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_19 - _in.LocalMatrix_18_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_31_e = saturate(_31_e + 0.5h);
{
half4 _32_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_27 - _in.LocalMatrix_26_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_32_e = saturate(_32_e + 0.5h);
return mix((((_31_e.x * _31_e.y) * _31_e.z) * _31_e.w) * sample(_globals.image_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_22 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.zw) * _in.LocalMatrix_18_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_20)), (((_32_e.x * _32_e.y) * _32_e.z) * _32_e.w) * sample(_globals.image_31, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_30 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.zw) * _in.LocalMatrix_26_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_28)), half((v - 0.015625) * 21.333334));
}
}
if (v < 0.25) {
half4 _33_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_27 - _in.LocalMatrix_26_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_33_e = saturate(_33_e + 0.5h);
{
half4 _34_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_35 - _in.LocalMatrix_34_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_34_e = saturate(_34_e + 0.5h);
return mix((((_33_e.x * _33_e.y) * _33_e.z) * _33_e.w) * sample(_globals.image_31, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_30 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.zw) * _in.LocalMatrix_26_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_28)), (((_34_e.x * _34_e.y) * _34_e.z) * _34_e.w) * sample(_globals.image_39, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_38 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.zw) * _in.LocalMatrix_34_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_36)), half((v - 0.0625) * 5.33333349));
}
}
half4 _35_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_35 - _in.LocalMatrix_34_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_35_e = saturate(_35_e + 0.5h);
half4 _36_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_43 - _in.LocalMatrix_42_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_36_e = saturate(_36_e + 0.5h);
return mix((((_35_e.x * _35_e.y) * _35_e.z) * _35_e.w) * sample(_globals.image_39, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_38 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.zw) * _in.LocalMatrix_34_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_36)), (((_36_e.x * _36_e.y) * _36_e.z) * _36_e.w) * sample(_globals.image_47, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_46 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_44.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_44.zw) * _in.LocalMatrix_42_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_44)), half((v - 0.25) * 1.33333337));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_15_Tex [[texture(1)]], sampler image_15_Smplr [[sampler(1)]], texture2d<half> image_23_Tex [[texture(2)]], sampler image_23_Smplr [[sampler(2)]], texture2d<half> image_31_Tex [[texture(3)]], sampler image_31_Smplr [[sampler(3)]], texture2d<half> image_39_Tex [[texture(4)]], sampler image_39_Smplr [[sampler(4)]], texture2d<half> image_47_Tex [[texture(5)]], sampler image_47_Smplr [[sampler(5)]], texture2d<half> image_53_Tex [[texture(6)]], sampler image_53_Smplr [[sampler(6)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_15_Tex, image_15_Smplr}, {image_23_Tex, image_23_Smplr}, {image_31_Tex, image_31_Smplr}, {image_39_Tex, image_39_Smplr}, {image_47_Tex, image_47_Smplr}, {image_53_Tex, image_53_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_in, _globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
