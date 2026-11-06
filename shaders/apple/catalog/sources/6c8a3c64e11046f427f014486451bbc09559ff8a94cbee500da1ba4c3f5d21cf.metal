#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float2 size_1;
float2 light_1;
float cornerRadius_1;
float cornerCurve_1;
float scale_1;
float dark_1;
float power_1;
float width_1;
float strength_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
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
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float glassRimDarkening_1_fff2fff2fffff(float rimDistance, float2 normal, float sceneLuma, float dark, float2 light, float power, float width, float scale, float strength, float shortSide);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle) {
float2 radial = p - circle.xy;
float2 join = se.zw - circle.xy;
if (radial.x * join.y - radial.y * join.x > 0.0) {
float len = length(radial);
return float3(len - circle.z, radial / max(len, 1e-06));
}
float m = max(max(p.x, p.y), 1e-06);
float norm = m * powr(powr(p.x / m, se.y) + powr(p.y / m, se.y), 1.0 / se.y);
float2 gradient = powr(p / max(norm, 1e-06), float2(se.y - 1.0));
float magnitude = max(length(gradient), 1e-06);
return float3((norm - se.x) / magnitude, gradient / magnitude);
}
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim) {
float2 q = abs(p);
float c = halfDim.x - halfDim.y;
float3 result;
if (q.y + c >= q.x) result = continuousOctant_1_f3f2f4f4(float2(q.x, q.y + c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTop_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerTopCircle_1); else {
result = continuousOctant_1_f3f2f4f4(float2(q.y, q.x - c), _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRight_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRightCircle_1);
result.yz = result.zy;
}
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
result.yz = result.yz * direction;
return result;
}
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim).yz;
float2 q = (abs(p) - halfDim) + r;
float2 direction = float2(float(p.x >= 0.0 ? 1.0 : -1.0), float(p.y >= 0.0 ? 1.0 : -1.0));
if (max(q.x, q.y) > 0.0) {
float2 n = max(q, 0.0);
return (direction * n) / max(length(n), 1e-06);
}
return q.x > q.y ? float2(direction.x, 0.0) : float2(0.0, direction.y);
}
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r) {
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _3_q = (abs(p) - halfDim) + r;
return float3((length(max(_3_q, 0.0)) + min(max(_3_q.x, _3_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float glassRimDarkening_1_fff2fff2fffff(float rimDistance, float2 normal, float sceneLuma, float dark, float2 light, float power, float width, float scale, float strength, float shortSide) {
float pixel = 1.0 / max(scale, 0.0001);
float band = clamp((max(width, 0.0) * 0.625 - rimDistance) / pixel + 0.5, 0.0, 1.0);
if ((band <= 0.0 || width <= 0.0) || strength <= 0.0) return 0.0;
float along = abs(dot(normal, light / max(length(light), 0.0001)));
float direction = 1.0 - mix(0.7, 1.0, dark) * powr(along, max(power, 1.0));
float bright = powr(clamp((sceneLuma - 0.5) * 2.0, 0.0, 1.0), 1.5);
float large = clamp((shortSide - 52.0) * 0.0147058824, 0.0, 1.0);
float darkDensity = (0.24 + 0.31 * bright) + large * (0.05 + 0.17 * smoothstep(0.5, 0.65, sceneLuma));
float density = mix(0.25, darkDensity, dark);
return ((band * direction) * density) * max(strength, 0.0);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _8_xy = _in.localCoordsVar / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].scale_1, 1.0);
float2 _9_halfDim = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1 * 0.5;
float2 _10_p = _8_xy - _9_halfDim;
float _11_r = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRadius_1, min(_9_halfDim.x, _9_halfDim.y));
float3 _12_surface = roundedBoxSample_1_f3f2f2f(_globals, _10_p, _9_halfDim, _11_r);
float _13_d = _12_surface.x;
float2 _14_normal = _12_surface.yz;
float2 _15_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * _in.localCoordsVar + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _16_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _15_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_16_e = saturate(_16_e + 0.5h);
half4 _17_original = (((_16_e.x * _16_e.y) * _16_e.z) * _16_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _15_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
float2 _18_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * ((_8_xy + _14_normal * max(2.0 - _13_d, 0.0)) * max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].scale_1, 1.0)) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _19_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _18_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_19_e = saturate(_19_e + 0.5h);
half4 _20_outside = (((_19_e.x * _19_e.y) * _19_e.z) * _19_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _18_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
float _21_luma = float(dot(_20_outside.xyz / max(_20_outside.w, 0.0001h), half3(0.2126h, 0.7152h, 0.0722h)));
float _22_density = glassRimDarkening_1_fff2fff2fffff(_13_d, _14_normal, _21_luma, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].dark_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].light_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].power_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].width_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].scale_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].strength_1, min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].size_1.y));
half3 _23_loss = min(half3(half(_22_density)) * _20_outside.w, _20_outside.xyz);
half4 outColor_1 = half4(max(_17_original.xyz - _23_loss, half3(0.0h)), _17_original.w);
_out.sk_FragColor = outColor_1;
return _out;
}
