#include "NativeGlassLighting.hpp"
#include "NativeGlassProfile.hpp"
#include <BackdropEffect.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace NXKit::Internal {

std::string_view nativeGlassLightingUniforms() {
    return R"sksl(
uniform float2 nativeOutputDistance;
uniform float4 nativeShadowShape;
uniform float2 nativeShadowOffset;
uniform float4 nativeShadowTransfer;
uniform float4 nativeShadowFill;
uniform float4 nativeShadowResponse;
uniform float4 nativeRingShadow;
uniform float nativeRingShadowOffset;
uniform float4 nativeBleedShape;
uniform float4 nativeBleedTransfer;
uniform float2 nativeBleedDistance;
uniform float4 nativeFilterHighlight;
uniform float3 nativeFilterHighlightColor;
uniform float4 nativeKeyHighlight;
uniform float4 nativeFillHighlight;
uniform float4 nativeKeyHighlightShape;
uniform float4 nativeFillHighlightShape;
uniform float4 nativeKeyHighlightColor;
uniform float4 nativeFillHighlightColor;
uniform float4 nativeHighlightResponse;
uniform float4 nativeHighlightMatrix0;
uniform float4 nativeHighlightMatrix1;
uniform float4 nativeHighlightMatrix2;
uniform float4 nativeHighlightMatrix3;
uniform float4 nativeHighlightMatrixBias;
uniform float4 nativeHighlightMatrixOptions;
uniform float3 nativeLightingClamp;
uniform float nativeLightingDisplayHeadroom;
uniform float4 nativeHoldingTone;
uniform float3 nativeFilterOptions;
)sksl";
}

