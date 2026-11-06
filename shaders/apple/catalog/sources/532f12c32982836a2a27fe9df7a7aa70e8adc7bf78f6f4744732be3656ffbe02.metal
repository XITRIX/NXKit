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
float2 invDimensions_44;
half mode_47;
float4 upper2x2_48;
float2 translation_48;
float2 invDimensions_50;
half mode_53;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 localCoordsVar [[user(locn1)]];
    float2 LocalMatrix_2_Var [[user(locn2)]];
    float2 LocalMatrix_10_Var [[user(locn3)]];
    float2 LocalMatrix_18_Var [[user(locn4)]];
    float2 LocalMatrix_26_Var [[user(locn5)]];
    float2 LocalMatrix_34_Var [[user(locn6)]];
    float2 CoordNormalize_44_Var [[user(locn7)]];
    float2 CoordNormalize_50_Var [[user(locn8)]];
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
    sampler2D image_45;
    sampler2D image_51;
};
half4 sk_csxform_prealpha_h4h4h(half4 a, half b);
half4 sk_csxform_prealpha_h4h4h(half4 a, half b) {
if (b < 0.0h) {
return half4(a.xyz / max(a.w, 0.0001h), a.w);
} else {
half c = max(a.w, b - 1.0h);
half d = max(c, 1.0h - b);
return half4(a.xyz * d, c);
}
}
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_11_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_19_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_27_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_35_h4f2f4(float2 b, float4 c);
half4 RuntimeEffect_1_h4f2(Inputs _in, thread Globals& _globals, float2 pos) {
float v = float(powr(clamp(sk_csxform_prealpha_h4h4h(sample(_globals.image_51, _in.CoordNormalize_50_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_53).w, 0.0h, 1.0h), 2.0h));
if (v < 0.00390625) {
half4 _10_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _in.LocalMatrix_2_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_10_e = saturate(_10_e + 0.5h);
{
half4 _11_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _in.LocalMatrix_10_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_11_e = saturate(_11_e + 0.5h);
return mix((((_10_e.x * _10_e.y) * _10_e.z) * _10_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _in.LocalMatrix_2_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4)), (((_11_e.x * _11_e.y) * _11_e.z) * _11_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _in.LocalMatrix_10_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12)), half(v * 256.0));
}
}
if (v < 0.015625) {
half4 _12_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_11 - _in.LocalMatrix_10_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_12_e = saturate(_12_e + 0.5h);
{
half4 _13_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_19 - _in.LocalMatrix_18_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_13_e = saturate(_13_e + 0.5h);
return mix((((_12_e.x * _12_e.y) * _12_e.z) * _12_e.w) * sample(_globals.image_15, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_14 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_12.zw) * _in.LocalMatrix_10_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_12)), (((_13_e.x * _13_e.y) * _13_e.z) * _13_e.w) * sample(_globals.image_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_22 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.zw) * _in.LocalMatrix_18_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_20)), half((v - 0.00390625) * 85.3333359));
}
}
if (v < 0.0625) {
half4 _14_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_19 - _in.LocalMatrix_18_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_14_e = saturate(_14_e + 0.5h);
{
half4 _15_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_27 - _in.LocalMatrix_26_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_15_e = saturate(_15_e + 0.5h);
return mix((((_14_e.x * _14_e.y) * _14_e.z) * _14_e.w) * sample(_globals.image_23, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_22 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_20.zw) * _in.LocalMatrix_18_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_20)), (((_15_e.x * _15_e.y) * _15_e.z) * _15_e.w) * sample(_globals.image_31, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_30 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.zw) * _in.LocalMatrix_26_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_28)), half((v - 0.015625) * 21.333334));
}
}
if (v < 0.25) {
half4 _16_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_27 - _in.LocalMatrix_26_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_16_e = saturate(_16_e + 0.5h);
{
half4 _17_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_35 - _in.LocalMatrix_34_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_17_e = saturate(_17_e + 0.5h);
return mix((((_16_e.x * _16_e.y) * _16_e.z) * _16_e.w) * sample(_globals.image_31, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_30 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_28.zw) * _in.LocalMatrix_26_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_28)), (((_17_e.x * _17_e.y) * _17_e.z) * _17_e.w) * sample(_globals.image_39, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_38 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.zw) * _in.LocalMatrix_34_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_36)), half((v - 0.0625) * 5.33333349));
}
}
half4 _18_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_35 - _in.LocalMatrix_34_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_18_e = saturate(_18_e + 0.5h);
return mix((((_18_e.x * _18_e.y) * _18_e.z) * _18_e.w) * sample(_globals.image_39, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_38 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_36.zw) * _in.LocalMatrix_34_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_36)), sk_csxform_prealpha_h4h4h(sample(_globals.image_45, _in.CoordNormalize_44_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_47), half((v - 0.25) * 1.33333337));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_15_Tex [[texture(1)]], sampler image_15_Smplr [[sampler(1)]], texture2d<half> image_23_Tex [[texture(2)]], sampler image_23_Smplr [[sampler(2)]], texture2d<half> image_31_Tex [[texture(3)]], sampler image_31_Smplr [[sampler(3)]], texture2d<half> image_39_Tex [[texture(4)]], sampler image_39_Smplr [[sampler(4)]], texture2d<half> image_45_Tex [[texture(5)]], sampler image_45_Smplr [[sampler(5)]], texture2d<half> image_51_Tex [[texture(6)]], sampler image_51_Smplr [[sampler(6)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_15_Tex, image_15_Smplr}, {image_23_Tex, image_23_Smplr}, {image_31_Tex, image_31_Smplr}, {image_39_Tex, image_39_Smplr}, {image_45_Tex, image_45_Smplr}, {image_51_Tex, image_51_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_in, _globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
