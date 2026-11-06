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
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 LocalMatrix_2_h4f2(thread Globals& _globals, float2 pos) {
float2 _1_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _2_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _1_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_2_e = saturate(_2_e + 0.5h);
return (((_2_e.x * _2_e.y) * _2_e.z) * _2_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _1_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _3_grid = floor(_in.localCoordsVar - 0.5) + 0.5;
float2 _4_f = fract(_in.localCoordsVar - 0.5);
half4 outColor_1 = half4(mix(mix(float4(LocalMatrix_2_h4f2(_globals, _3_grid)), float4(LocalMatrix_2_h4f2(_globals, _3_grid + float2(1.0, 0.0))), _4_f.x), mix(float4(LocalMatrix_2_h4f2(_globals, _3_grid + float2(0.0, 1.0))), float4(LocalMatrix_2_h4f2(_globals, _3_grid + float2(1.0))), _4_f.x), _4_f.y));
_out.sk_FragColor = outColor_1;
return _out;
}
