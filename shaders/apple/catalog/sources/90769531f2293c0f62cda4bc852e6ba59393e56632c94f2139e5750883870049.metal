#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 color_1;
float3x3 maskToDeviceRemainder;
};
struct Inputs {
    half4 SolidColor_1_Var [[user(locn0) center_no_perspective]];
    float4 maskBounds [[user(locn1)]];
    float2 textureCoords [[user(locn2)]];
    half invert [[user(locn3)]];
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
    constant IntrinsicUniforms* _anonInterface1;
    sampler2D pathAtlas;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> pathAtlas_Tex [[texture(0)]], sampler pathAtlas_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, &_anonInterface1, {pathAtlas_Tex, pathAtlas_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
half4 outColor_1 = _in.SolidColor_1_Var;
half4 outputCoverage = half4(1.0h);
half c = sample(_globals.pathAtlas, clamp(_in.textureCoords, _in.maskBounds.xy, _in.maskBounds.zw)).x;
outputCoverage = half4(mix(c, 1.0h - c, _in.invert));
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