std::string_view nativeGlassLightingShaderSource() {
    return R"sksl(
// Native distances remain view-local points. Scalar equations are independently
// translated from shipped QuartzCore AIR; source capture/pyramid sampling and
// geometry-field generation remain the portable renderer's responsibilities.
float nativeLightingAA() { return 0.5 / max(contentScale, 1.0); }
float nativeLightingHeadroom() { return min(nativeLightingClamp.z,nativeLightingDisplayHeadroom); }
float nativeLightingLuma(float3 c) { return dot(c, float3(0.2126, 0.7152, 0.0722)); }
float3 nativeLightingUnpremul(half4 c) {
    return float3(c.rgb) / max(float(c.a), 0.000001);
}
float nativeLightingRamp(float x, float a, float b) {
    float span = b-a;
    if (abs(span) < 0.00001) return step(a,x);
    float t = clamp((x-a)/span,0.0,1.0);
    return t*t*(3.0-2.0*t);
}
float nativeGlassOutputCoverage(float sdf) {
    float aa = nativeLightingAA();
    return smoothstep(nativeOutputDistance.x-aa,nativeOutputDistance.x+aa,sdf)
         * (1.0-smoothstep(nativeOutputDistance.y-aa,nativeOutputDistance.y+aa,sdf));
}
float3 nativeLightingTransfer(float3 c, float black, float white, float saturation) {
    float luma = nativeLightingLuma(c);
    return float3(black+(white-black)*luma)+saturation*(c-float3(luma));
}
float3 nativeLightingClampColor(float3 c, float limit, float preserveHue) {
    // This is the material's extended-range clamp. The SDR attachment clamp
    // is a separate pass boundary; applying it here destroys negative vibrant
    // matrix components before their source-over operation.
    if (limit <= 0.0) return c;
    if (preserveHue > 0.5) {
        float maximum=max(max(c.r,c.g),c.b);
        return maximum>limit?c*(limit/maximum):c;
    }
    return clamp(c,float3(-.75),float3(limit));
}
float nativeLightingErfc(float valueInput) {
    // Bounded polynomial used by the shipped shader's half-precision erf
    // survival approximation. Radius conversion belongs to each caller.
    half x=half(clamp(valueInput,-2.0,2.0));
    half square=x*x;
    half value=half(.0029544830322265625)*square+half(-.034454345703125);
    value=value*square+half(.168212890625);
    value=value*square+half(-.560546875);
    return clamp(float(value*x+half(.5)),0.0,1.0);
}
float nativeLightingGaussianCoverage(float distance,float sigma) {
    if(sigma<=.000001) return 1.0-smoothstep(-nativeLightingAA(),nativeLightingAA(),distance);
    return nativeLightingErfc(distance/(sigma*1.4142135623730951));
}
float nativeLightingRingAlpha(float sdf,float shiftedRingSDF,float2 normal) {
    if(nativeRingShadow.w<=0.0 || nativeRingShadow.y<=0.0) return 0.0;
    float inverseRadius=1.0/max(nativeRingShadow.x,.000001);
    float start=shiftedRingSDF*inverseRadius;
    float finish=(shiftedRingSDF+nativeRingShadow.y)*inverseRadius;
    float ring=clamp(nativeLightingErfc(start*.7071067811865476)
                    -nativeLightingErfc(finish*.7071067811865476),0.0,1.0);
    float width=max((abs(normal.x)+abs(normal.y))/max(contentScale,1.0),.0001);
    float coverage=clamp(.5-sdf/width,0.0,1.0);
    return ring*nativeRingShadow.w*mix(1.0,coverage,clamp(nativeRingShadow.z,0.0,1.0));
}
half4 nativeLightingOver(half4 foreground, half4 background) {
    return foreground + background*(1.0-foreground.a);
}
// Equations independently translated from the shipped QuartzCore AIR
// sdf_diffuse_kf_highlight kernel. Amount is Schlick bias, not paint opacity.
float nativeLightingBias(float coverage,float amount) {
    amount=clamp(amount,0.0,1.0);
    if(amount<=0.0 || coverage<=0.0) return 0.0;
    float coefficient=1.0/max(amount,.000001)-2.0;
    return coverage/max(1.0+coefficient*(1.0-coverage),.000001);
}
float nativeLightingSDFBand(float sdf,float height,float curvature,float2 normal) {
    if(height<=0.0) return 0.0;
    float depth=-sdf;
    if(depth< -5.0) return 0.0;
    float t=clamp(depth/height,0.0,1.0);
    float shape=mix(t<1.0?1.0:0.0,1.0-t,curvature);
    // fwidth(distance) for the supplied unit SDF gradient, in device pixels.
    // Isolated uniform probes cannot use derivatives of a constant test SDF.
    float width=max((abs(normal.x)+abs(normal.y))/max(contentScale,1.0),.0001);
    return shape*clamp(depth/width+.5,0.0,1.0)
                *clamp((height-depth)/width+.5,0.0,1.0);
}
float nativeLightingSDFDirection(float2 normal,float angle,float spread) {
    float threshold=cos(clamp(abs(spread),.0001,3.141592654));
    float cosine=dot(normal,float2(cos(angle-1.5707963267948966),
                                  sin(angle-1.5707963267948966)));
    return clamp((cosine-threshold)/max(1.0-threshold,.000001),0.0,1.0);
}
float nativeGlassFilterHighlight(float sdf,float2 normal,float localScale) {
    if(nativeFilterHighlight.x<=0.0 || nativeFilterHighlight.z<=0.0) return 0.0;
    float spread=nativeLightingHeadroom()<=1.0
        ?nativeFilterHighlightColor.z:nativeFilterHighlight.w;
    float threshold=cos(clamp(abs(spread),.0001,3.141592654));
    float cosine=dot(normal,float2(sin(nativeFilterHighlight.y),-cos(nativeFilterHighlight.y)));
    float2 angular=clamp((float2(cosine,-cosine)-threshold)/max(1.0-threshold,.000001),0.0,1.0);
    float2 localNormal=normal*max(contentScale,1.0)/max(localScale,1.0);
    float band;
    if(nativeFilterOptions.z>.5) {
        // The native outside/stroke mode uses geometric coverage rather
        // than a curvature lobe. Its opposite edge is the shape boundary.
        float width=max((abs(localNormal.x)+abs(localNormal.y))/max(contentScale,1.0),.0001);
        float coverage=clamp(.5-sdf/width,0.0,1.0);
        float depth=-(sdf+nativeFilterHighlightColor.x);
        band=(1.0-coverage)*clamp(depth/width+.5,0.0,1.0);
    } else {
        band=nativeLightingSDFBand(sdf+nativeFilterHighlightColor.x,nativeFilterHighlight.z,.75,
            localNormal);
    }
    return nativeLightingBias(band*angular.x,nativeFilterHighlight.x)
          +nativeLightingBias(band*angular.y,nativeFilterHighlight.x);
}
half4 nativeGlassHoldingTone(float sdf,half4 material) {
    if(nativeHoldingTone.x<=0.0) return material;
    float span=nativeHoldingTone.w-nativeHoldingTone.z;
    float progress=abs(span)>.000001?clamp((sdf-nativeHoldingTone.z)/span,0.0,1.0):0.0;
    float weight=nativeHoldingTone.x*(1.0-progress);
    float alpha=clamp(float(material.a),0.0,1.0);
    float3 target=float3(material.rgb)*nativeHoldingTone.y*alpha/max(float(material.a),.000001);
    return mix(material,half4(half3(target),half(alpha)),half(weight));
}
// material is the premultiplied glassBackground result BEFORE it is composed
// over original. The native face-extension/cubic pass follows body+shadow+ring
// coverage, then holding tone. The caller applies accent and CASDF afterwards.
half4 nativeGlassBackgroundFinalize(float sdf,half4 material,half4 original,
                                   half3 exteriorFace,float faceOpacity,
                                   float2 normal,float localScale) {
    float amount=nativeGlassFilterHighlight(sdf,normal,localScale);
    if(amount>.000001) {
        float3 raw=nativeLightingUnpremul(original);
        float3 face=float3(exteriorFace);
        if(nativeFilterOptions.x>.5) face=nativeFilterHighlightColor.y<0.0?min(face,raw):max(face,raw);
        float alpha=float(material.a);
        float3 color=float3(material.rgb)+(1.0-alpha)*mix(raw,face,faceOpacity*amount);
        color*=1.0+nativeFilterHighlightColor.y*amount*(3.0-2.0*color);
        float resultAlpha=alpha+(1.0-alpha)*amount*nativeFilterOptions.y;
        material=half4(half3(color-(1.0-resultAlpha)*raw),half(resultAlpha));
    }
    return nativeGlassHoldingTone(sdf,material);
}
// Backdrop inputs are premultiplied; returned color is a premultiplied
// exterior overlay. Shadow/ring SDFs must be evaluated at actual shifted
// coordinates, not approximated by a normal-dot-offset at rounded corners.
half4 nativeGlassExterior(float sdf, float shiftedShadowSDF, float shiftedRingSDF,
                         float2 normal, half4 backdrop, half4 blurredBackdrop,
                         half4 bleedBackdrop) {
    float aa = nativeLightingAA();
    float outside = smoothstep(-aa,aa,sdf);
    float outputMask = nativeGlassOutputCoverage(sdf);
    if (outputMask <= 0.0) return half4(0.0);
    float3 shadowSource = nativeLightingUnpremul(blurredBackdrop);
    half4 result = half4(0.0);
    if (nativeShadowShape.w > 0.0 || nativeShadowResponse.w > 0.0) {
        // shadowRadius is the erf radius, sqrt(2) times Gaussian sigma.
        // shadowBlurRadius selects the independently sampled source LOD.
        float coverage=nativeLightingErfc(shiftedShadowSDF/max(nativeShadowShape.x,.000001));
        float sourceLuma=nativeLightingLuma(shadowSource);
        float3 linear=(nativeShadowTransfer.y-nativeShadowTransfer.x)*float3(sourceLuma)
            +nativeShadowTransfer.z*(shadowSource-float3(sourceLuma));
        float fillAlpha=clamp(nativeShadowFill.a,0.0,1.0);
        float3 bias=float3(nativeShadowTransfer.x)*(1.0-fillAlpha)+nativeShadowFill.rgb*fillAlpha;
        float contribution=clamp(nativeShadowResponse.z,0.0,1.0);
        float3 transformed=linear*(1.0-fillAlpha)*contribution+bias;
        float nativeAlpha=mix(fillAlpha,1.0,contribution);
        float opacity=clamp(nativeShadowShape.w,0.0,1.0);
        // The native contour field supplies shadow coverage. Backdrop alpha
        // only participates in unpremultiplying its sampled color.
        float alpha=coverage*opacity*nativeAlpha;
        // RGB already contains the matrix's premultiplied fill bias.
        transformed/=max(nativeAlpha,.000001);
        result = half4(half3(transformed*alpha),half(alpha));
    }
    return result*half(outputMask);
}
half4 nativeGlassExteriorComposite(float sdf,float shiftedShadowSDF,float shiftedRingSDF,
                                  float2 normal,half4 backdrop,half4 shadowBackdrop,
                                  half4 bleedBackdrop,float2 localSize,float localScale,
                                  half3 exteriorFace,float faceOpacity) {
    half4 field=nativeGlassExterior(sdf,shiftedShadowSDF,shiftedRingSDF,normal,
        backdrop,shadowBackdrop,bleedBackdrop);
    float ring=clamp(nativeLightingRingAlpha(sdf,shiftedRingSDF,normal),0.0,1.0);
    field=nativeLightingOver(half4(0,0,0,half(ring)),field);
    field=nativeGlassBackgroundFinalize(sdf,field,backdrop,exteriorFace,faceOpacity,normal,localScale);
    return nativeLightingOver(field,backdrop);
}
half4 nativeGlassExteriorComposite(float sdf,float shiftedShadowSDF,float shiftedRingSDF,
                                  float2 normal,half4 backdrop,half4 shadowBackdrop,
                                  half4 bleedBackdrop,float2 localSize,float localScale) {
    return nativeGlassExteriorComposite(sdf,shiftedShadowSDF,shiftedRingSDF,normal,
        backdrop,shadowBackdrop,bleedBackdrop,localSize,localScale,
        half3(nativeLightingUnpremul(backdrop)),0.0);
}
half3 nativeGlassBleed(float sdf,half3 body,half4 bleedBackdrop) {
    float3 color=float3(body);
    if(sdf>nativeLightingAA()) return body;
    if(nativeBleedShape.w>0) {
        // AIR glass_background_e_lpf: literal SDF ramp, squared, with a
        // fourth-power post-face luminance response. bleed_darken is a
        // packed luminance affine transform, not a minimum-color blend.
        float luminance=clamp(dot(color,float3(.2125,.7154,.0721)),0.0,1.0);
        float tone=nativeBleedTransfer.w>.5?luminance:1.0-luminance;
        float span=nativeBleedDistance.y-nativeBleedDistance.x;
        float edge=abs(span)>.000001
            ?clamp((sdf-nativeBleedDistance.x)/span,0.0,1.0)
            :(sdf<=nativeBleedDistance.x?1.0:0.0);
        float gain=tone*tone*edge;gain*=gain;
        float3 bleed=nativeLightingTransfer(nativeLightingUnpremul(bleedBackdrop),
            nativeBleedTransfer.x,nativeBleedTransfer.y,nativeBleedTransfer.z);
        color=mix(color,bleed,clamp(gain*nativeBleedShape.w,0,1));
    }
    return half3(color);
}
half4 nativeGlassRingComposite(float sdf,float shiftedRingSDF,float2 normal,
                              float fieldAlpha,half4 material) {
    float alpha=clamp(nativeLightingRingAlpha(sdf,shiftedRingSDF,normal)*fieldAlpha,0.0,1.0);
    return nativeLightingOver(half4(0,0,0,half(alpha)),material);
}
half3 nativeGlassInterior(float sdf,float2 normal,half3 body,half4 bleedBackdrop,
                          float2 localSize,float shiftedRingSDF,half4 originalBackdrop) {
    float3 color=float3(nativeGlassBleed(sdf,body,bleedBackdrop));
    color*=1.0-clamp(nativeLightingRingAlpha(sdf,shiftedRingSDF,normal),0.0,1.0);
    return half3(color);
}
half3 nativeGlassInterior(float sdf,float2 normal,half3 body,half4 bleedBackdrop,
                          float2 localSize,float shiftedRingSDF) {
    return nativeGlassInterior(sdf,normal,body,bleedBackdrop,localSize,shiftedRingSDF,bleedBackdrop);
}
half3 nativeGlassInterior(float sdf,float2 normal,half3 body,half4 bleedBackdrop,float2 localSize) {
    // Geometry-free compatibility probes have only a local tangent available.
    // Production callers pass the exact shifted contour to the overload above.
    return nativeGlassInterior(sdf,normal,body,bleedBackdrop,localSize,
                               sdf-normal.y*nativeRingShadowOffset);
}
half3 nativeGlassInterior(float sdf,float2 normal,half3 body,half4 bleedBackdrop) {
    // Retain the geometry-free helper for isolated native parameter probes.
    return nativeGlassInterior(sdf,normal,body,bleedBackdrop,float2(max(nativeBleedShape.z,0.0)*2));
}
float nativeLightingAngular(float2 normal, float angle, float spread) {
    float cosine = clamp(dot(normal,float2(cos(angle),sin(angle))),-1.0,1.0);
    float angularDistance = acos(cosine);
    float extent = clamp(abs(spread),0.0001,3.141592654);
    float t = clamp(1.0-angularDistance/extent,0.0,1.0);
    return t*t*(3.0-2.0*t);
}
float nativeLightingSDFAngular(float2 normal,float angle,float spread) {
    // CASDF key/fill angles use zero at the top of the material.
    return nativeLightingAngular(normal,angle-1.5707963267948966,spread);
}
float nativeLightingLobe(float sdf, float height, float curvature) {
    if (height <= 0.0) return 0.0;
    float t = clamp(1.0-abs(sdf)/max(height,nativeLightingAA()),0.0,1.0);
    return pow(t,max(curvature,0.0001));
}
float4 nativeLightingHighlightMatrix(float4 c) {
    float4 result = float4(dot(nativeHighlightMatrix0,c),dot(nativeHighlightMatrix1,c),
        dot(nativeHighlightMatrix2,c),dot(nativeHighlightMatrix3,c))+nativeHighlightMatrixBias;
    result.rgb = nativeLightingClampColor(result.rgb,nativeHighlightMatrixOptions.x,
        nativeHighlightMatrixOptions.y);
    return result;
}
half3 nativeGlassHighlight(float sdf, float2 localNormal, float2 globalNormal, half3 backdrop) {
    float3 color = float3(backdrop);
    float2 normal = nativeHighlightResponse.w > 0.5 ? globalNormal : localNormal;
    float norm = length(normal);
    if (norm > 0.00001) normal /= norm;
    float keyHeight = max(0.0,nativeKeyHighlight.y*nativeKeyHighlightShape.x+nativeKeyHighlightShape.y);
    float fillHeight = max(0.0,nativeFillHighlight.y*nativeFillHighlightShape.x+nativeFillHighlightShape.y);
    float keySpread = nativeKeyHighlight.w*nativeKeyHighlightShape.z+nativeKeyHighlightShape.w;
    float fillSpread = nativeFillHighlight.w*nativeFillHighlightShape.z+nativeFillHighlightShape.w;
    float curvature = nativeHighlightResponse.x;
    float key = 0.0;
    float fill = 0.0;
    float localSDF=nativeHighlightResponse.w>.5?sdf/max(norm,.000001):sdf;
    if (nativeKeyHighlight.z > 0.0) {
        float primary=nativeLightingSDFBand(localSDF,keyHeight,curvature,normal)
            *nativeLightingSDFDirection(normal,nativeKeyHighlight.x,keySpread);
        float diffuse=nativeLightingSDFBand(localSDF,keyHeight*nativeHighlightMatrixOptions.w,1.0,normal)
            *nativeLightingSDFDirection(normal,nativeKeyHighlight.x,keySpread*nativeHighlightResponse.z);
        key=nativeLightingBias(primary,nativeKeyHighlight.z)
           +nativeLightingBias(diffuse,nativeKeyHighlight.z*nativeHighlightResponse.y);
    }
    if (nativeFillHighlight.z > 0.0) {
        float primary=nativeLightingSDFBand(localSDF,fillHeight,curvature,normal)
            *nativeLightingSDFDirection(normal,nativeFillHighlight.x,fillSpread);
        float diffuse=nativeLightingSDFBand(localSDF,fillHeight*nativeHighlightMatrixOptions.w,1.0,normal)
            *nativeLightingSDFDirection(normal,nativeFillHighlight.x,fillSpread*nativeHighlightResponse.z);
        fill=nativeLightingBias(primary,nativeFillHighlight.z)
            +nativeLightingBias(diffuse,nativeFillHighlight.z*nativeHighlightResponse.y);
    }
    // inputShadowAmount displaces the exterior shadow's source sample. It
    // is not an independent interior darkening pass.
    // The attached CASDFKeyFillHighlightEffect is an overlay above the
    // glassBackground filter in the native layer tree. Apply that overlay
    // after the filter's internal highlight/shadow, so its specular light
    // cannot be darkened again by inputKeyFillHighlightColorBias.
    float3 unlitColor=color;
    // vibrantColorMatrix transforms its destination; source RGB is ignored.
    // Only the CASDF effect's premultiplied alpha carries key/fill coverage.
    float4 transformed=nativeLightingHighlightMatrix(float4(unlitColor,1.0));
    float coverage=key*nativeKeyHighlightColor.a+fill*nativeFillHighlightColor.a;
    coverage*=clamp(transformed.a,0.0,1.0);
    color=unlitColor+coverage*(transformed.rgb-unlitColor);
    return half3(nativeLightingClampColor(color,min(nativeLightingClamp.x,nativeLightingHeadroom()),
        nativeLightingClamp.y));
}
half3 nativeGlassHighlight(float sdf, float2 normal, half3 backdrop) {
    return nativeGlassHighlight(sdf,normal,normal,backdrop);
}
)sksl";
}

