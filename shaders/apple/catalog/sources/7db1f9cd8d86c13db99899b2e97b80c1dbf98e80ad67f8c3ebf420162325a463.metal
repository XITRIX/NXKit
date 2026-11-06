#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const int captureTapCount_1 = 4;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float captureTaps_1;
float4 upper2x2_2;
float2 translation_2;
float4 decalBounds_3;
float4 upper2x2_4;
float2 translation_4;
float2 invImgSize_6;
float4 subsetInsetClamp_6;
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
    sampler2D image_6;
};
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c) {
return sample(c, a * b);
}
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 captureCell_1_h4f2(thread Globals& _globals, float2 p);
half4 captureCell_1_h4f2(thread Globals& _globals, float2 p) {
half4 sum = half4(0.0h);
for (int y = 0;y < captureTapCount_1; ++y) for (int x = 0;x < captureTapCount_1; ++x) {
float2 _8_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * ((p + (float2(float(x), float(y)) + 0.5) / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1) - 0.5) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _9_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _8_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_9_e = saturate(_9_e + 0.5h);
float4 _10_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_6;
sum += (((_9_e.x * _9_e.y) * _9_e.z) * _9_e.w) * q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _8_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4, _10_c.xy, _10_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_6, _globals.image_6);
}
return sum / half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1 * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].captureTaps_1);
}
half4 RuntimeEffect_1_h4f2(thread Globals& _globals, float2 pos) {
float2 grid = floor(pos - 0.5) + 0.5;
float2 f = fract(pos - 0.5);
half4 first = captureCell_1_h4f2(_globals, grid);
half4 top = half4(f.x == 0.0 ? float4(first) : mix(float4(first), float4(captureCell_1_h4f2(_globals, grid + float2(1.0, 0.0))), f.x));
if (f.y == 0.0) return top;
half4 bottom = captureCell_1_h4f2(_globals, grid + float2(0.0, 1.0));
if (f.x != 0.0) bottom = half4(mix(float4(bottom), float4(captureCell_1_h4f2(_globals, grid + float2(1.0))), f.x));
return half4(mix(float4(top), float4(bottom), f.y));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_6_Tex [[texture(0)]], sampler image_6_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_6_Tex, image_6_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 outColor_1 = RuntimeEffect_1_h4f2(_globals, _in.localCoordsVar);
_out.sk_FragColor = outColor_1;
return _out;
}
