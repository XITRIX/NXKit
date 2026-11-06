#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 color_1;
float4 upper2x2_5;
float2 translation_5;
float4 upper2x2_7;
float2 translation_7;
float2 invDimensions_9;
half mode_12;
half4x4 colorMatrix_13;
half4 colorTranslate_13;
half2 minMaxRGB_13;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    half4 SolidColor_1_Var [[user(locn1) center_no_perspective]];
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
    sampler2D image_10;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_10_Tex [[texture(0)]], sampler image_10_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_10_Tex, image_10_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = _in.SolidColor_1_Var;
half4 outputCoverage = half4(1.0h);
float2 devCoord = float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).xy - _globals._anonInterface1->viewport.xy;
half4 _24_sk_csxform_prealpha;
half4 _25_a = sample(_globals.image_10, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_9 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_7.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_7.zw) * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_5.zw) * devCoord + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_5) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_7));
half _26_b = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_12;
if (_26_b < 0.0h) {
_24_sk_csxform_prealpha = half4(_25_a.xyz / max(_25_a.w, 0.0001h), _25_a.w);
} else {
half _27_c = max(_25_a.w, _26_b - 1.0h);
half _28_d = max(_27_c, 1.0h - _26_b);
_24_sk_csxform_prealpha = half4(_25_a.xyz * _28_d, _27_c);
}
half4 _29_a = _24_sk_csxform_prealpha;
half2 _30_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].minMaxRGB_13;
_29_a = half4(_29_a.xyz / max(_29_a.w, 0.0001h), _29_a.w);
_29_a = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorMatrix_13 * _29_a + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].colorTranslate_13;
_29_a = clamp(_29_a, half4(_30_d.xxx, 0.0h), half4(_30_d.yyy, 1.0h));
_29_a.xyz = _29_a.xyz * _29_a.w;
half4 outColor_5 = _29_a;
outputCoverage *= outColor_5.w;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
