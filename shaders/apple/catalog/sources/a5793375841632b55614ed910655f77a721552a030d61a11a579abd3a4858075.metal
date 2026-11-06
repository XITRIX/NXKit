#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const int kMaxLoopLimit_1 = 8;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<half4, 14> offsetsAndKernel_1;
half2 dir_1;
float4 upper2x2_2;
float2 translation_2;
float2 invImgSize_4;
float4 subsetInsetClamp_4;
half mode_6;
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
    sampler2D image_4;
};
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c);
half4 q_Qh4f2f2Z(float2 a, float2 b, sampler2D c) {
return sample(c, a * b);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_4_Tex [[texture(0)]], sampler image_4_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_4_Tex, image_4_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _12_sum = half4(0.0h);
for (int _13_i = 0;_13_i < kMaxLoopLimit_1; ++_13_i) {
half4 _14_s = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].offsetsAndKernel_1[_13_i];
float4 _15_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_4;
half4 _16_a = q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_in.localCoordsVar + float2(_14_s.x * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dir_1)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _15_c.xy, _15_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals.image_4);
half _17_c = mix(_16_a.w, _16_a.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6);
_12_sum += _14_s.y * half4(0.0h, 0.0h, 0.0h, _17_c);
float4 _18_c = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].subsetInsetClamp_4;
half4 _19_a = q_Qh4f2f2Z(clamp(float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_in.localCoordsVar + float2(_14_s.z * _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dir_1)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2, _18_c.xy, _18_c.zw), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invImgSize_4, _globals.image_4);
half _20_c = mix(_19_a.w, _19_a.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_6);
_12_sum += _14_s.w * half4(0.0h, 0.0h, 0.0h, _20_c);
}
half4 outColor_1 = _12_sum;
outColor_1 = half4(outColor_1.w, 0.0h, 0.0h, 0.0h);
_out.sk_FragColor = outColor_1;
return _out;
}
