#include "ProgressiveBlur.hpp"
#include <private/UIBlurEffect.h>
#include <UIImage.h>
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkShader.h"
#include "include/core/SkM44.h"
#include <cmath>
#include <cstdlib>
#include <stdexcept>
using namespace NXKit;
UIBlurEffect UIBlurEffect::effectWithVariableBlurRadius(NXFloat radius,const std::shared_ptr<UIImage>& mask) {
    if(!std::isfinite(radius)||radius<0||!mask||!mask->cgImage()||!mask->cgImage()->pointee)throw std::invalid_argument("Variable blur requires a finite nonnegative radius and an image mask");
    return UIBlurEffect(radius,mask);
}
namespace {
sk_sp<SkRuntimeEffect> edgeKernelShader() {
    static const auto effect=[] {
        auto result=SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader content;
uniform shader mask;
uniform float2 axis;
uniform float radius;
uniform float subtractVariance;
uniform float maximumSigma;
half4 main(float2 p) {
    float localSigma=radius*clamp(mask.eval(p).a,0.0,1.0);
    if(maximumSigma>0.0 && localSigma>=maximumSigma)return content.eval(p);
    float sigma=sqrt(max(localSigma*localSigma-subtractVariance,0.0));
    if(sigma<0.01)return content.eval(p);
    half4 sum=half4(0);float total=0.0;
    for(int i=-16;i<=16;i++) {
        float d=float(i)*(3.0/16.0);
        float w=exp(-0.5*d*d);
        sum+=content.eval(p+axis*(d*sigma))*half(w);total+=w;
    }
    return sum/half(total);
})"));
        if(!result.effect)throw std::runtime_error(result.errorText.c_str());
        return result.effect;
    }();return effect;
}
sk_sp<SkRuntimeEffect> edgeResolutionShader() {
    static const auto effect=[] {
        auto result=SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader small;
uniform shader large;
uniform shader mask;
uniform float radius;
uniform float splitSigma;
uniform float subtractVariance;
uniform float2 axis;
half4 main(float2 p) {
    float localSigma=radius*clamp(mask.eval(p).a,0.0,1.0);
    if(localSigma<0.01)return small.eval(p);
    half4 sum=half4(0);float total=0.0;
    if(localSigma>=splitSigma) {
        float sigma=sqrt(max(localSigma*localSigma-subtractVariance,0.0));
        for(int i=-8;i<=8;i++) {
            float d=float(i)*(3.0/8.0);float w=exp(-0.5*d*d);
            sum+=large.eval(p+axis*(d*sigma))*half(w);total+=w;
        }
    }else {
        for(int i=-16;i<=16;i++) {
            float d=float(i)*(3.0/16.0);float w=exp(-0.5*d*d);
            sum+=small.eval(p+axis*(d*localSigma))*half(w);total+=w;
        }
    }
    return sum/half(total);
})"));
        if(!result.effect)throw std::runtime_error(result.errorText.c_str());
        return result.effect;
    }();return effect;
}
sk_sp<SkRuntimeEffect> blurShader() {
    static const auto effect=[] {
        auto result=SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader content;
uniform shader mask;
uniform float2 axis;
uniform float radius;
half4 main(float2 p) {
    float sigma=radius*clamp(mask.eval(p).a,0.0,1.0);
    if(sigma<0.01)return content.eval(p);
    half4 sum=half4(0);float total=0.0;
    for(int i=-8;i<=8;i++) {
        float d=float(i)/3.0;float w=exp(-0.5*d*d);
        sum+=content.eval(p+axis*d*sigma)*half(w);total+=w;
    }
    return sum/half(total);
})"));
        if(!result.effect)throw std::runtime_error(result.errorText.c_str());
        return result.effect;
    }();return effect;
}
sk_sp<SkRuntimeEffect> pyramidShader() {
    static const auto effect=[] {
        auto result=SkRuntimeEffect::MakeForShader(SkString(R"(
uniform shader sharp;
uniform shader blur1;
uniform shader blur2;
uniform shader blur3;
uniform shader blur4;
uniform shader blur5;
uniform shader mask;
half4 main(float2 p) {
    float v=pow(clamp(mask.eval(p).a,0.0,1.0),2.0);
    if(v<0.00390625)return mix(sharp.eval(p),blur1.eval(p),half(v/0.00390625));
    if(v<0.015625)return mix(blur1.eval(p),blur2.eval(p),half((v-0.00390625)/0.01171875));
    if(v<0.0625)return mix(blur2.eval(p),blur3.eval(p),half((v-0.015625)/0.046875));
    if(v<0.25)return mix(blur3.eval(p),blur4.eval(p),half((v-0.0625)/0.1875));
    return mix(blur4.eval(p),blur5.eval(p),half((v-0.25)/0.75));
})"));
        if(!result.effect)throw std::runtime_error(result.errorText.c_str());
        return result.effect;
    }();return effect;
}
}
sk_sp<SkImageFilter> Internal::progressiveEdgeBlurFilter(NXFloat radius,sk_sp<SkShader> mask,NXSize viewport,NXFloat scale,bool verticalGradient) {
    if(radius<=0)return nullptr;
    // Sampling bounds are allocation hints, not a change to the convolution. Each pass
    // samples along just one axis; avoid rendering an unused halo on the other axis.
    const bool isotropic = std::getenv("NXKIT_ISOTROPIC_BLUR_BOUNDS") != nullptr;
    const auto support = [&](NXFloat reach, bool vertical) {
        return isotropic ? SkSize::Make(reach, reach)
                         : SkSize::Make(vertical ? 0 : reach, vertical ? reach : 0);
    };
    const auto kernel=[&](sk_sp<SkShader> field,NXFloat density,NXFloat subtractVariance,NXFloat maximumSigma,sk_sp<SkImageFilter> input,bool secondPass=true) {
        SkRuntimeShaderBuilder along(edgeKernelShader()),across(edgeKernelShader());
        for(auto* builder:{&along,&across}){
            builder->uniform("radius")=radius*density;builder->child("mask")=field;
            builder->uniform("subtractVariance")=subtractVariance;builder->uniform("maximumSigma")=maximumSigma;
        }
        along.uniform("axis")=verticalGradient?SkV2{0,1}:SkV2{1,0};
        across.uniform("axis")=verticalGradient?SkV2{1,0}:SkV2{0,1};
        // The field is constant along the second sampling axis, so every
        // input there uses the same local sigma as the output pixel.
        const auto reach=(maximumSigma>0?maximumSigma:radius*density)*3;
        auto first=SkImageFilters::RuntimeShader(along,support(reach,verticalGradient),"content",std::move(input));
        if(!secondPass)return first;
        return SkImageFilters::RuntimeShader(across,support(reach,!verticalGradient),"content",std::move(first));
    };
    // Edge sampling extends the scroll viewport's actual boundary pixels,
    // rather than treating the area outside it as transparent black.
    auto source=SkImageFilters::Crop(SkRect::MakeWH(viewport.width*scale,viewport.height*scale),SkTileMode::kClamp,nullptr);
    if(scale<=2)return kernel(mask,scale,0,0,std::move(source));
    // Large kernels can run at one pixel per point. A uniform one-point
    // prefilter removes downsampling aliasing; subtracting its variance keeps
    // the combined Gaussian at the requested local width. Small kernels stay
    // at display resolution, including exact identity at zero radius.
    constexpr NXFloat splitSigma=2,prefilterSigma=1;
    const SkSamplingOptions linear(SkFilterMode::kLinear);
    auto reduced=SkImageFilters::MatrixTransform(SkMatrix::Scale(1/scale,1/scale),linear,
        SkImageFilters::Blur(prefilterSigma*scale,prefilterSigma*scale,SkTileMode::kClamp,source));
    auto field=mask->makeWithLocalMatrix(SkMatrix::Scale(1/scale,1/scale));
    const auto resamplingVariance=(1-1/(scale*scale))/3;
    const auto removedVariance=prefilterSigma*prefilterSigma+resamplingVariance;
    auto large=kernel(std::move(field),1,removedVariance,0,std::move(reduced),false);
    large=SkImageFilters::Crop(SkRect::MakeWH(viewport.width,viewport.height),SkTileMode::kClamp,std::move(large));
    large=SkImageFilters::MatrixTransform(SkMatrix::Scale(scale,scale),linear,std::move(large));
    auto small=kernel(mask,scale,0,splitSigma*scale,std::move(source),false);
    SkRuntimeShaderBuilder resolve(edgeResolutionShader());resolve.child("mask")=std::move(mask);
    resolve.uniform("radius")=radius*scale;resolve.uniform("splitSigma")=splitSigma*scale;
    resolve.uniform("subtractVariance")=removedVariance*scale*scale;
    resolve.uniform("axis")=verticalGradient?SkV2{1,0}:SkV2{0,1};
    std::string_view names[]={"small","large"};sk_sp<SkImageFilter> inputs[]={small,large};
    return SkImageFilters::RuntimeShader(resolve,support(radius*scale*3,!verticalGradient),names,inputs,2);
}
sk_sp<SkImageFilter> Internal::progressiveBlurFilter(NXFloat radius,sk_sp<SkShader> mask,NXFloat scale) {
    if(radius<=0)return nullptr;
    if(radius*scale>4) {
        // A fixed number of widely spaced taps aliases fine detail at large
        // radii. Gaussian levels keep those frequencies filtered; interpolate
        // local variance, rather than fading one uniformly blurred image.
        SkRuntimeShaderBuilder builder(pyramidShader());builder.child("mask")=std::move(mask);
        std::string_view names[]={"sharp","blur1","blur2","blur3","blur4","blur5"};
        sk_sp<SkImageFilter> inputs[6];
        for(int i=1;i<6;++i) {
            const auto sigma=radius*scale/std::pow(2.f,5-i);
            inputs[i]=SkImageFilters::Blur(sigma,sigma,SkTileMode::kClamp,nullptr);
        }
        return SkImageFilters::RuntimeShader(builder,0.f,names,inputs,6,true);
    }
    const auto runtime=blurShader();
    SkRuntimeShaderBuilder horizontal(runtime),vertical(runtime);
    for(auto* builder:{&horizontal,&vertical}) {builder->uniform("radius")=radius*scale;builder->child("mask")=mask;}
    horizontal.uniform("axis")=SkV2{1,0};vertical.uniform("axis")=SkV2{0,1};
    auto first=SkImageFilters::RuntimeShader(horizontal,radius*scale*3,"content",nullptr);
    return SkImageFilters::RuntimeShader(vertical,radius*scale*3,"content",std::move(first));
}
sk_sp<SkImageFilter> Internal::progressiveBlurFilter(const UIBlurEffect& effect,NXSize size,NXFloat scale) {
    sk_sp<SkShader> mask=SkShaders::Color(SK_ColorWHITE);
    if(const auto image=effect.imageMask();image&&image->cgImage()&&image->cgImage()->pointee) {
        const auto pixels=image->cgImage()->pointee;
        const auto matrix=SkMatrix::Scale(size.width*scale/pixels->width(),size.height*scale/pixels->height());
        mask=pixels->makeShader(SkTileMode::kClamp,SkTileMode::kClamp,SkSamplingOptions(SkFilterMode::kLinear),matrix);
    }
    // iOS 26's private radius is not a Gaussian sigma. The retained radius-24
    // probe fits approximately 44 pt sigma; this portable mapping is empirical.
    return progressiveBlurFilter(effect.blurRadius()*1.8f,std::move(mask),scale);
}
