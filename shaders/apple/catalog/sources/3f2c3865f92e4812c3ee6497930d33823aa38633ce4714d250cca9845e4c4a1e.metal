#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct CombinedUniformData {
float2 shadowSize_1;
float cornerRadius_1;
float cornerCurve_1;
float shadowSigma_1;
float shadowOpacity_1;
float2 shadowOffset_1;
float4 cornerTop_1;
float4 cornerTopCircle_1;
float4 cornerRight_1;
float4 cornerRightCircle_1;
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
};
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float3 continuousOctant_1_f3f2f4f4(float2 p, float4 se, float4 circle);
float3 continuousCornerSample_1_f3f2f2(thread Globals& _globals, float2 p, float2 halfDim);
float boxRoundedSDF_1_ff2f2f(float2 p, float2 halfDim, float r);
float glassShadowTail_1_ff(float z);
float circularBoxSDF_1_ff2f2f(float2 p, float2 halfDim, float r) {
float2 q = (abs(p) - halfDim) + r;
return (length(max(q, 0.0)) + min(max(q.x, q.y), 0.0)) - r;
}
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device CombinedUniforms& _anonInterface0 [[buffer(1)]], constant IntrinsicUniforms& _anonInterface1 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
Globals _globals{&_anonInterface0, {}, &_anonInterface1};
(void)_globals;
Outputs _out;
(void)_out;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _5_halfDim = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowSize_1 * 0.5;
float2 _6_p = (_in.localCoordsVar - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowOffset_1) - _5_halfDim;
float _7_r = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRadius_1, min(_5_halfDim.x, _5_halfDim.y));
float _8_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || _7_r <= 0.0 ? circularBoxSDF_1_ff2f2f(_6_p, _5_halfDim, _7_r) : continuousCornerSample_1_f3f2f2(_globals, _6_p, _5_halfDim).x;
float _9_z = _8_d / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowSigma_1;
float _10_t = 1.0 / (1.0 + 0.2316419 * abs(_9_z));
float _11_q = ((0.3989423 * exp((-0.5 * _9_z) * _9_z)) * _10_t) * (0.319381535 + _10_t * (-0.356563777 + _10_t * (1.78147793 + _10_t * (-1.82125592 + _10_t * 1.33027446))));
half4 outColor_1 = half4(0.0h, 0.0h, 0.0h, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowOpacity_1 * (_9_z >= 0.0 ? _11_q : 1.0 - _11_q)));
_out.sk_FragColor = outColor_1;
return _out;
}
