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
float2 invDimensions_12;
half mode_15;
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
    sampler2D image_13;
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
half4 _13_sk_csxform_prealpha;
half4 _14_a = sample(_globals.image_13, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_12 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10));
half _15_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_15;
if (_15_b < 0.0h) {
_13_sk_csxform_prealpha = half4(_14_a.xyz / max(_14_a.w, 0.0001h), _14_a.w);
} else {
half _16_c = max(_14_a.w, _15_b - 1.0h);
half _17_d = max(_16_c, 1.0h - _15_b);
_13_sk_csxform_prealpha = half4(_14_a.xyz * _17_d, _16_c);
}
float sigma = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].radius_1 * float(clamp(_13_sk_csxform_prealpha.w, 0.0h, 1.0h));
if (sigma < 0.01) {
float2 _18_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _19_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _18_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_19_e = saturate(_19_e + 0.5h);
return (((_19_e.x * _19_e.y) * _19_e.z) * _19_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _18_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
}
half4 sum = half4(0.0h);
float total = 0.0;
for (int i = -8;i <= 8; i++) {
float d = float(i) * 0.333333343;
float w = exp((-0.5 * d) * d);
float2 _20_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (pos + (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].axis_1 * d) * sigma) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _21_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _20_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_21_e = saturate(_21_e + 0.5h);
sum += ((((_21_e.x * _21_e.y) * _21_e.z) * _21_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _20_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4))) * half(w);
total += w;
}
return sum / half(total);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_13_Tex [[texture(1)]], sampler image_13_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_13_Tex, image_13_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
