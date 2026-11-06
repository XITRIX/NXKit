#include "NativeGlassShader.hpp"
#include "NativeGlassLighting.hpp"
#include "NativeGlassAccent.hpp"
#include "NativeGlassBlur.hpp"
#include "LayerCornerGeometry.hpp"
#include <algorithm>
#include <cmath>
#include <array>

namespace NXKit::Internal {
namespace {
const std::string source = std::string(R"sksl(
uniform float2 resolution;
uniform float2 lensCenter;
uniform float2 lensSize;
uniform float cornerRadius;
uniform float cornerCurve;
uniform float externalShapeCoverage;
uniform float maximumSampleRadius;
uniform float contentScale;
uniform float filterScale;
uniform float interactionIntensity;
uniform float2 interactionLocation;
// Shared grouping/ownership state and portable optical overrides.
uniform float bodyMode;
uniform float tintMode;
uniform float referenceAppearance;
uniform float backdropLuma;
uniform float4 tint;
uniform float nativeFaceBlack;
uniform float3 nativeFaceTransfer;
uniform float4 nativeFaceFill;
uniform float2 nativeFaceLuma;
uniform float nativeFaceOpacity;
uniform float3 nativeFillOpacity;
uniform float4 nativeBlurDistances;
uniform float4 nativeBlurOpacities;
uniform float4 nativeRefraction;
uniform float3 nativeRefractionMask;
uniform float2 nativeSDRGradient;
uniform float nativeDisplayHeadroom;
uniform float nativeAccentEnabled;
uniform float4 nativeAccent0;
uniform float4 nativeAccent1;
uniform float4 nativeAccent2;
uniform float3 nativeAccentDistances;
uniform float4 nativeAccentTiming0;
uniform float4 nativeAccentTiming1;
uniform float2 nativeAccentTimingKnown;
uniform float2 nativeAccentMaskOptions;
uniform float4 nativeAccentFill;
uniform float4 nativeAccentMatrix0;
uniform float4 nativeAccentMatrix1;
uniform float4 nativeAccentMatrix2;
uniform float4 nativeAccentMatrix3;
uniform float4 nativeAccentMatrixBias;
uniform float3 nativeAccentMatrixOptions;
uniform float nativeDispersion;
uniform float nativeEdgeAbsorption;
uniform float nativeTintOpacity;
uniform float nativeBrightBackdropShade;
uniform float nativeDarkSpecularGain;
uniform float nativeBrightSpecularGain;
uniform float nativeReflectionDistance;
uniform float nativeReflectionSpread;
uniform shader content;
uniform shader sharpContent;
uniform float4 backdropPyramidRect;
uniform float4 backdropPyramidInfo;
uniform float nativePyramidStretch;
uniform float nativeMinimumBlurRadius;
uniform float4 nativeBlurRadii;
const float SMOOTH_EDGE_PX = 1.5;
)sksl") + layerCornerShaderUniforms + std::string(nativeGlassLightingUniforms())
    + layerCornerShaderSource() + "float toBrightness(half3 c) { return dot(c,half3(.2126,.7152,.0722)); }\n"
    + std::string(nativeGlassLightingShaderSource()) + R"sksl(
)sksl" + std::string(nativeGlassBlurShaderSource()) + R"sksl(
float3 nativeUnpremul(half4 p) { return float3(p.rgb)/max(float(p.a),.000001); }
float nativeRamp(float x,float a,float b) {
    if(abs(b-a)<.00001) return step(a,x);
    float t=clamp((x-a)/(b-a),0,1);return t*t*(3-2*t);
}
float nativeLinearRamp(float x,float a,float b) {
    return abs(b-a)<.00001?step(a,x):clamp((x-a)/(b-a),0,1);
}
float nativeBlurCoverage(float d) {
    return nativeBlurOpacities.x
        +(nativeBlurOpacities.y-nativeBlurOpacities.x)*nativeLinearRamp(d,nativeBlurDistances.x,nativeBlurDistances.y)
        +(nativeBlurOpacities.z-nativeBlurOpacities.y)*nativeLinearRamp(d,nativeBlurDistances.y,nativeBlurDistances.z)
        +(nativeBlurOpacities.w-nativeBlurOpacities.z)*nativeLinearRamp(d,nativeBlurDistances.z,nativeBlurDistances.w);
}
float nativeSRGB(float x) { return x<=.0031308 ? x*12.92 : 1.055*pow(max(x,0),1.0/2.4)-.055; }
// Native framebuffer perturbations establish the quadratic luma response:
// q = c * (1 + (maxLuma - 1) * luma(c)), followed by endpoint gain/bias.
// Endpoint gain affects luminance only; chroma has its own saturation gain.
// Independent endpoint perturbations distinguish this from RGB contrast.
float3 nativeFace(float3 color,float sdf) {
    float luma=dot(color,float3(.2126,.7152,.0722));
    float sdr=1.0-step(1.00001,nativeDisplayHeadroom);
    float maxLuma=mix(nativeFaceLuma.x,nativeFaceLuma.y,sdr);
    // Native AIR establishes a separate chroma boost before the face matrix.
    // Preserve luminance while restoring chroma lost by the SDR compression.
    float compression=clamp(1+(maxLuma-1)*luma,0,1);
    float qLuma=luma*compression;
    float3 q=float3(qLuma)+(color*compression-float3(qLuma))*(1+.3*(1-compression));
    float3 result=nativeFaceBlack+(nativeFaceTransfer.x-nativeFaceBlack)*qLuma
        +nativeFaceTransfer.y*(q-float3(qLuma));
    result=mix(result,nativeFaceFill.rgb,nativeFaceFill.a);
    return result;
}
float4 nativeAccentMatrix(float4 color) {
    float4 result=float4(dot(nativeAccentMatrix0,color),dot(nativeAccentMatrix1,color),
        dot(nativeAccentMatrix2,color),dot(nativeAccentMatrix3,color))+nativeAccentMatrixBias;
    result.rgb=nativeLightingClampColor(result.rgb,nativeAccentMatrixOptions.x,nativeAccentMatrixOptions.y);
    return result;
}
float nativeCubic(float t,float p1,float p2) {
    float u=1-t;return 3*u*u*t*p1+3*u*t*t*p2+t*t*t;
}
float nativeTiming(float x,float4 control,float known) {
    if(x<=0 || x>=1) return clamp(x,0,1);
    if(known<.5) return x;
    float low=0,high=1,t=x;
    for(int i=0;i<14;++i) {
        float value=nativeCubic(t,control.x,control.z);
        if(value<x) low=t;else high=t;
        t=(low+high)*.5;
    }
    return nativeCubic(t,control.y,control.w);
}
float4 nativeGradientColor(float d) {
    float4 a=d<nativeAccentDistances.y?nativeAccent0:nativeAccent1;
    float4 b=d<nativeAccentDistances.y?nativeAccent1:nativeAccent2;
    float lo=d<nativeAccentDistances.y?nativeAccentDistances.x:nativeAccentDistances.y;
    float hi=d<nativeAccentDistances.y?nativeAccentDistances.y:nativeAccentDistances.z;
    float raw=abs(hi-lo)>.00001?clamp((d-lo)/(hi-lo),0,1):step(lo,d);
    float t=d<nativeAccentDistances.y?nativeTiming(raw,nativeAccentTiming0,nativeAccentTimingKnown.x)
        :nativeTiming(raw,nativeAccentTiming1,nativeAccentTimingKnown.y);
    if(nativeAccentMaskOptions.x>.5) {
        a.rgb*=a.a;b.rgb*=b.a;float4 c=mix(a,b,t);c.rgb/=max(c.a,.000001);return c;
    }
    return mix(a,b,t);
}
float nativeBevel(float distance,float height,float radius) {
    float t=clamp(-distance/max(height,.000001),0,1);
    return height>0 ? 1-sqrt(max(t*(2-t),0)) : 0;
}
)sksl" + R"sksl(
half4 nativeGlassBackgroundBody(float2 xy,float sdf,float2 normal,float radius) {
    float inside=nativeBevel(sdf,nativeRefraction.y,radius);
    float outside=nativeBevel(sdf,nativeRefraction.w,radius);
    float displacement=nativeRefraction.x*inside;
    float2 target=xy+normal*displacement;
    float effectiveRadius=nativeBlurRadii.x*max(nativeBlurCoverage(sdf+displacement),0);
    half4 sharp=nativeSharp(xy,target);
    half4 pixel=nativeAtRadius(xy,target,effectiveRadius);
    if(nativeDispersion>0 && abs(displacement)>.001) {
        float2 shift=normal*abs(displacement)*nativeDispersion*.3;
        half4 red=nativeAtRadius(xy,target+shift,effectiveRadius),blue=nativeAtRadius(xy,target-shift,effectiveRadius);
        pixel=half4(red.r,pixel.g,blue.b,pixel.a);
    }
    float3 rgb=nativeUnpremul(pixel);
    // glassBackground gives the unpremultiplied inner sample opaque material
    // coverage. Outer refraction subsequently mixes literal sampled RGBA.
    float bodyAlpha=1;
    if(nativeFillOpacity.x>0 || nativeFillOpacity.y>0 || nativeFillOpacity.z>0) {
        half4 fill;
        if(backdropPyramidInfo.w<0)fill=nativeSharp(xy,xy);
        else {
            float lod=nativeRadiusLOD(nativeBlurRadii.y);
            float2 world=nativeSamplingWorld(nativeSampleXY(xy,xy));
            float2 offset=float2(lod*.25/max(backdropPyramidInfo.z,.000001));
            fill=(nativePyramidAt(world+offset,lod)+nativePyramidAt(world-offset,lod))*.5;
        }
        float3 fillRGB=nativeUnpremul(fill);
        rgb=(1-nativeFillOpacity.y-nativeFillOpacity.z)*rgb
            +nativeFillOpacity.y*max(rgb,fillRGB)+nativeFillOpacity.z*min(rgb,fillRGB);
        rgb=mix(rgb,fillRGB,nativeFillOpacity.x);
    }
    float outerMask=nativeLinearRamp(sdf,nativeRefractionMask.x,nativeRefractionMask.y);
    if(nativeRefractionMask.z*outerMask>0) {
        float outerShift=nativeRefraction.z*outside;
        half4 outer=nativeAtRadius(xy,xy+normal*outerShift,
            nativeBlurRadii.x*max(nativeBlurCoverage(sdf+outerShift),0));
        rgb=mix(rgb,float3(outer.rgb),nativeRefractionMask.z*outerMask);
        bodyAlpha=mix(bodyAlpha,float(outer.a),nativeRefractionMask.z*outerMask);
    }
    float3 processed=nativeFace(rgb,sdf);
    rgb=mix(rgb,processed,nativeFaceOpacity);
    if(nativeBleedShape.w>0)
        rgb=float3(nativeGlassBleed(sdf,half3(rgb),nativeBleed(xy,
            xy+normal*nativeBleedShape.x*nativeBevel(sdf,nativeBleedShape.z,radius))));
    return half4(half3(rgb),half(bodyAlpha));
}
half4 nativeGlassDecorate(float2 xy,float sdf,float2 normal,float2 worldNormal,float radius,half4 material,float surfaceCoverage) {
    float3 rgb=nativeUnpremul(material);
    float materialAlpha=float(material.a);
    float inside=nativeBevel(sdf,nativeRefraction.y,radius);
    if(nativeAccentEnabled>.5 && tint.a>0) {
        float4 gradient=nativeGradientColor(sdf);
        float4 matrixInput=float4(rgb,material.a);
        float4 accent=nativeAccentMatrix(matrixInput);
        accent.a=clamp(accent.a,0,1);
        accent.rgb*=accent.a;
        float mask=gradient.a*surfaceCoverage;
        if(nativeAccentMaskOptions.y>.5) mask*=nativeAccentFill.a;
        else {
            accent.rgb=nativeAccentFill.rgb*nativeAccentFill.a+accent.rgb*(1-nativeAccentFill.a);
            accent.a=nativeAccentFill.a+accent.a*(1-nativeAccentFill.a);
        }
        accent*=clamp(mask*float(tint.a),0,1);
        float outAlpha=accent.a+materialAlpha*(1-accent.a);
        rgb=(accent.rgb+rgb*materialAlpha*(1-accent.a))/max(outAlpha,.000001);
        materialAlpha=outAlpha;
    } else if(nativeTintOpacity>=0) {
        rgb=mix(rgb,float3(tint.rgb),clamp(nativeTintOpacity*float(tint.a)*surfaceCoverage,0,1));
    }
    rgb*=1-nativeEdgeAbsorption*inside;
    rgb*=1-nativeBrightBackdropShade*smoothstep(.85,.98,dot(rgb,float3(.2126,.7152,.0722)));
    float3 unlit=rgb;
    rgb=float3(nativeGlassHighlight(sdf,normal,worldNormal,half3(rgb)));
    float specularGain=mix(nativeDarkSpecularGain,nativeBrightSpecularGain,
        clamp(dot(unlit,float3(.2126,.7152,.0722)),0,1));
    rgb=mix(unlit,rgb,specularGain);
    if(nativeReflectionDistance>0 && inside>0) {
        float2 tangent=float2(-normal.y,normal.x)*nativeReflectionSpread;
        half4 reflected=(nativeSharp(xy,xy+normal*nativeReflectionDistance+tangent)
            +nativeSharp(xy,xy+normal*nativeReflectionDistance-tangent))*.5;
        rgb=mix(rgb,nativeUnpremul(reflected),inside*inside*.1);
    }
    // The material is already composed over its original backdrop here.
    // Press lighting belongs to the glass surface, not the exterior pixels
    // retained for its shadow/rim. Brightening those pixels creates a halo.
    float glow = clamp(interactionIntensity, 0.0, 1.0) * surfaceCoverage;
    if (glow > 0.0) {
    float glowRadius = max(1.0, min(lensSize.x, lensSize.y) * 1.6);
    float sheen = 1.0 - smoothstep(0.0, glowRadius, length(xy - interactionLocation));
    // Native held-minus-settled samples separate regular neutral and tinted
    // responses: the neutral total white opacity is about .50/.44 in light/
    // dark, while explicit regular tints use .30 ambient plus the local sheen.
    // Clear retains its existing response.
    float ambientPressOpacity = bodyMode > 0.5 ? mix(0.14, 0.22, backdropLuma)
        : (tintMode > 1.5 ? 0.30 : mix(0.385, 0.47, backdropLuma));
    float3 restingResult = rgb;
    rgb = mix(rgb, float3(1.0), glow * ambientPressOpacity);
    rgb = mix(rgb, float3(1.0), glow * sheen * mix(0.07, 0.10, backdropLuma));
    // Long neutral platters brighten transmitted color instead of painting
    // the button's white press veil over the entire surface. Native four-tab
    // captures in both themes retain blue/brown chroma; over black the dark
    // interior rises from32 to44, rather than32 to121. Keep short buttons,
    // explicit tints and clear glass on their independently calibrated path.
    float shortSide = max(1.0, min(lensSize.x,lensSize.y));
    float longSide = max(lensSize.x,lensSize.y);
    float platterPress = (bodyMode < 0.5 && tintMode < 0.5 ? 1.0 : 0.0)
        * smoothstep(160.0,280.0,longSide) * smoothstep(2.5,4.0,longSide/shortSide);
    float peak = max(restingResult.r,max(restingResult.g,restingResult.b));
    float chromaticRange = peak-min(restingResult.r,min(restingResult.g,restingResult.b));
    float neutralPress = 1.0-smoothstep(0.01,0.10,chromaticRange);
    float lightAppearance = smoothstep(0.15,0.85,backdropLuma);
    float pressGain = mix(1.18-0.06*peak,1.06,lightAppearance);
    float3 colorPress = mix(float3(toBrightness(restingResult)),restingResult,mix(1.03,1.06,lightAppearance))*pressGain;
    colorPress += float3(neutralPress*mix(7.0/255.0,11.5/255.0,lightAppearance));
    rgb = mix(rgb,mix(restingResult,colorPress,glow),platterPress);
    }
    rgb=nativeLightingClampColor(rgb,min(nativeLightingClamp.x,nativeDisplayHeadroom),nativeLightingClamp.y);
    float alpha=clamp(materialAlpha,0,1);
    return half4(half3(rgb*alpha),half(alpha));
}
half4 nativeGlassSurface(float2 xy,float sdf,float2 normal,float2 worldNormal,float radius,
    float shadowSDF,float ringSDF,half4 original,float coverage,float localScale) {
    // The body replaces the exterior at full coverage. Avoid its unrelated
    // shadow texture reads in the interior while retaining the original mix
    // on antialiased edges and the complete exterior path outside the body.
    half4 field;
    if(coverage>=1)field=nativeGlassBackgroundBody(xy,sdf,normal,radius);
    else {
        float shadowShift=nativeShadowResponse.x*nativeBevel(sdf+nativeShadowResponse.y,nativeShadowShape.z,radius);
        half4 shadow=nativeShadow(xy,xy+normal*shadowShift);
        field=nativeGlassExterior(sdf,shadowSDF,ringSDF,normal,original,shadow,original);
        if(coverage>0)field=mix(field,nativeGlassBackgroundBody(xy,sdf,normal,radius),half(coverage));
    }
    field=nativeGlassRingComposite(sdf,ringSDF,normal,1.0,field);
    field=nativeGlassBackgroundFinalize(sdf,field,original,
        half3(nativeFace(nativeUnpremul(original),sdf)),nativeFaceOpacity,normal,localScale);
    float3 fieldColor=nativeLightingClampColor(nativeUnpremul(field),
        min(nativeLightingClamp.x,nativeDisplayHeadroom),nativeLightingClamp.y);
    field=half4(half3(fieldColor)*field.a,field.a);
    half4 composited=nativeLightingOver(field,original);
    float3 clamped=clamp(nativeUnpremul(composited),float3(0),
        float3(min(nativeLightingClamp.x,nativeDisplayHeadroom)));
    composited=half4(half3(clamped)*composited.a,composited.a);
    return nativeGlassDecorate(xy,sdf,normal,worldNormal,radius,composited,coverage);
}
half4 nativeGlassBody(float2 xy,float sdf,float2 normal,float2 worldNormal,float radius,float ringSDF) {
    half4 original=sharpContent.eval(nativeSampleXY(xy,xy));
    return nativeGlassSurface(xy,sdf,normal,worldNormal,radius,sdf,ringSDF,original,1,max(contentScale,1));
}
half4 referenceGlass(float2 xy,float2 halfDim,float radius,float2 p,float sdf,float2 normal) {
    float ring=boxRoundedSDF(xy-float2(0,nativeRingShadowOffset)-lensCenter,halfDim,radius);
    return nativeGlassBody(xy,sdf,normal,normal,radius,ring);
}
half4 main(float2 filterXY) {
    float2 xy=filterXY/max(filterScale,1.0),halfDim=lensSize*.5;
    float radius=min(cornerRadius,min(halfDim.x,halfDim.y));
    float3 sample=roundedBoxSample(xy-lensCenter,halfDim,radius);
    float sdf=sample.x;float2 normal=sample.yz;
    half4 original=sharpContent.eval(nativeSampleXY(xy,xy));
    if(nativeGlassOutputCoverage(sdf)<=0) return original;
    float shiftedShadow=boxRoundedSDF(xy-nativeShadowOffset-lensCenter,halfDim,radius);
    float shiftedRing=boxRoundedSDF(xy-float2(0,nativeRingShadowOffset)-lensCenter,halfDim,radius);
    float coverage=externalShapeCoverage>.5?1:clamp(.5-sdf*max(contentScale,1),0,1);
    return nativeGlassSurface(xy,sdf,normal,normal,radius,shiftedShadow,shiftedRing,original,coverage,max(contentScale,1));
}
)sksl";
void vec(BackdropEffect& effect,const char* name,float a,float b,float c,float d) {
    effect.setUniform(name,std::array<NXFloat,4>{a,b,c,d});
}
void col(BackdropEffect& effect,const char* name,const NativeGlassColor& c) {vec(effect,name,c.r,c.g,c.b,c.a);}
}
BackdropEffect makeNativeGlassEffect(const NativeGlassProfile& p,UIGlassEffect::Style style,
    UIColor tint,bool accent,NXFloat radius) {
    auto effect=BackdropEffect(source,"content",radius,0);
    const auto mainRadius=(p.blurOpacity0>0 || p.blurOpacity1>0 || p.blurOpacity2>0 || p.blurOpacity3>0)?p.blurRadius:0;
    const auto fillRadius=(p.blurFillNormalOpacity>0 || p.blurFillLightenOpacity>0 || p.blurFillDarkenOpacity>0)?p.blurFillRadius:0;
    const auto bleedRadius=p.bleedOpacity>0?p.bleedBlurRadius*.5f:0;
    const bool shadowSamplesBackdrop=p.shadowOpacity>0 && p.shadowVibrancyContribution>0;
    const auto shadowRadius=shadowSamplesBackdrop?p.shadowBlurRadius*.5f:0;
    // Native Updater aggregates all four main-curve endpoints unconditionally,
    // retaining zero even when auxiliary lanes request positive radii.
    NXFloat minimumRadius=p.blurRadius*std::min({p.blurOpacity0,p.blurOpacity1,p.blurOpacity2,p.blurOpacity3});
    NXFloat maximumRadius=p.blurRadius*std::max({p.blurOpacity0,p.blurOpacity1,p.blurOpacity2,p.blurOpacity3});
    const auto includeRadius=[&](NXFloat value) {
        minimumRadius=std::min(minimumRadius,value);maximumRadius=std::max(maximumRadius,value);
    };
    if(p.blurFillNormalOpacity>0 || p.blurFillLightenOpacity>0 || p.blurFillDarkenOpacity>0)includeRadius(fillRadius);
    if(p.bleedOpacity>0)includeRadius(bleedRadius);
    if(shadowSamplesBackdrop)includeRadius(shadowRadius);
    effect.setBackdropScale(p.context.backdropScale);
    effect.setBackdropPyramid(1.600002f*maximumRadius,nativeGlassPyramidTaps(),
        1.6f*std::max(minimumRadius,0.f),nativeGlassPyramidTaps());
    vec(effect,"nativeBlurRadii",mainRadius,fillRadius,bleedRadius,shadowRadius);
    effect.setUniform("nativeMinimumBlurRadius",std::max(minimumRadius,0.f));
    effect.setUniform("nativePyramidStretch",1.f);
    effect.setOutputOutset(nativeGlassExteriorOutset(p));
    effect.setUniform("bodyMode",style==UIGlassEffect::Style::clear?1.f:0.f);
    effect.setUniform("tintMode",accent?(style==UIGlassEffect::Style::clear?1.f:2.f):0.f);
    effect.setUniform("referenceAppearance",1.f);
    effect.setUniform("backdropLuma",p.context.background=="systemBlack"?.15f:.85f);
    effect.setUniform("tint",tint);
    effect.setUniform("nativeFaceBlack",p.faceBlack);
    effect.setUniform("nativeFaceTransfer",std::array<NXFloat,3>{p.faceWhite,p.faceSaturation,0});
    col(effect,"nativeFaceFill",p.faceColorMatrixFillColor);
    effect.setUniform("nativeFaceLuma",NXPoint(p.faceColorMatrixMaxLuma,p.faceColorMatrixMaxLumaSDR));
    effect.setUniform("nativeFaceOpacity",p.faceOpacity);
    effect.setUniform("nativeFillOpacity",std::array<NXFloat,3>{p.blurFillNormalOpacity,p.blurFillLightenOpacity,p.blurFillDarkenOpacity});
    vec(effect,"nativeBlurDistances",p.blurDistance0,p.blurDistance1,p.blurDistance2,p.blurDistance3);
    vec(effect,"nativeBlurOpacities",p.blurOpacity0,p.blurOpacity1,p.blurOpacity2,p.blurOpacity3);
    vec(effect,"nativeRefraction",p.innerRefractionAmount,p.innerRefractionHeight,p.outerRefractionAmount,p.outerRefractionHeight);
    effect.setUniform("nativeRefractionMask",std::array<NXFloat,3>{p.refractionDistance0,p.refractionDistance1,p.refractionOpacity});
    effect.setUniform("nativeSDRGradient",NXPoint(p.sdrGradientDistance0,p.sdrGradientDistance1));
    effect.setUniform("nativeDisplayHeadroom",1.f);
    effect.setUniform("nativeAccentEnabled",accent?1.f:0.f);
    col(effect,"nativeAccent0",p.accent.colors[0]);col(effect,"nativeAccent1",p.accent.colors[1]);col(effect,"nativeAccent2",p.accent.colors[2]);
    effect.setUniform("nativeAccentDistances",p.accent.distances);
    effect.setUniform("nativeAccentTiming0",p.accent.interpolationControlPoints[0]);
    effect.setUniform("nativeAccentTiming1",p.accent.interpolationControlPoints[1]);
    effect.setUniform("nativeAccentTimingKnown",NXPoint(p.accent.hasInterpolationControlPoints[0]?1.f:0.f,
        p.accent.hasInterpolationControlPoints[1]?1.f:0.f));
    effect.setUniform("nativeAccentMaskOptions",NXPoint(p.accent.premultiplied?1.f:0.f,p.accent.fillCompositingFilter=="destIn"?1.f:0.f));
    col(effect,"nativeAccentFill",p.accent.fillColor);
    const auto accentTransform=nativeGlassAccentMatrix(p,{tint.r()/255.f,tint.g()/255.f,tint.b()/255.f,1});
    const auto& m=accentTransform.matrix.values;
    vec(effect,"nativeAccentMatrix0",m[0],m[1],m[2],m[3]);vec(effect,"nativeAccentMatrix1",m[5],m[6],m[7],m[8]);
    vec(effect,"nativeAccentMatrix2",m[10],m[11],m[12],m[13]);vec(effect,"nativeAccentMatrix3",m[15],m[16],m[17],m[18]);
    vec(effect,"nativeAccentMatrixBias",m[4],m[9],m[14],m[19]);
    effect.setUniform("nativeAccentMatrixOptions",std::array<NXFloat,3>{accentTransform.clamp,
        accentTransform.clampPreserveHue?1.f:0.f,accentTransform.backdropAware?1.f:0.f});
    effect.setUniform("nativeDispersion",0.f);effect.setUniform("nativeEdgeAbsorption",0.f);
    effect.setUniform("nativeTintOpacity",-1.f);effect.setUniform("nativeBrightBackdropShade",0.f);
    effect.setUniform("nativeDarkSpecularGain",1.f);effect.setUniform("nativeBrightSpecularGain",1.f);
    effect.setUniform("nativeReflectionDistance",0.f);effect.setUniform("nativeReflectionSpread",0.f);
    bindNativeGlassLighting(p,effect);
    return effect;
}
}
