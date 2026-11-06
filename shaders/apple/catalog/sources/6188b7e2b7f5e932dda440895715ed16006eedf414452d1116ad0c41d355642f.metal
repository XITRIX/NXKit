#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float4 color_1;
float4x4 maskToDevice;
float4x4 localToDevice;
float2 atlasSizeInv;
};
struct Inputs {
    half4 SolidColor_1_Var [[user(locn0) center_no_perspective]];
    float2 textureCoords [[user(locn1)]];
    half texIndex [[user(locn2) flat]];
    half maskFormat [[user(locn3) flat]];
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
    sampler2D text_atlas_0;
    sampler2D text_atlas_1;
    sampler2D text_atlas_2;
    sampler2D text_atlas_3;
};
half4 sample_indexed_atlas_h4f2iZZZZ(float2 a, int b, sampler2D c, sampler2D d, sampler2D e, sampler2D f);
half4 sample_indexed_atlas_h4f2iZZZZ(float2 a, int b, sampler2D c, sampler2D d, sampler2D e, sampler2D f) {
switch (b) {
case 1:
return sample(d, a);
case 2:
return sample(e, a);
case 3:
return sample(f, a);
default:
return sample(c, a);
}
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> text_atlas_0_Tex [[texture(0)]], sampler text_atlas_0_Smplr [[sampler(0)]], texture2d<half> text_atlas_1_Tex [[texture(1)]], sampler text_atlas_1_Smplr [[sampler(1)]], texture2d<half> text_atlas_2_Tex [[texture(2)]], sampler text_atlas_2_Smplr [[sampler(2)]], texture2d<half> text_atlas_3_Tex [[texture(3)]], sampler text_atlas_3_Smplr [[sampler(3)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, &_anonInterface1, {text_atlas_0_Tex, text_atlas_0_Smplr}, {text_atlas_1_Tex, text_atlas_1_Smplr}, {text_atlas_2_Tex, text_atlas_2_Smplr}, {text_atlas_3_Tex, text_atlas_3_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
half4 outColor_1 = _in.SolidColor_1_Var;
half4 outputCoverage = half4(1.0h);
half4 _0_a = sample_indexed_atlas_h4f2iZZZZ(_in.textureCoords, int(_in.texIndex), _globals.text_atlas_0, _globals.text_atlas_1, _globals.text_atlas_2, _globals.text_atlas_3);
outputCoverage = int(_in.maskFormat) == 0 ? _0_a.xxxx : _0_a;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
