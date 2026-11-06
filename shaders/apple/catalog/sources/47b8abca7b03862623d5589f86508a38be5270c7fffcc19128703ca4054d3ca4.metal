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
    float4 jacobian [[user(locn2)]];
    float4 edgeDistances [[user(locn3)]];
    float4 xRadii [[user(locn4) flat]];
    float4 yRadii [[user(locn5) flat]];
    float2 strokeParams [[user(locn6) flat]];
    float2 perPixelControl [[user(locn7)]];
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
void K_Qvf2f22f2f2f2f2(thread float2& a, float2x2 b, float2 c, float2 d, float2 e, float2 f);
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g);
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0.xy), float2(x0.zw));
}
void K_Qvf2f22f2f2f2f2(thread float2& a, float2x2 b, float2 c, float2 d, float2 e, float2 f) {
float2 g = f - d;
if (all((g > float2(0.0)))) if (all((f > float2(0.0))) || c.x > 0.0 && c.y < 0.0) {
float2 _83_a = g * e;
float2 _84_e = 1.0 / (f * f + c.x * c.x);
float2 _85_g = _84_e * _83_a;
float2 _86_c = _85_g * b;
float _87_h = rsqrt(dot(_86_c, _86_c));
float _88_i = (0.5 * _87_h) * (dot(_83_a, _85_g) - 1.0);
float _89_j = ((f.x * c.x) * _84_e.x) * _87_h;
float2 h = float2(_89_j - _88_i, _89_j + _88_i);
h.y = f.x - c.x <= 0.0 ? 1.0 : -h.y;
a = min(a, h);
} else if (c.y == 0.0) {
float2 _90_c = e * b;
float h = ((c.x - g.x) - g.y) * rsqrt(dot(_90_c, _90_c));
a.x = min(a.x, h);
}
}
void L_Qvf2f22f2f4f4f4(thread float2& a, float2x2 b, float2 c, float4 e, float4 f, float4 g) {
    float2 _skTemp0;
    float2 _skTemp1;
    float2 _skTemp2;
    float2 _skTemp3;
((K_Qvf2f22f2f2f2f2((_skTemp0 = a), b, c, e.xy, float2(-1.0), float2(f.x, g.x))), (a = _skTemp0));
((K_Qvf2f22f2f2f2f2((_skTemp1 = a), b, c, e.zy, float2(1.0, -1.0), float2(f.y, g.y))), (a = _skTemp1));
((K_Qvf2f22f2f2f2f2((_skTemp2 = a), b, c, e.zw, float2(1.0), float2(f.z, g.z))), (a = _skTemp2));
((K_Qvf2f22f2f2f2f2((_skTemp3 = a), b, c, e.xw, float2(-1.0, 1.0), float2(f.w, g.w))), (a = _skTemp3));
}
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
    float2 _skTemp4;
_globals.uniformSsboIndex = _in.ssboIndexVar;
float2 _5_halfDim = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowSize_1 * 0.5;
float2 _6_p = (_in.localCoordsVar - _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowOffset_1) - _5_halfDim;
float _7_r = min(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerRadius_1, min(_5_halfDim.x, _5_halfDim.y));
float _8_d = _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].cornerCurve_1 < 0.5 || _7_r <= 0.0 ? circularBoxSDF_1_ff2f2f(_6_p, _5_halfDim, _7_r) : continuousCornerSample_1_f3f2f2(_globals, _6_p, _5_halfDim).x;
float _9_z = _8_d / _globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowSigma_1;
float _10_t = 1.0 / (1.0 + 0.2316419 * abs(_9_z));
float _11_q = ((0.3989423 * exp((-0.5 * _9_z) * _9_z)) * _10_t) * (0.319381535 + _10_t * (-0.356563777 + _10_t * (1.78147793 + _10_t * (-1.82125592 + _10_t * 1.33027446))));
half4 outColor_1 = half4(0.0h, 0.0h, 0.0h, half(_globals._anonInterface0->combinedUniformData[_globals.uniformSsboIndex].shadowOpacity_1 * (_9_z >= 0.0 ? _11_q : 1.0 - _11_q)));
half4 outputCoverage = half4(1.0h);
half4 _12_analytic_rrect_coverage_fn;
if (_in.perPixelControl.x > 0.0) _12_analytic_rrect_coverage_fn = half4(1.0h); else if (_in.perPixelControl.y > 1.0) {
float2 _13_h = min(_in.edgeDistances.xy, _in.edgeDistances.zw);
float _14_i = min(_13_h.x, _13_h.y) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _15_j = (_in.perPixelControl.y - 1.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _16_k = 1.0 - 0.5 * _15_j;
_12_analytic_rrect_coverage_fn = half4(half(saturate(_15_j * (_14_i + _16_k))));
} else {
float2x2 _17_h = float2x2_from_float4(_in.jacobian) * (1.0 / float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w);
float2 _18_c = float2(1.0, 0.0) * _17_h;
float2 _19_c = float2(0.0, 1.0) * _17_h;
float2 _20_i = float2(rsqrt(dot(_18_c, _18_c)), rsqrt(dot(_19_c, _19_c)));
float2 _21_j = _20_i * (_in.strokeParams.x + min(_in.edgeDistances.xy, _in.edgeDistances.zw));
float2 _22_k = float2(min(_21_j.x, _21_j.y), -1.0);
float _23_l;
float _24_m;
if (_in.perPixelControl.x > -0.95) {
float2 _25_n = _20_i * ((_in.edgeDistances.xy + _in.edgeDistances.zw) + 2.0 * _in.strokeParams.xx);
_23_l = min(min(_25_n.x, _25_n.y), 1.0);
_24_m = 1.0 - 0.5 * _23_l;
} else {
float2 _26_n = (2.0 * _in.strokeParams.x) * _20_i;
float2 _27_o = _26_n - _21_j;
_22_k.y = -max(_27_o.x, _27_o.y);
if (_in.strokeParams.x > 0.0) {
float _28_p = min(_26_n.x, _26_n.y);
float2 _29_q = select(float2(_28_p), _26_n, (_27_o >= float2(-0.5)));
_23_l = saturate(max(_29_q.x, _29_q.y));
_24_m = 1.0 - 0.5 * _23_l;
} else _23_l = (_24_m = 1.0);
}
((L_Qvf2f22f2f4f4f4((_skTemp4 = _22_k), _17_h, _in.strokeParams, _in.edgeDistances, _in.xRadii, _in.yRadii)), (_22_k = _skTemp4));
float _30_n = min(_in.perPixelControl.y, 0.0) * float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w).w;
float _31_o = _23_l * (min(_22_k.x + _30_n, -_22_k.y) + _24_m);
_12_analytic_rrect_coverage_fn = half4(half(saturate(_31_o)));
}
outputCoverage = _12_analytic_rrect_coverage_fn;
_out.sk_FragColor = outColor_1 * outputCoverage;
return _out;
}
