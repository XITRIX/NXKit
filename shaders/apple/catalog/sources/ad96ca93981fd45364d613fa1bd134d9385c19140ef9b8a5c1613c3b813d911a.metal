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
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 CoordNormalize_3_Var [[user(locn1)]];
    float4 edgeDistances [[user(locn2)]];
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
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _10_sk_csxform_prealpha;
half4 _11_a = sample(_globals.image_4, _in.CoordNormalize_3_Var);
half _12_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6;
if (_12_b < 0.0h) {
_10_sk_csxform_prealpha = half4(_11_a.xyz / max(_11_a.w, 0.0001h), _11_a.w);
} else {
half _13_c = max(_11_a.w, _12_b - 1.0h);
half _14_d = max(_13_c, 1.0h - _12_b);
_10_sk_csxform_prealpha = half4(_11_a.xyz * _14_d, _13_c);
}
half4 outColor_1 = _10_sk_csxform_prealpha;
half4 outputCoverage = half4(1.0h);
float2 _15_d = min(_in.edgeDistances.xy, _in.edgeDistances.zw);
float _16_e = min(_15_d.x, _15_d.y) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
outputCoverage = half4(half(saturate(_16_e)));
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
