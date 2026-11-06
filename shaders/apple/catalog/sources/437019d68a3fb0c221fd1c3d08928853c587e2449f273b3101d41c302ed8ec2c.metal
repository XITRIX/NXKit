#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float captureTaps_1;
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
half4 captureCell_1_h4f2(thread Globals& _globals, float2 p);
half4 captureCell_1_h4f2(thread Globals& _globals, float2 p) {
half4 sum = half4(0.0h);
for (int y = 0;y < 4; ++y) for (int x = 0;x < 4; ++x) {
if (float(x) < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1 && float(y) < _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1) {
float2 _3_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * ((p + (float2(float(x), float(y)) + 0.5) / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1) - 0.5) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _4_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _3_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_4_e = saturate(_4_e + 0.5h);
sum += (((_4_e.x * _4_e.y) * _4_e.z) * _4_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _3_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
}
}
return sum / half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _5_grid = floor(_in.localCoordsVar - 0.5) + 0.5;
float2 _6_f = fract(_in.localCoordsVar - 0.5);
half4 outColor_1 = half4(mix(mix(float4(captureCell_1_h4f2(_globals, _5_grid)), float4(captureCell_1_h4f2(_globals, _5_grid + float2(1.0, 0.0))), _6_f.x), mix(float4(captureCell_1_h4f2(_globals, _5_grid + float2(0.0, 1.0))), float4(captureCell_1_h4f2(_globals, _5_grid + float2(1.0))), _6_f.x), _6_f.y));
_out.sk_FragColor = outColor_1;
return _out;
}
