#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 upper2x2_1;
float2 translation_1;
float4 decalBounds_2;
float4 upper2x2_3;
float2 translation_3;
float2 invDimensions_5;
};
struct Inputs {
    uint ssboIndexVar [[user(locn0) flat]];
    float2 LocalMatrix_1_Var [[user(locn1)]];
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
    sampler2D image_6;
};
half4 sk_decal_image_2_h4f2f4(float2 b, float4 c);
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _2_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_2 - _in.LocalMatrix_1_Var.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_2_e = saturate(_2_e + 0.5h);
half4 outColor_1 = (((_2_e.x * _2_e.y) * _2_e.z) * _2_e.w) * sample(_globals.image_6, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_5 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_3.zw) * _in.LocalMatrix_1_Var + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_3));
_out.sk_FragColor = outColor_1;
return _out;
}
