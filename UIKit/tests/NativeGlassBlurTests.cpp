#include "NativeGlassBlur.hpp"
#include "BackdropShader.hpp"
#include "NativeGlassBlurReference.hpp"
#include "NativeGlassBlurStripeReference.hpp"
#include "include/core/SkBitmap.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <string_view>

using namespace NXKit;
using namespace NXKit::Internal;
namespace {
int failures=0;
void expect(bool v,const char* message) { if(!v) { ++failures;std::cerr<<message<<std::endl; } }
std::array<unsigned char,384> render(float radius,float captureScale,int period=0) {
    constexpr int width=1206,height=1206;
    auto input=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,height));
    auto canvas=input->getCanvas();canvas->clear(SK_ColorBLACK);
    SkPaint white;white.setColor(SK_ColorWHITE);
    if(period>0)for(int x=period*3/2;x<width;x+=period*3)canvas->drawRect(SkRect::MakeLTRB(x,0,x+period*3/2,height),white);
    else canvas->drawRect(SkRect::MakeLTRB(600,0,width,height),white);
    BackdropEffect effect(std::string("uniform shader content; uniform shader sharpContent; uniform float4 backdropPyramidRect; uniform float4 backdropPyramidInfo; uniform float maximumSampleRadius; uniform float filterScale; uniform float nativePyramidStretch; uniform float nativeMinimumBlurRadius; uniform float4 nativeBlurRadii;")+std::string(nativeGlassBlurShaderSource())+"half4 main(float2 p){return nativeAtRadius(p/3,p/3,nativeBlurRadii.x);}");
    effect.setBackdropScale(captureScale);
    effect.setBackdropPyramid(radius*1.600002f,nativeGlassPyramidTaps(),radius*1.6f,nativeGlassPyramidTaps());
    effect.setUniform("nativePyramidStretch",1);
    effect.setUniform("nativeMinimumBlurRadius",radius);
    effect.setUniform("nativeBlurRadii",std::array<NXFloat,4>{radius,0,0,0});
    auto runtime=BackdropShader::compile(effect);
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,8));
    surface->getCanvas()->translate(0,-height/2+4);
    SkPaint filtered;filtered.setImageFilter(BackdropShader::makeFilter(effect,runtime,{402,402},0,CALayerCornerCurve::circular,3));
    surface->getCanvas()->drawImage(input->makeImageSnapshot(),0,0,SkSamplingOptions(),&filtered);
    SkBitmap bitmap;bitmap.allocN32Pixels(width,8);surface->readPixels(bitmap,0,0);
    std::array<unsigned char,384> result;
    for(int i=0;i<384;++i)result[i]=SkColorGetR(bitmap.getColor((period>0?300:408)+i,4));
    return result;
}
}
int main(int argc,char**argv) {
    double current=0;size_t count=0;
    for(const auto& ref:nativeBlurReferences) {
        const auto a=render(ref.radius,ref.scale);
        double sum=0;
        for(size_t i=0;i<a.size();++i) {sum+=std::abs(int(a[i])-int(ref.samples[i]));}
        std::cout<<"radius="<<ref.radius<<" scale="<<ref.scale<<" nativeMAE="<<sum/a.size()<<std::endl;
        current+=sum;count+=a.size();
    }
    std::cout<<"total MAE "<<current/count<<std::endl;
    expect(current/count<.5,"measured native kernel reconstruction stays within half a byte mean error");

    double stripeError=0;size_t stripeCount=0;
    for(const auto& ref:nativeBlurStripes) {
        auto a=render(ref.radius,ref.scale,ref.period);
        double sum=0;
        for(size_t i=0;i<a.size();++i) {sum+=std::abs(int(a[i])-int(ref.samples[i]));}
        std::cout<<"STRIPE radius="<<ref.radius<<" scale="<<ref.scale<<" period="<<ref.period<<" new="<<sum/a.size()<<std::endl;
        stripeError+=sum;stripeCount+=a.size();
    }
    std::cout<<"holdout stripe MAE "<<stripeError/stripeCount<<std::endl;
    expect(stripeError/stripeCount<1.8,"independent native stripe comparison stays within1.8bytes mean");

    expect(std::abs(nativeGlassBlurLOD(5,3,.5f)-std::log2(12.f))<1e-6,"native radius includes recovered 1.6 host factor");
    expect(std::abs(nativeGlassBlurLOD(1,3,.5f,1)-(1+std::log2(1.6f)))<1e-6,
        "native skip-base halves radius before nonlinear LOD conversion");
    BackdropEffect description("uniform shader content;half4 main(float2 p){return content.eval(p);}");
    description.setBackdropBlurKernel({2,5,.4f});
    expect(description.backdropBlurRadius()==5,"legacy radius exposes full sampling support");
    description.setBackdropBlurRadius(3);
    expect(description.backdropBlurKernel()==BackdropBlurKernel{3,3,0},"single radius clears earlier mixture");
    bool rejected=false;try{description.setBackdropInput("aux",BackdropBlurKernel{1,2,1.1f});}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"kernel rejects invalid weights");
    return failures?1:0;
}
