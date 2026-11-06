#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct sampler2D {texture2d<half> tex;sampler smp;};half4 sample(sampler2D i, float2 p, float b=-0.475) { return i.tex.sample(i.smp, p, bias(b)); }half4 sample(sampler2D i, float3 p, float b=-0.475) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }half4 sampleLod(sampler2D i, float3 p, float lod) {return i.tex.sample(i.smp, p.xy / p.z, level(lod));}half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));}struct CombinedUniformData {
float2 resolution_1;
float filterScale_1;
float cornerCurve_1;
float contentScale_1;
float2 displacement_1;
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
float4 upper2x2_10;
float2 translation_10;
float4 decalBounds_11;
float4 upper2x2_12;
float2 translation_12;
float2 invDimensions_14;
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
    sampler2D image_15;
};
half4 sk_decal_image_3_h4f2f4(float2 b, float4 c);
half4 sk_decal_image_11_h4f2f4(float2 b, float4 c);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float2 lensNormalDirection_1_f2f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 roundedBoxSample_1_f3f2f2f(thread Globals& _globals, float2 p, float2 halfDim, float r);
float3 lensShape_1_f3f2f2(float2 p, float2 size);
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization);
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
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
return len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
}
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
if ((halfDim.x == halfDim.y && r == halfDim.x) && dot(p, p) > 1e-06) {
float len = length(p);
float2 normal = len == 0.0 && (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || r <= 0.0) ? float2(0.0, 1.0) : p / max(len, 1e-06);
return float3(len - r, normal);
}
if (_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 > 0.5 && r > 0.0) return continuousCornerSample_1_f3f2f2(_globals, p, halfDim);
float2 _6_q = (abs(p) - halfDim) + r;
return float3((length(max(_6_q, 0.0)) + min(max(_6_q.x, _6_q.y), 0.0)) - r, lensNormalDirection_1_f2f2f2f(_globals, p, halfDim, r));
}
float2 lensDisplaced_1_f2f2f2ffff(thread Globals& _globals, float2 p, float2 size, float amount, float height, float curvature, float ovalization) {
if (amount == 0.0 || height <= 0.0) return p;
float3 field = roundedBoxSample_1_f3f2f2f(_globals, p - size * 0.5, size * 0.5, min(size.x, size.y) * 0.5);
float depth = -field.x;
float width = max((abs(field.y) + abs(field.z)) / max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].contentScale_1, 1.0), 0.0001);
float t = clamp(depth / height, 0.0, 1.0);
float flatBand = float(t < 1.0 ? 0.2929 : 0.0);
float strength = mix(flatBand, 1.0 - sqrt(max(t * (2.0 - t), 0.0)), curvature);
float endpoint = clamp((height - depth) / width + 0.5, 0.0, 1.0);
float2 delta = p - size * 0.5;
float2 radial = float2(delta.x, (delta.y * size.x) / max(size.y, 0.0001));
float2 normal = field.yz;
if (length(radial) > 1e-06) {
float2 blended = mix(normal, normalize(radial), ovalization);
if (length(blended) > 1e-06) normal = normalize(blended);
}
return p + normal * ((amount * strength) * endpoint);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], texture2d<half> image_7_Tex [[texture(0)]], sampler image_7_Smplr [[sampler(0)]], texture2d<half> image_15_Tex [[texture(1)]], sampler image_15_Smplr [[sampler(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1, {image_7_Tex, image_7_Smplr}, {image_15_Tex, image_15_Smplr}};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float _9_scale = max(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].filterScale_1, 1.0);
float2 _10_p = _in.localCoordsVar / _9_scale;
float2 _11_sample = lensDisplaced_1_f2f2f2ffff(_globals, _10_p, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].resolution_1, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].displacement_1.x, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].displacement_1.y, 1.0, 0.5);
float2 _12_pos = float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_2.zw) * (_11_sample * _9_scale) + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_2;
half4 _13_e = half4(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].decalBounds_3 - _12_pos.xyxy) * half4(-1.0h, -1.0h, 1.0h, 1.0h);
_13_e = saturate(_13_e + 0.5h);
half4 outColor_1 = (((_13_e.x * _13_e.y) * _13_e.z) * _13_e.w) * sample(_globals.image_7, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].invDimensions_6 * (float2x2(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.xy, _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].upper2x2_4.zw) * _12_pos + _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].translation_4));
_out.sk_FragColor = outColor_1;
return _out;
}