namespace {
void vector(BackdropEffect& effect, const char* name, NXFloat a,NXFloat b,NXFloat c,NXFloat d) {
    effect.setUniform(name,std::array<NXFloat,4>{a,b,c,d});
}
void color(BackdropEffect& effect,const char* name,const NativeGlassColor& value) {
    vector(effect,name,value.r,value.g,value.b,value.a);
}
}

void bindNativeGlassLighting(const NativeGlassProfile& p, BackdropEffect& effect, NXFloat displayHeadroom) {
    effect.setUniform("nativeOutputDistance",NXPoint(p.output.minimum,p.output.maximum));
    vector(effect,"nativeShadowShape",p.shadowRadius,p.shadowBlurRadius,p.shadowDistanceOffset,p.shadowOpacity);
    effect.setUniform("nativeShadowOffset",NXPoint(p.shadowOffset.x,p.shadowOffset.y));
    vector(effect,"nativeShadowTransfer",p.shadowColorMatrixBlack,p.shadowColorMatrixWhite,p.shadowColorMatrixSaturation,0);
    color(effect,"nativeShadowFill",p.shadowColorMatrixFillColor);
    vector(effect,"nativeShadowResponse",p.shadowAmount,p.shadowHeight,p.shadowVibrancyContribution,p.sdrShadowOpacity);
    vector(effect,"nativeRingShadow",p.ringShadowBlurRadius,p.ringShadowStrokeWidth,p.ringShadowMask,p.ringShadowOpacity);
    effect.setUniform("nativeRingShadowOffset",p.ringShadowOffset);
    vector(effect,"nativeBleedShape",p.bleedAmount,p.bleedBlurRadius,p.bleedHeight,p.bleedOpacity);
    vector(effect,"nativeBleedTransfer",p.bleedColorMatrixBlack,p.bleedColorMatrixWhite,p.bleedColorMatrixSaturation,p.bleedDarkenBlend?1:0);
    effect.setUniform("nativeBleedDistance",NXPoint(p.bleedDistance0,p.bleedDistance1));
    vector(effect,"nativeFilterHighlight",p.keyFillHighlightAmount,p.keyFillHighlightAngle,p.keyFillHighlightHeight,p.keyFillHighlightSpread);
    effect.setUniform("nativeFilterHighlightColor",std::array<NXFloat,3>{p.keyFillHighlightEffectOffset,p.keyFillHighlightColorBias,p.keyFillHighlightSpreadSDR});
    const auto& h=p.highlight;
    vector(effect,"nativeKeyHighlight",h.keyAngle,h.keyHeight,h.keyAmount,h.keySpread);
    vector(effect,"nativeFillHighlight",h.fillAngle,h.fillHeight,h.fillAmount,h.fillSpread);
    vector(effect,"nativeKeyHighlightShape",h.keyHeightScale,h.keyHeightOffset,h.keySpreadScale,h.keySpreadOffset);
    vector(effect,"nativeFillHighlightShape",h.fillHeightScale,h.fillHeightOffset,h.fillSpreadScale,h.fillSpreadOffset);
    color(effect,"nativeKeyHighlightColor",h.keyColor);
    color(effect,"nativeFillHighlightColor",h.fillColor);
    vector(effect,"nativeHighlightResponse",h.curvature,h.diffuseAmountScale,h.diffuseSpreadScale,h.global?1:0);
    const auto& m=h.vibrant.matrix.values;
    vector(effect,"nativeHighlightMatrix0",m[0],m[1],m[2],m[3]);
    vector(effect,"nativeHighlightMatrix1",m[5],m[6],m[7],m[8]);
    vector(effect,"nativeHighlightMatrix2",m[10],m[11],m[12],m[13]);
    vector(effect,"nativeHighlightMatrix3",m[15],m[16],m[17],m[18]);
    vector(effect,"nativeHighlightMatrixBias",m[4],m[9],m[14],m[19]);
    vector(effect,"nativeHighlightMatrixOptions",h.vibrant.clamp,h.vibrant.clampPreserveHue?1:0,
        !h.vibrant.hasBackdropAware||h.vibrant.backdropAware?1:0,h.diffuseHeightScale);
    effect.setUniform("nativeLightingClamp",std::array<NXFloat,3>{p.clamp,p.clampPreserveHue?1.0f:0.0f,p.maxHeadroom});
    effect.setUniform("nativeLightingDisplayHeadroom",std::isfinite(displayHeadroom)?std::max<NXFloat>(1,displayHeadroom):1);
    vector(effect,"nativeHoldingTone",p.sdrHoldingToneEnabled?1.f:0.f,
        p.sdrHoldingToneWhite,p.sdrGradientDistance0,p.sdrGradientDistance1);
    effect.setUniform("nativeFilterOptions",std::array<NXFloat,3>{p.keyFillHighlightColorBias!=0?1.f:0.f,
        p.keyFillHighlightEffectOffset<0?1.f:0.f,
        p.keyFillHighlightEffectOffset<0 && std::abs(p.keyFillHighlightHeight+p.keyFillHighlightEffectOffset)<.001f?1.f:0.f});

}

NXFloat nativeGlassExteriorOutset(const NativeGlassProfile& p) {
    // CASDFOutputEffect's explicit output mask is authoritative. No unbounded
    // Gaussian tail may extend the draw rectangle beyond the captured mask.
    return std::isfinite(p.output.maximum)?std::max<NXFloat>(0,p.output.maximum):0;
}
NXPoint nativeGlassShadowOffset(const NativeGlassProfile& p) { return {p.shadowOffset.x,p.shadowOffset.y}; }
NXFloat nativeGlassRingOffset(const NativeGlassProfile& p) { return p.ringShadowOffset; }
}
