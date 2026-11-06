#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const int kMaxLoopLimit_1 = 7;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
array<half4, 7> kernel_1;
array<half4, 14> offsets_1;
float4 upper2x2_2;
float2 translation_2;
float2 invDimensions_4;
half mode_7;
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
    sampler2D image_5;
};
half4 LocalMatrix_2_h4f2(thread Globals& _globals, float2 pos) {
half4 _2_a = sample(_globals.image_5, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_4 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2));
half _3_c = mix(_2_a.w, _2_a.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].mode_7);
return half4(0.0h, 0.0h, 0.0h, _3_c);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_5_Tex [[texture(0)]], sampler image_5_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_5_Tex, image_5_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
half4 _4_sum = half4(0.0h);
for (int _5_i = 0;_5_i < kMaxLoopLimit_1; ++_5_i) {
half4 _6_k = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].kernel_1[_5_i];
half4 _7_o = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].offsets_1[2 * _5_i];
_4_sum += _6_k.x * LocalMatrix_2_h4f2(_globals, _in.localCoordsVar + float2(_7_o.xy));
_4_sum += _6_k.y * LocalMatrix_2_h4f2(_globals, _in.localCoordsVar + float2(_7_o.zw));
_7_o = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].offsets_1[2 * _5_i + 1];
_4_sum += _6_k.z * LocalMatrix_2_h4f2(_globals, _in.localCoordsVar + float2(_7_o.xy));
_4_sum += _6_k.w * LocalMatrix_2_h4f2(_globals, _in.localCoordsVar + float2(_7_o.zw));
}
half4 outColor_1 = _4_sum;
outColor_1 = half4(outColor_1.w, 0.0h, 0.0h, 0.0h);
_out.sk_FragColor = outColor_1;
return _out;
}
