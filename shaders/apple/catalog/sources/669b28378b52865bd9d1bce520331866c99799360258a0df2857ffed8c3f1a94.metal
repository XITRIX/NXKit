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
float4 upper2x2_11;
float2 translation_11;
float2 invDimensions_13;
half mode_16;
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
struct Globals {
    const device CombinedUniforms* _anonInterface0;
    uint uniformSsboIndex;
    constant IntrinsicUniforms* _anonInterface1;
    sampler2D image_4;
    sampler2D image_14;
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], texture2d<half> image_14_Tex [[texture(1)]], sampler image_14_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}, {image_14_Tex, image_14_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = sk_csxform_prealpha_h4h4h(sample(_globals.image_4, _in.CoordNormalize_3_Var), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6);
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
half4 outColor_10 = sk_csxform_prealpha_h4h4h(sample(_globals.image_14, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_13 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_11.zw) * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_10.zw) * devCoord + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_10) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_11)), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_16);
outputCoverage *= outColor_10.w;
_out.sk_FragColor = outColor_1 * outputCoverage;
_out.sk_SecondaryFragColor = outputCoverage;
return _out;
}
