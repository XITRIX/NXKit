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
float4 upper2x2_8;
float2 translation_8;
float2 invDimensions_10;
half mode_13;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 CoordNormalize_4_Var [[user(locn1)]];
    float2 CoordNormalize_10_Var [[user(locn2)]];
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
    sampler2D image_11;
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_5_Tex [[texture(0)]], sampler image_5_Smplr [[sampler(0)]], texture2d<half> image_11_Tex [[texture(1)]], sampler image_11_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_5_Tex, image_5_Smplr}, {image_11_Tex, image_11_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = min(sk_csxform_prealpha_h4h4h(sample(_globals.image_5, _in.CoordNormalize_4_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_7) + sk_csxform_prealpha_h4h4h(sample(_globals.image_11, _in.CoordNormalize_10_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_13), 1.0h);
_out.sk_FragColor = outColor_1;
return _out;
}
