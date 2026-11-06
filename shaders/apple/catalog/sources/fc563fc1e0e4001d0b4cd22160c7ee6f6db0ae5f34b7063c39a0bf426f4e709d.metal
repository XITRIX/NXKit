#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 upper2x2_2;
float2 translation_2;
float2 invDimensions_4;
half mode_7;
float4 color_9;
half4 coeffs_11;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 CoordNormalize_4_Var [[user(locn1)]];
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_5_Tex [[texture(0)]], sampler image_5_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_5_Tex, image_5_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _21_sk_csxform_prealpha;
half4 _22_a = sample(_globals.image_5, _in.CoordNormalize_4_Var);
half _23_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_7;
if (_23_b < 0.0h) {
_21_sk_csxform_prealpha = half4(_22_a.xyz / max(_22_a.w, 0.0001h), _22_a.w);
} else {
half _24_c = max(_22_a.w, _23_b - 1.0h);
half _25_d = max(_24_c, 1.0h - _23_b);
_21_sk_csxform_prealpha = half4(_22_a.xyz * _25_d, _24_c);
}
half4 _26_a = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].color_9);
half4 _27_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].coeffs_11;
half2 _28_d = _27_c.xy + _27_c.zw * half2(_21_sk_csxform_prealpha.w, _26_a.w);
half4 outColor_1 = _26_a * _28_d.x + _21_sk_csxform_prealpha * _28_d.y;
_out.sk_FragColor = outColor_1;
return _out;
}
