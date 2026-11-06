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
half4 paintColor;
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
half4 _15_sk_csxform_prealpha;
half4 _16_a = sample(_globals.image_5, _in.CoordNormalize_4_Var);
half _17_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_7;
if (_17_b < 0.0h) {
_15_sk_csxform_prealpha = half4(_16_a.xyz / max(_16_a.w, 0.0001h), _16_a.w);
} else {
half _18_c = max(_16_a.w, _17_b - 1.0h);
half _19_d = max(_18_c, 1.0h - _17_b);
_15_sk_csxform_prealpha = half4(_16_a.xyz * _19_d, _18_c);
}
half4 outColor_1 = _15_sk_csxform_prealpha * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].paintColor.w;
_out.sk_FragColor = outColor_1;
return _out;
}
