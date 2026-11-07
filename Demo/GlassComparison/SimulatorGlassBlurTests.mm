// Run with simctl spawn. Actual iOS Simulator Graphite/Metal execution, including
// all64 independent native blur controls and fractional capture reconstruction.
#include <UIKit.h>
#include "include/core/SkRRect.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include <fstream>
#include <limits>
#include "include/core/SkData.h"
#include <SkiaCtx.h>
#include "BackdropShader.hpp"
#include "LensMaterial.hpp"
#include "LayerUniformPrefix.hpp"
#include "ProgressiveBlur.hpp"
#include "LayerContentMask.hpp"
#include "LensForegroundComposition.hpp"
#include "RenderPerformance.hpp"
#include "NativeGlassBlur.hpp"
#include "LayerCornerGeometry.hpp"
#include "platforms/apple/MetalPipelineCache.hpp"
#include "../../UIKit/tests/NativeGlassBlurReference.hpp"
#include "../../UIKit/tests/GlassPressCoverageFixture.hpp"
#include "../../UIKit/tests/NativeGlassBlurStripeReference.hpp"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#import <Metal/Metal.h>
#include <UISwitch.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <vector>

using namespace NXKit;
using namespace NXKit::Internal;
namespace NXKit {
class GlassMaterialTestHarness {
public:
    static std::shared_ptr<UITraitCollection> traits(int scale,bool dark,float amount) {
        auto result=UITraitCollection::traitCollectionWithGlassTintAmount(amount);
        result->_displayScale=scale;
        result->_userInterfaceStyle=dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;
        return result;
    }
};
class LensViewTestHarness {
public:
    static void progress(const std::shared_ptr<LensView>& lens,NXFloat value) {
        lens->setControlLift(value>0,value,1-value,value,lens->bounds().size);
    }
};
class UISwitchTestHarness {
public:
    static void pose(const std::shared_ptr<UISwitch>& control,float progress) {
        control->setPressed(progress>0,false);control->layoutSubviews();
        LensViewTestHarness::progress(control->_lens,progress);
    }
};
class UISliderTestHarness {
public:
    static void pressed(const std::shared_ptr<UISlider>& slider,bool value) {slider->setPressed(value,true);}
    static void advance(const std::shared_ptr<UISlider>& slider,double dt) {slider->advanceThumb(dt);}
    static void pose(const std::shared_ptr<UISlider>& slider,float progress) {
        slider->setPressed(progress>0,false);slider->layoutSubviews();
        slider->_lens->setLifted(progress>0);
        LensViewTestHarness::progress(slider->_lens,progress);
    }
};
class LensMetalRoot final:public UIView {
public:
    LensMetalRoot(int scale,bool dark) {
        _traitCollection=GlassMaterialTestHarness::traits(scale,dark,.5);
        setFrame({0,0,180,160});setBackgroundColor(UIColor(32,76,128));
    }
};
}
namespace {
struct GPU {
    std::shared_ptr<MetalPipelineCache> cache;
    std::unique_ptr<skgpu::graphite::Context> context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    GPU() {
        auto device=MTLCreateSystemDefaultDevice();
        if(!device)throw std::runtime_error("Simulator Metal unavailable");
        skgpu::graphite::MtlBackendContext backend;
        backend.fDevice=sk_ret_cfp((CFTypeRef)device);
        backend.fQueue=sk_ret_cfp((CFTypeRef)[device newCommandQueue]);
        cache=std::make_shared<MetalPipelineCache>(backend.fDevice.get());
        backend.fPipelineCache=cache;
        skgpu::graphite::ContextOptions options;options.fRequireOrderedRecordings=true;
        cache->configure(backend,options);
        context=skgpu::graphite::ContextFactory::MakeMetal(backend,options);
        if(!context)throw std::runtime_error("Graphite initialization failed");
        recorder=context->makeRecorder();
    }
    ~GPU(){context->submit(skgpu::graphite::SyncToCpu::kYes);recorder.reset();context.reset();cache->save();}
    sk_sp<SkSurface> surface(int width,int height) {
        auto result=SkSurfaces::RenderTarget(recorder.get(),SkImageInfo::MakeN32Premul(width,height));
        if(!result)throw std::runtime_error("GPU surface allocation failed");return result;
    }
    std::vector<uint8_t> read(const sk_sp<SkSurface>& surface) {
        auto recording=recorder->snap();
        if(!recording)throw std::runtime_error("Recording failed");
        skgpu::graphite::InsertRecordingInfo insert;insert.fRecording=recording.get();
        if(context->insertRecording(insert)!=skgpu::graphite::InsertStatus::kSuccess)
            throw std::runtime_error("Insert failed");
        if(!context->submit(skgpu::graphite::SyncToCpu::kYes))throw std::runtime_error("GPU submission failed");
        const auto info=SkImageInfo::Make(surface->width(),surface->height(),kRGBA_8888_SkColorType,kPremul_SkAlphaType);
        std::unique_ptr<const SkImage::AsyncReadResult> pixels;
        context->asyncRescaleAndReadPixels(surface.get(),info,SkIRect::MakeWH(info.width(),info.height()),
            SkImage::RescaleGamma::kSrc,SkImage::RescaleMode::kNearest,
            [](void* out,std::unique_ptr<const SkImage::AsyncReadResult> value){
                *static_cast<std::unique_ptr<const SkImage::AsyncReadResult>*>(out)=std::move(value);
            },&pixels);
        context->submit(skgpu::graphite::SyncToCpu::kYes);context->checkAsyncWorkCompletion();
        if(!pixels)throw std::runtime_error("GPU readback failed");
        std::vector<uint8_t> result(info.width()*info.height()*4);
        for(int y=0;y<info.height();++y)std::copy_n(
            static_cast<const uint8_t*>(pixels->data(0))+y*pixels->rowBytes(0),info.width()*4,result.data()+y*info.width()*4);
        return result;
    }
};
sk_sp<SkRuntimeEffect> compile(const std::string& source) {
    auto result=SkRuntimeEffect::MakeForShader(SkString(source));
    if(!result.effect)throw std::runtime_error(result.errorText.c_str());return result.effect;
}
// Frozen pre-optimization capture equation. Its zero-weight neighbours and
// fractional interpolation are kept literally for the equivalence control.
sk_sp<SkImageFilter> referenceCapture(float scale) {
    static const auto runtime=compile(R"sksl(
        uniform shader captureSource; uniform float captureTaps;
        half4 cell(float2 p){half4 sum=half4(0);
            for(int y=0;y<4;++y)for(int x=0;x<4;++x)
                if(float(x)<captureTaps&&float(y)<captureTaps)
                    sum+=captureSource.eval(p+(float2(x,y)+.5)/captureTaps-.5);
            return sum/half(captureTaps*captureTaps);}
        half4 main(float2 xy){float2 grid=floor(xy-.5)+.5,f=fract(xy-.5);
            return mix(mix(cell(grid),cell(grid+float2(1,0)),f.x),
                mix(cell(grid+float2(0,1)),cell(grid+float2(1,1)),f.x),f.y);}
    )sksl");
    SkRuntimeShaderBuilder builder(runtime);
    builder.uniform("captureTaps")=std::clamp(std::round(.5f/scale),1.f,4.f);
    return SkImageFilters::RuntimeShader(builder,2,"captureSource",
        SkImageFilters::MatrixTransform(SkMatrix::Scale(scale,scale),SkSamplingOptions(SkFilterMode::kLinear),nullptr));
}
struct Raster {
    sk_sp<SkSurface> surface(int width,int height) {
        return SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,height));
    }
    std::vector<uint8_t> read(const sk_sp<SkSurface>& surface) {
        const auto info=SkImageInfo::Make(surface->width(),surface->height(),kRGBA_8888_SkColorType,kPremul_SkAlphaType);
        std::vector<uint8_t> pixels(info.computeMinByteSize());
        if(!surface->readPixels(info,pixels.data(),info.minRowBytes(),0,0))
            throw std::runtime_error("Raster readback failed");
        return pixels;
    }
};
void directionalFilterContracts() {
    auto effect=compile("uniform shader content; half4 main(float2 p){return content.eval(p+float2(3,0));}");
    SkRuntimeShaderBuilder builder(effect);
    for(auto invalid:{SkSize::Make(-1,0),SkSize::Make(0,-1),
                      SkSize::Make(std::numeric_limits<float>::infinity(),0),
                      SkSize::Make(0,std::numeric_limits<float>::quiet_NaN())})
        if(SkImageFilters::RuntimeShader(builder,invalid,"content",nullptr))
            throw std::runtime_error("Invalid directional sampling extent accepted");
    if(SkImageFilters::RuntimeShader(builder,SkSize::Make(3,0),"missing",nullptr))
        throw std::runtime_error("Unknown directional child accepted");
    std::string_view names[]={"content","content"};sk_sp<SkImageFilter> inputs[2];
    if(SkImageFilters::RuntimeShader(builder,SkSize::Make(3,0),names,inputs,2) ||
       SkImageFilters::RuntimeShader(builder,SkSize::Make(3,0),names,inputs,-1))
        throw std::runtime_error("Invalid directional input list accepted");
    auto filter=SkImageFilters::RuntimeShader(builder,SkSize::Make(3,0),"",nullptr);
    if(!filter)throw std::runtime_error("Implicit directional child rejected");
    auto data=filter->serialize();
    if(!data || !SkImageFilter::Deserialize(data->data(),data->size()))
        throw std::runtime_error("Directional filter serialization failed");
}
// Verify allocation hints against the original isotropic request with identical kernels.
// Fractional placement, transparent sources and narrow clips exercise filter boundary pixels.
template<class Backend> void directionalBlurEquivalence(Backend& backend,const char* name) {
    int maximum=0,cases=0;double error=0;size_t values=0;
    for(int scale:{1,2,3})for(bool vertical:{false,true})for(bool reverse:{false,true})
    for(float offset:{0.f,.375f})for(float alpha:{1.f,.47f})for(bool narrow:{false,true}) {
        const int width=72*scale,height=96*scale;
        std::array<std::vector<uint8_t>,3> pixels;
        for(int mode=0;mode<3;++mode) {
            if(mode==0)setenv("NXKIT_ISOTROPIC_BLUR_BOUNDS","1",1);
            else unsetenv("NXKIT_ISOTROPIC_BLUR_BOUNDS");
            auto surface=backend.surface(width,height);auto* canvas=surface->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);canvas->translate(offset*scale,-offset*scale);
            SkPaint paint;
            for(int y=0;y<height;y+=3*scale)for(int x=0;x<width;x+=3*scale) {
                paint.setColor4f({float((x/scale)%17)/17,float((y/scale)%19)/19,
                                  float((x+y)/scale%23)/23,alpha});
                canvas->drawRect(SkRect::MakeXYWH(x,y,2*scale,2*scale),paint);
            }
            canvas->save();
            const float extent=(vertical?height:width)*.6f;
            auto clip=vertical?SkRect::MakeWH(width,extent):SkRect::MakeWH(extent,height);
            if(narrow)clip.inset(7.25f*scale,8.125f*scale);
            canvas->clipRect(clip);
            static const auto effect=compile("uniform float2 axis; uniform float extent; uniform float reverse; half4 main(float2 p){float t=clamp(dot(p,axis)/extent,0.0,1.0); return half4(1,1,1,half(reverse>0.0?t:1.0-t));}");
            SkRuntimeShaderBuilder builder(effect);builder.uniform("axis")=vertical?SkV2{0,1}:SkV2{1,0};
            builder.uniform("extent")=extent;builder.uniform("reverse")=reverse?1.f:0.f;
            auto mask=builder.makeShader();
            auto filter=progressiveEdgeBlurFilter(8,std::move(mask),{72,96},scale,vertical);
            if(mode==2) {
                const auto data=filter->serialize();filter=SkImageFilter::Deserialize(data->data(),data->size());
                if(!filter)throw std::runtime_error("Serialized edge filter rejected");
            }
            paint.setImageFilter(std::move(filter));
            canvas->saveLayer(SkCanvas::SaveLayerRec(&clip,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));
            canvas->restore();canvas->restore();pixels[mode]=backend.read(surface);
        }
        int caseMaximum=0;
        for(int mode=1;mode<3;++mode)for(size_t i=0;i<pixels[0].size();++i) {
            const int delta=std::abs(int(pixels[0][i])-int(pixels[mode][i]));
            caseMaximum=std::max(caseMaximum,delta);error+=delta;
        }
        maximum=std::max(maximum,caseMaximum);values+=pixels[0].size()*2;++cases;
        if(caseMaximum>2)std::cout<<name<<" directional case scale="<<scale<<" vertical="<<vertical<<" reverse="<<reverse<<" offset="<<offset<<" alpha="<<alpha<<" narrow="<<narrow<<" max="<<caseMaximum<<std::endl;
    }
    unsetenv("NXKIT_ISOTROPIC_BLUR_BOUNDS");
    if(maximum>2 || error/values>.001)throw std::runtime_error(std::string(name)+" directional blur bounds changed pixels: max="+std::to_string(maximum));
    std::cout<<name<<" directional blur bounds: cases="<<cases<<" max="<<maximum<<" MAE="<<error/values<<std::endl;
}
// Clear-only resolves may be reloaded into MSAA after a read-only snapshot.
// Check the stored color, retained snapshots and intervening writes separately.
std::vector<uint8_t> metalClearSemantics(GPU& gpu) {
    std::vector<uint8_t> all;
    int cases=0;
    for(bool srgb:{false,true})for(auto type:{kN32_SkColorType,kRGBA_F16_SkColorType})
    for(const SkColor4f color:{SkColor4f{0,0,0,0},SkColor4f{.123f,.451f,.789f,1},SkColor4f{.17f,.67f,.31f,.43f}})
    for(int state=0;state<4;++state) {
        const auto info=SkImageInfo::Make(128,96,type,kPremul_SkAlphaType,srgb?SkColorSpace::MakeSRGB():nullptr);
        auto surface=SkSurfaces::RenderTarget(gpu.recorder.get(),info);
        auto* canvas=surface->getCanvas();canvas->clear(color);
        const auto uniform=uniformLayerPrefix(SkCanvasPriv::TopDevice(canvas));
        if(!uniform)throw std::runtime_error("Uniform clear metadata was not available");
        auto prefix=SkCanvasPriv::TopDevice(canvas)->snapSpecial(SkIRect::MakeWH(128,96),true);
        if(!prefix)throw std::runtime_error("Clear snapshot failed");
        auto expected=color;
        if(state==1) {
            SkPaint red;red.setColor(SK_ColorRED);canvas->drawRect(SkRect::MakeXYWH(2,72,20,20),red);
            if(!SkCanvasPriv::TopDevice(canvas)->snapSpecial(SkIRect::MakeWH(128,96),true))throw std::runtime_error("Write snapshot failed");
        }else if(state==2) {
            auto other=gpu.surface(128,96);other->getCanvas()->clear(SK_ColorGREEN);
            if(!SkCanvasPriv::TopDevice(other->getCanvas())->snapSpecial(SkIRect::MakeWH(128,96),true))throw std::runtime_error("Other clear snapshot failed");
        }else if(state==3) {
            expected={.73f,.19f,.47f,.61f};canvas->clear(expected);
            if(!SkCanvasPriv::TopDevice(canvas)->snapSpecial(SkIRect::MakeWH(128,96),true))throw std::runtime_error("Replacement clear snapshot failed");
        }
        SkPathBuilder path;path.moveTo(32,24).cubicTo(60,12,92,24,96,48).lineTo(80,76).cubicTo(48,88,24,60,32,24).close();
        SkPaint blue;blue.setColor(SK_ColorBLUE);blue.setAntiAlias(true);canvas->drawPath(path.detach(),blue);
        const auto pixels=gpu.read(surface);
        auto reference=SkSurfaces::Raster(info);reference->getCanvas()->clear(expected);Raster raster;
        const auto background=raster.read(reference);
        const auto check=[&](const std::vector<uint8_t>& actual,int x,int y,const std::vector<uint8_t>& wanted) {
            const auto at=(y*128+x)*4;
            for(int channel=0;channel<4;++channel)if(std::abs(int(actual[at+channel])-int(wanted[at+channel]))>1)
                throw std::runtime_error("Metal clear/write state lost stored background");
        };
        check(pixels,4,4,background);
        if(state==1) {
            const auto at=(80*128+8)*4;
            if(pixels[at]!=255||pixels[at+1]!=0||pixels[at+2]!=0||pixels[at+3]!=255)throw std::runtime_error("MSAA reload discarded intervening write");
        }
        const auto at=(48*128+60)*4;
        if(pixels[at]!=0||pixels[at+1]!=0||pixels[at+2]!=255||pixels[at+3]!=255)throw std::runtime_error("MSAA draw missing");
        auto copy=gpu.surface(128,96);SkPaint paint;paint.setShader(prefix->asShader(SkTileMode::kDecal,SkSamplingOptions(SkFilterMode::kNearest),SkMatrix::I(),true));
        paint.setBlendMode(SkBlendMode::kSrc);copy->getCanvas()->drawPaint(paint);
        const auto frozen=gpu.read(copy);reference->getCanvas()->clear(color);const auto original=raster.read(reference);
        auto constant=gpu.surface(128,96);paint.setShader(uniform);constant->getCanvas()->drawPaint(paint);
        const auto constantPixels=gpu.read(constant);
        for(size_t i=0;i<frozen.size();++i)if(std::abs(int(frozen[i])-int(constantPixels[i]))>1)throw std::runtime_error("Uniform prefix changed stored clear values");
        for(size_t i=0;i<frozen.size();++i)if(std::abs(int(frozen[i])-int(original[i]))>1)throw std::runtime_error("Clear snapshot changed after later rendering");
        all.insert(all.end(),pixels.begin(),pixels.end());all.insert(all.end(),frozen.begin(),frozen.end());++cases;
    }
    std::cout<<"Metal clear/snapshot/write semantics: "<<cases<<" cases passed"<<std::endl;
    return all;
}
template<class Renderer>
void captureEquivalence(Renderer& gpu,const char* backend) {
    auto input=gpu.surface(256,192);auto canvas=input->getCanvas();canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    for(int y=0;y<192;y+=3)for(int x=0;x<256;x+=3) {
        paint.setColor(SkColorSetARGB((x*13+y*23)%256,(x*29+y*11)%256,(x*17+y*3)%256,(x*5+y*31)%256));
        canvas->drawRect(SkRect::MakeXYWH(x,y,3,3),paint);
    }
    auto image=input->makeImageSnapshot();
    static const auto sample=compile("uniform shader scene;uniform float2 offset;half4 main(float2 p){return scene.eval(p+offset);}");
    int cases=0,maximum=0;double error=0;size_t values=0;
    for(float scale:{.125f,1.f/6,.2f,.25f,.5f})for(const SkV2 offset:{SkV2{0,0},SkV2{.25f,.5f},SkV2{-.3f,1.75f},SkV2{3.125f,-.625f}}) {
        std::vector<uint8_t> a,b;
        for(bool reference:{true,false}) {
            auto output=gpu.surface(24,20);output->getCanvas()->clear(SK_ColorTRANSPARENT);
            SkRuntimeShaderBuilder builder(sample);builder.uniform("offset")=offset;
            auto filter=SkImageFilters::RuntimeShader(builder,6,"scene",reference?referenceCapture(scale):BackdropShader::capture(scale));
            SkPaint filtered;filtered.setImageFilter(filter);
            output->getCanvas()->drawImage(image,0,0,SkSamplingOptions(),&filtered);
            (reference?a:b)=gpu.read(output);
        }
        if(std::none_of(a.begin(),a.end(),[](auto value){return value!=0;}))
            throw std::runtime_error("Reference capture unexpectedly empty");
        for(size_t i=0;i<a.size();++i){const auto difference=std::abs(int(a[i])-int(b[i]));maximum=std::max(maximum,difference);error+=difference;}
        values+=a.size();++cases;
    }
    std::cout<<backend<<" capture equivalence "<<cases<<" cases: max="<<maximum<<" MAE="<<error/values<<std::endl;
    if(maximum!=0)throw std::runtime_error("Capture optimization changed RGBA reconstruction");
}
class LensSourceProbe final:public CALayer {
public:
    int draws=0;
    void draw(SkCanvas* canvas) override {
        ++draws;SkPaint paint;paint.setColor(0xff008aff);
        canvas->drawRect(SkRect::MakeXYWH(35,35,25,65),paint);
        paint.setColor(0x90ff0000);canvas->drawRect(SkRect::MakeXYWH(58,42,42,35),paint);
        constexpr SkColor colors[]={0xff008aff,0xfffb3449,0xff34c96a,0xffffa233,0xffff389d,
            0xff6760d3,0xff00c3aa,0xffffd400,0xffffffff,0xff8f8f93};
        for(size_t i=0;i<std::size(colors);++i){paint.setColor(colors[i]);canvas->drawRect(SkRect::MakeXYWH(31+i*9.3f,48,9.3f,22),paint);}
    }
};
class PrefixMaskFixture final:public LayerContentMask {
public:
    bool active(const CALayer&) const override{return true;}
    sk_sp<SkShader> contentShader(const CALayer&) const override {
        static const auto effect=compile("half4 main(float2 p){half a=half(clamp(p.y/64,0,1));return half4(a);}");
        SkRuntimeShaderBuilder builder(effect);return builder.makeShader();
    }
    std::optional<std::vector<SkRect>> affectedRegions(const CALayer& layer)const override {
        return std::vector<SkRect>{SkRect::MakeWH(layer.bounds().width(),64)};
    }
    void draw(SkCanvas* canvas,const CALayer& layer)const override {
        SkPaint paint;paint.setShader(contentShader(layer));canvas->drawPaint(paint);
    }
};
std::vector<uint8_t> maskPrefixSemantics(GPU& gpu) {
    std::vector<uint8_t> all;int cases=0,uniformCases=0;
    const auto snapshot=std::getenv("NXKIT_SNAPSHOT_MASK_PREFIX");const bool reference=snapshot&&snapshot[0]=='1';
    for(int scale:{1,2,3})for(bool srgb:{false,true})for(auto type:{kN32_SkColorType,kRGBA_F16_SkColorType})
    for(const SkColor4f color:{SkColor4f{0,0,0,0},SkColor4f{.123f,.451f,.789f,1},SkColor4f{.17f,.67f,.31f,.43f}})
    for(float alpha:{1.f,.61f})for(float offset:{0.f,12.25f})for(bool nested:{false,true}) {
        auto root=new_shared<UIView>(NXRect{0,0,128,96});root->setClipsToBounds(true);root->setAlpha(alpha);
        root->setBounds({0,offset,128,96});root->layer()->setContentsScale(scale);
        auto probe=new_shared<LensSourceProbe>();probe->setOpacity(.65);
        root->addSubview(new_shared<UIView>(NXRect{0,0,128,96},probe));
        if(nested) {
            BackdropEffect effect("uniform shader content;half4 main(float2 p){return content.eval(p);}");effect.setBackdropBlurRadius(3);
            auto blur=new_shared<BackdropEffectView>(effect);blur->setFrame({20,24,80,56});root->addSubview(blur);
        }
        auto mask=std::make_shared<PrefixMaskFixture>();LayerContentMaskAccess::add(root->layer(),mask);
        const auto info=SkImageInfo::Make(128*scale,96*scale,type,kPremul_SkAlphaType,srgb?SkColorSpace::MakeSRGB():nullptr);
        auto surface=SkSurfaces::RenderTarget(gpu.recorder.get(),info);auto* canvas=surface->getCanvas();canvas->clear(color);canvas->scale(scale,scale);
        RenderPerformance metrics;
        {struct Restore{RenderPerformance* old;~Restore(){RenderPerformance::current=old;}} restore{RenderPerformance::current};
         RenderPerformance::current=&metrics;root->layer()->skiaRender(canvas);}
        if(probe->draws!=1)throw std::runtime_error("Uniform mask prefix changed callback count");
        if(reference&&metrics.uniformMaskPrefixes)throw std::runtime_error("Snapshot control used uniform prefix");
        if(!reference&&alpha==1&&metrics.uniformMaskPrefixes!=1)throw std::runtime_error("Eligible full viewport did not use uniform prefix");
        uniformCases+=metrics.uniformMaskPrefixes>0;const auto pixels=gpu.read(surface);
        all.insert(all.end(),pixels.begin(),pixels.end());++cases;
    }
    std::cout<<"Mask prefix fixture "<<cases<<" cases; uniform cases="<<uniformCases<<std::endl;
    return all;
}
class ComposedLensProbe final:public CALayer {
public:
    std::shared_ptr<LensLayer> foreground;
    bool bounded=false;
    bool cropSource=false;
    void renderSublayers(SkCanvas* canvas) override {
        LensForegroundComposition composition(canvas,bounds().size,bounded);
        CALayer::renderSublayers(canvas);foreground->drawSource(canvas,false,cropSource);
    }
};
void liftedLensMetalEquivalence(GPU& gpu) {
    Raster raster;double total=0;size_t count=0;int cases=0,maximum=0;
    for(int scale:{1,2,3})for(bool dark:{false,true})for(bool selection:{false,true})
      for(bool vibrant:{false,true})for(float progress:{.5f,1.f}) {
        const auto draw=[&](auto& renderer) {
            auto root=new_shared<LensMetalRoot>(scale,dark);
            auto source=new_shared<UIView>(root->bounds());root->addSubview(source);
            const std::array<NXRect,5> frames={NXRect{42,49,2,18},NXRect{118,49,2,18},NXRect{70,42,24,1},NXRect{70,71,24,1},NXRect{76,54,8,8}};
            for(size_t i=0;i<frames.size();++i) {
                auto mark=new_shared<UIView>(frames[i]);
                mark->setBackgroundColor(i%2?UIColor(0,210,255,160):UIColor(17,94,231));source->addSubview(mark);
            }
            auto lens=new_shared<LensView>(selection?LensView::Style::selection:LensView::Style::regular);
            lens->setFrame({30.25f,35.5f,100,44});root->addSubview(lens);lens->setWarpsContentBelow(true);lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);lens->setLiftedContentView(source);
            lens->setContentVibrancyEnabled(vibrant);lens->setLifted(true);LensViewTestHarness::progress(lens,progress);
            auto surface=renderer.surface(180*scale,160*scale);auto canvas=surface->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);root->layer()->skiaRender(canvas);
            return renderer.read(surface);
        };
        const auto a=draw(raster),b=draw(gpu);double error=0;size_t values=0;int caseMax=0;
        for(int y=23*scale;y<92*scale;++y)for(int x=18*scale;x<143*scale;++x)for(int c=0;c<4;++c) {
            const auto i=(y*180*scale+x)*4+c;const auto delta=std::abs(int(a[i])-int(b[i]));
            error+=delta;++values;caseMax=std::max(caseMax,delta);
        }
        if(error/values>1 || caseMax>80) {
            std::cerr<<"Lifted lens mismatch scale="<<scale<<" selection="<<selection<<" dark="<<dark<<" vibrant="<<vibrant<<" progress="<<progress<<" max="<<caseMax<<" MAE="<<error/values<<'\n';
            throw std::runtime_error("Lifted native lens Metal/raster disagreement");
        }
        total+=error;count+=values;maximum=std::max(maximum,caseMax);++cases;
    }
    std::cout<<"Lifted native lens Metal/raster "<<cases<<" cases: max="<<maximum<<" MAE="<<total/count<<std::endl;
}
void sliderMetalEquivalence(GPU& gpu) {
    Raster raster;double total=0;size_t values=0;int cases=0,maximum=0;
    for(int scale:{1,2,3})for(bool dark:{false,true})for(float value:{0.f,.0001f,.01f,.02f,.25f,.75f,.98f,.99f,.9999f,1.f})
      for(float progress:{0.f,.5f,1.f})for(bool enabled:{false,true}) {
        const auto draw=[&](auto& renderer) {
            auto root=new_shared<LensMetalRoot>(scale,dark);
            for(int i=0;i<8;++i) {
                auto stripe=new_shared<UIView>(NXRect{float(i*24),20,8,120});
                stripe->setBackgroundColor(i%2?UIColor::systemOrange:UIColor::systemGreen);root->addSubview(stripe);
            }
            auto slider=new_shared<UISlider>(NXRect{12,64,156,44});root->addSubview(slider);
            slider->setValue(value);slider->setEnabled(enabled);UISliderTestHarness::pose(slider,progress);
            auto surface=renderer.surface(180*scale,160*scale);auto canvas=surface->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);root->layer()->skiaRender(canvas);
            return renderer.read(surface);
        };
        const auto a=draw(raster),b=draw(gpu);double error=0;size_t count=0;int caseMax=0;
        for(size_t i=0;i<a.size();++i) {
            const int delta=std::abs(int(a[i])-int(b[i]));error+=delta;++count;caseMax=std::max(caseMax,delta);
        }
        // Raster and Graphite use different edge coverage at the pill contour;
        // retain a whole-image error bound as well as a worst-channel bound.
        if(error/count>.15 || caseMax>120) {
            std::cerr<<"Slider raster/Metal mismatch scale="<<scale<<" dark="<<dark<<" value="<<value<<" progress="<<progress<<" enabled="<<enabled<<" max="<<caseMax<<" MAE="<<error/count<<std::endl;
            throw std::runtime_error("Slider raster/Metal disagreement");
        }
        total+=error;values+=count;maximum=std::max(maximum,caseMax);++cases;
    }
    std::cout<<"Slider raster/Metal "<<cases<<" cases max="<<maximum<<" MAE="<<total/values<<std::endl;
}
void sliderMotionMetalEquivalence(GPU& gpu) {
    Raster raster;double total=0;size_t values=0;int maximum=0,cases=0,frames=0;
    for(int scale:{1,2,3})for(bool dark:{false,true}) {
        auto make=[&] {
            auto root=new_shared<LensMetalRoot>(scale,dark);
            auto slider=new_shared<UISlider>(NXRect{12,64,156,44});root->addSubview(slider);slider->setValue(.25);
            UISliderTestHarness::pressed(slider,true);return std::pair(root,slider);
        };
        auto [cpuRoot,cpuSlider]=make();auto [gpuRoot,gpuSlider]=make();
        for(int frame=0;frame<=144;++frame) {
            if(frame==72) {UISliderTestHarness::pressed(cpuSlider,false);UISliderTestHarness::pressed(gpuSlider,false);}
            else if(frame) {UISliderTestHarness::advance(cpuSlider,1./120);UISliderTestHarness::advance(gpuSlider,1./120);}
            const auto draw=[&](auto& renderer,const auto& root) {
                auto surface=renderer.surface(180*scale,160*scale);auto canvas=surface->getCanvas();
                canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);root->layer()->skiaRender(canvas);return renderer.read(surface);
            };
            const auto b=draw(gpu,gpuRoot);++frames;
            if(frame%6)continue;
            const auto a=draw(raster,cpuRoot);double error=0;int caseMax=0;
            for(size_t i=0;i<a.size();++i) {const auto delta=std::abs(int(a[i])-int(b[i]));error+=delta;caseMax=std::max(caseMax,delta);}
            if(error/a.size()>.15||caseMax>120)throw std::runtime_error("Slider motion Gaussian/Metal disagreement");
            total+=error;values+=a.size();maximum=std::max(maximum,caseMax);++cases;
        }
    }
    std::cout<<"Slider motion Metal "<<frames<<" frames / "<<cases<<" raster comparisons max="<<maximum<<" MAE="<<total/values<<std::endl;
}
void switchMetalEquivalence(GPU& gpu) {
    Raster raster;int cases=0,maximum=0;double total=0;size_t values=0;
    for(int scale:{1,2,3})for(bool dark:{false,true})for(bool on:{false,true})for(bool tinted:{false,true})for(bool enabled:{false,true})for(float progress:{0.f,.5f,1.f}) {
        const auto draw=[&](auto& renderer) {
            auto root=new_shared<LensMetalRoot>(scale,dark);root->setBackgroundColor(dark?UIColor::black:UIColor::white);
            auto control=new_shared<UISwitch>(NXRect{58.5f,62.25f,63,28});root->addSubview(control);
            control->setOn(on,false);control->setEnabled(enabled);
            if(tinted){control->setOnTintColor(UIColor::systemPurple);control->setThumbTintColor(UIColor::systemOrange);}
            UISwitchTestHarness::pose(control,progress);
            auto output=renderer.surface(180*scale,160*scale);auto canvas=output->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);root->layer()->skiaRender(canvas);return renderer.read(output);
        };
        const auto a=draw(raster),b=draw(gpu);double error=0;int worst=0;
        for(size_t i=0;i<a.size();++i){const auto delta=std::abs(int(a[i])-int(b[i]));error+=delta;worst=std::max(worst,delta);}
        if(error/a.size()>.3||worst>120) {
            std::cerr<<"Switch mismatch scale="<<scale<<" dark="<<dark<<" on="<<on<<" tinted="<<tinted<<" enabled="<<enabled<<" progress="<<progress<<" max="<<worst<<" MAE="<<error/a.size()<<std::endl;
            throw std::runtime_error("Switch raster/Metal disagreement");
        }
        total+=error;values+=a.size();maximum=std::max(maximum,worst);++cases;
    }
    std::cout<<"Switch Metal/raster "<<cases<<" cases max="<<maximum<<" MAE="<<total/values<<std::endl;
}
template<class Renderer>
void emptyLensForegroundPreservesBackdrop(Renderer& renderer,const char* backend) {
    auto root=new_shared<UIView>(NXRect{0,0,180,160});
    auto source=new_shared<UIView>(root->bounds());root->addSubview(source);
    auto layer=new_shared<LensLayer>();auto owner=new_shared<UIView>(NXRect{30.25f,35.5f,100,44},layer);root->addSubview(owner);
    layer->setSource(source,owner);layer->setLiftState(true,1,1,true,false);layer->setContentsScale(1);
    auto target=renderer.surface(180,160);target->getCanvas()->clear(0x80402060);
    const auto before=renderer.read(target);root->layer()->skiaRender(target->getCanvas());const auto after=renderer.read(target);
    int worst=0;for(size_t i=0;i<before.size();++i)worst=std::max(worst,std::abs(int(before[i])-int(after[i])));
    if(worst>1)throw std::runtime_error("Empty lens foreground added the translucent backdrop twice");
    std::cout<<backend<<" empty lens preserves translucent backdrop max="<<worst<<std::endl;
}
// Cross-process control: run with and without NXKIT_UNBOUNDED_LENS=1.
// Capture the whole target, including the soft exterior and untouched prefix.
void movingLensOutput(GPU& gpu,const char* path) {
    std::ofstream output(path,std::ios::binary);if(!output)throw std::runtime_error("Cannot write lens equivalence output");
    int cases=0;uint64_t groups=0,direct=0,immutable=0;
    for(int scale:{1,3})for(bool dark:{false,true})for(bool selection:{false,true})for(bool vibrant:{false,true})for(float progress:{.35f,1.f})for(int pose=0;pose<4;++pose) {
        auto root=new_shared<LensMetalRoot>(scale,dark);root->setFrame({0,0,500,400});
        if(pose>=2)root->setBackgroundColor(UIColor(32,76,128,140));
        auto source=new_shared<UIView>(root->bounds());root->addSubview(source);
        for(int i=0;i<8;++i) {
            auto mark=new_shared<UIView>(NXRect{float(60+i*47),40,6,300});
            mark->setBackgroundColor(i%2?UIColor(255,230,90,155):UIColor(17,94,231));source->addSubview(mark);
        }
        auto lens=new_shared<LensView>(selection?LensView::Style::large:LensView::Style::small);
        lens->setFrame({200.25f,180.5f,100,44});root->addSubview(lens);
        lens->setWarpsContentBelow(true);lens->setLiftedContentMode(LensView::LiftedContentMode::hideSource);
        lens->setLiftedContentView(source);lens->setContentVibrancyEnabled(vibrant);
        lens->setLifted(true);LensViewTestHarness::progress(lens,progress);
        if(pose==1)lens->setTransform({1.12f,0,0,.88f,3,-2});
        if(pose==2)lens->setTransform({.98f,.2f,-.2f,.98f,3,-2});
        auto surface=gpu.surface(500*scale,400*scale);auto canvas=surface->getCanvas();
        canvas->clear(0x80402060);canvas->scale(scale,scale);
        if(pose==3)canvas->clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(210,174,90,60),12,12),true);
        RenderPerformance metrics;RenderPerformance::current=&metrics;root->layer()->skiaRender(canvas);RenderPerformance::current=nullptr;
        groups+=metrics.boundedLensGroups;direct+=metrics.directLensForegrounds;immutable+=metrics.immutableLensInputs;
        const auto pixels=gpu.read(surface);output.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());++cases;
    }
    std::cout<<"Moving lens output cases="<<cases<<" boundedGroups="<<groups<<" directForegrounds="<<direct<<" immutableInputs="<<immutable<<std::endl;
}
template<class Renderer>
void boundedLensEquivalence(Renderer& renderer,const char* backend) {
    int maximum=0,cases=0;double error=0;size_t values=0;
    for(int sourceWidth:{180,2500})for(bool plainSource:{false,true})for(bool whole:{false,true})for(int scale:{1,2,3})for(bool dark:{false,true})for(bool transparent:{false,true})for(int pose=0;pose<5;++pose) {
        auto root=new_shared<UIView>(NXRect{0,0,180,160});
        auto parent=new_shared<UIView>(root->bounds());parent->setAlpha(.65);root->addSubview(parent);
        auto source=new_shared<UIView>(NXRect{0,0,float(sourceWidth),160});parent->addSubview(source);
        auto probe=new_shared<LensSourceProbe>();probe->setBounds({0,0,180,160});probe->setPosition({90,80});if(plainSource) {
            auto bitmap=renderer.surface(180*scale,160*scale);
            bitmap->getCanvas()->clear(SK_ColorTRANSPARENT);bitmap->getCanvas()->scale(scale,scale);probe->draw(bitmap->getCanvas());
            auto imageLayer=new_shared<CALayer>();imageLayer->setBounds(probe->bounds());imageLayer->setPosition(probe->position());
            imageLayer->setContents(std::make_shared<CGImage>(bitmap->makeImageSnapshot()));imageLayer->setContentsScale(scale);
            source->layer()->addSublayer(imageLayer);
        }else source->layer()->addSublayer(probe);
        auto lens=new_shared<LensLayer>();auto composed=new_shared<ComposedLensProbe>();composed->foreground=lens;
        auto owner=new_shared<UIView>(NXRect{30,35,100,44},whole?std::static_pointer_cast<CALayer>(composed):lens);root->addSubview(owner);
        lens->setBounds({0,0,100,44});
        if(whole) {
            auto background=new_shared<UIView>(NXRect{0,0,100,44});background->setClipsToBounds(true);
            background->layer()->setCornerRadius(22);background->setBackgroundColor(UIColor::black.withAlphaComponent(.25));owner->addSubview(background);
            auto tone=new_shared<BackdropEffectView>(BackdropEffect("uniform shader content; half4 main(float2 p){half4 v=content.eval(p);return half4(v.rgb*.7,v.a);}"));
            tone->setFrame({0,0,100,44});background->addSubview(tone);
        }
        lens->setContentsScale(scale);lens->setSource(source,owner);lens->setLiftState(true,0,1,true,dark);
        std::vector<uint8_t> a,b;
        for(bool bounded:{false,true}) {
            auto output=renderer.surface(180*scale,160*scale);auto* canvas=output->getCanvas();
            canvas->clear(transparent?0x60402080:0xff402080);canvas->scale(scale,scale);
            SkPaint paint;paint.setColor(0x9010e040);canvas->drawRect(SkRect::MakeXYWH(45,25,55,75),paint);
            canvas->translate(pose==1?30.25:30,pose==1?35.5:35);
            if(pose==2)canvas->rotate(13,50,22);
            if(pose==3)canvas->clipRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(2,2,96,40),8,8),true);
            if(pose==4){canvas->scale(.45,1.7);canvas->rotate(7,50,22);}
            RenderPerformance metrics;
            {
                struct Scope {RenderPerformance* previous;~Scope(){RenderPerformance::current=previous;}} scope{RenderPerformance::current};
                RenderPerformance::current=&metrics;
                if(whole){composed->bounded=bounded;composed->cropSource=bounded;composed->renderSublayers(canvas);}else lens->drawSource(canvas,bounded,bounded);
            }
            if(metrics.boundedLenses!=(bounded&&pose<=1?1:0))throw std::runtime_error("Unexpected bounded lens/fallback selection");
            if(metrics.croppedLensSources!=(bounded&&plainSource&&pose!=2&&pose!=4?1:0))throw std::runtime_error("Unexpected lens source crop/fallback selection");
            (bounded?b:a)=renderer.read(output);
        }
        if(probe->draws!=(plainSource?1:2))throw std::runtime_error("Lens isolation changed source callback count");
        int caseMax=0,edgePixels=0;
        for(size_t i=0;i<a.size();i+=4) {
            int pixelMax=0;
            for(int c=0;c<4;++c){const auto delta=std::abs(int(a[i+c])-int(b[i+c]));pixelMax=std::max(pixelMax,delta);error+=delta;}
            caseMax=std::max(caseMax,pixelMax);maximum=std::max(maximum,pixelMax);
            if(pixelMax>1) {
                const auto px=(i/4)%(180*scale),py=(i/4)/(180*scale);
                const auto x=(px+.5)/scale-(pose==1?30.25:30),y=(py+.5)/scale-(pose==1?35.5:35);
                const auto distance=layerCornerDistance({float(x-50),float(y-22)},{100,44},22,CALayerCornerCurve::continuous)*scale;
                // Moving a tessellated path onto a local target can change one
                // 4x-MSAA sample at an exactly coincident edge. Body pixels must
                // remain within one code value; do not allow a displaced fringe.
                if(!whole||pose>1||std::abs(distance)>1||pixelMax>64)throw std::runtime_error("Lens isolation changed body pixels or more than one coverage sample");
                ++edgePixels;
            }
        }
        if(edgePixels>4)throw std::runtime_error("Lens isolation changed a continuous antialias fringe");
        values+=a.size();++cases;
    }
    std::cout<<backend<<" bounded lens "<<cases<<" cases: max="<<maximum<<" MAE="<<error/values<<std::endl;
    if(error/values>.0001)throw std::runtime_error("Bounded lens exceeded the edge-rounding error budget");
}
template<class Renderer>
void standaloneOutputBounds(Renderer& renderer,const char* backend) {
    for(int scale:{1,2,3}) {
        auto root=new_shared<UIView>(NXRect(0,0,200,640));
        root->setBackgroundColor(UIColor(25,105,190));
        auto glass=new_shared<BackdropEffectView>(UIGlassEffect());
        glass->setFrame({70,540,52,52});glass->layer()->setCornerRadius(26);
        root->addSubview(glass);glass->layer()->setContentsScale(scale);
        auto output=renderer.surface(200*scale,640*scale);
        output->getCanvas()->clear(SK_ColorTRANSPARENT);output->getCanvas()->scale(scale,scale);
        root->layer()->presentationOrSelf()->skiaRender(output->getCanvas());
        const auto pixels=renderer.read(output);
        const auto i=(10*scale*200*scale+10*scale)*4;
        if(pixels[i]!=25||pixels[i+1]!=105||pixels[i+2]!=190||pixels[i+3]!=255)
            throw std::runtime_error("Standalone glass replaced content outside its declared output");
    }
    std::cout<<backend<<" standalone output bounds: 1x/2x/3x passed"<<std::endl;
}
// Keep the pre-optimization exterior/body equation as an independent output
// control: a fully covered body must hide the shadow without changing its
// antialiased boundary, tint, translucent backdrop, or exterior pixels.
BackdropEffect referenceSurface(const BackdropEffect& effect) {
    auto source=effect.shaderSource();
    const auto begin=source.find("    // The body replaces the exterior");
    const auto end=source.find("    field=nativeGlassRingComposite",begin);
    if(begin==std::string::npos || end==std::string::npos)
        throw std::runtime_error("Native surface control marker missing");
    source.replace(begin,end-begin,R"sksl(
    float shadowShift=nativeShadowResponse.x*nativeBevel(sdf+nativeShadowResponse.y,nativeShadowShape.z,radius);
    half4 shadow=nativeShadow(xy,xy+normal*shadowShift);
    half4 field=nativeGlassExterior(sdf,shadowSDF,ringSDF,normal,original,shadow,original);
    if(coverage>0)field=mix(field,nativeGlassBackgroundBody(xy,sdf,normal,radius),half(coverage));
)sksl");
    BackdropEffect result(source,effect.backdropShaderName(),effect.maximumSampleRadius(),effect.backdropBlurRadius());
    result.setBackdropScale(effect.backdropScale());result.setOutputOutset(effect.outputOutset());
    if(const auto& pyramid=effect.backdropPyramid())result.setBackdropPyramid(
        pyramid->maximumRadius,pyramid->taps,pyramid->minimumRadius,pyramid->initialTaps);
    const auto runtime=BackdropShader::compile(effect);
    for(const auto& uniform:runtime->uniforms()) {
        const std::string name(uniform.name);
        const auto value=BackdropShader::uniformValues(effect,name);
        if(!value.empty())result.setUniform(name,std::vector<NXFloat>(value.begin(),value.end()));
    }
    return result;
}
template<class Renderer>
void coveredBodyEquivalence(Renderer& renderer,const char* backend) {
    int cases=0,maximum=0;double error=0;size_t values=0;
    for(int scale:{1,2,3})for(bool dark:{false,true})for(float amount:{0.f,.5f,1.f})
    for(auto style:{UIGlassEffect::Style::clear,UIGlassEffect::Style::regular})
    for(bool tinted:{false,true}) {
        UIGlassEffect glass(style);if(tinted)glass.setTintColor(UIColor::systemBlue);
        const auto effect=glass.backdropEffect({88,48},GlassMaterialTestHarness::traits(scale,dark,amount));
        const auto reference=referenceSurface(effect);
        std::vector<uint8_t> a,b;
        for(bool old:{true,false}) {
            auto surface=renderer.surface(128*scale,100*scale);auto canvas=surface->getCanvas();
            canvas->clear(SK_ColorTRANSPARENT);canvas->scale(scale,scale);
            SkPaint paint;paint.setColor(SkColorSetARGB(175,230,80,30));canvas->drawRect(SkRect::MakeWH(128,100),paint);
            paint.setColor(SkColorSetARGB(110,15,105,230));canvas->drawRect(SkRect::MakeLTRB(30,15,100,85),paint);
            auto view=new_shared<BackdropEffectView>(old?reference:effect);view->setFrame({20.25f,25.5f,88,48});
            view->layer()->setCornerRadius(24);view->layer()->setContentsScale(scale);
            view->layer()->skiaRender(canvas);(old?a:b)=renderer.read(surface);
        }
        for(size_t i=0;i<a.size();++i){const auto delta=std::abs(int(a[i])-int(b[i]));maximum=std::max(maximum,delta);error+=delta;}
        values+=a.size();++cases;
    }
    std::cout<<backend<<" covered-body equivalence "<<cases<<" cases: max="<<maximum<<" MAE="<<error/values<<std::endl;
    if(maximum>1)throw std::runtime_error("Covered-body shortcut changed material output");
}
std::array<uint8_t,384> nativeControl(GPU& gpu,float radius,float captureScale,int period=0,bool frozen=false) {
    constexpr int width=1206,height=1206;
    auto input=gpu.surface(width,height);auto canvas=input->getCanvas();canvas->clear(SK_ColorBLACK);
    SkPaint white;white.setColor(SK_ColorWHITE);
    if(period>0)for(int x=period*3/2;x<width;x+=period*3)canvas->drawRect(SkRect::MakeLTRB(x,0,x+period*3/2,height),white);
    else canvas->drawRect(SkRect::MakeLTRB(600,0,width,height),white);
    BackdropEffect effect(std::string("uniform shader content;uniform shader sharpContent;uniform float4 backdropPyramidRect;uniform float4 backdropPyramidInfo;uniform float maximumSampleRadius;uniform float filterScale;uniform float nativePyramidStretch;uniform float nativeMinimumBlurRadius;uniform float4 nativeBlurRadii;")+
        std::string(nativeGlassBlurShaderSource())+"half4 main(float2 p){return nativeAtRadius(p/3,p/3,nativeBlurRadii.x);}");
    effect.setBackdropScale(captureScale);
    effect.setBackdropPyramid(radius*1.600002f,nativeGlassPyramidTaps(),radius*1.6f,nativeGlassPyramidTaps());
    effect.setUniform("nativePyramidStretch",1);effect.setUniform("nativeMinimumBlurRadius",radius);
    effect.setUniform("nativeBlurRadii",std::array<NXFloat,4>{radius,0,0,0});
    auto output=gpu.surface(width,8);output->getCanvas()->translate(0,-height/2+4);
    if(frozen)canvas->scale(3,3);
    SkPaint filtered;filtered.setImageFilter(BackdropShader::makeFilter(effect,BackdropShader::compile(effect),{402,402},0,CALayerCornerCurve::circular,3,0,{},frozen?canvas:nullptr));
    output->getCanvas()->drawImage(input->makeImageSnapshot(),0,0,SkSamplingOptions(),&filtered);
    const auto pixels=gpu.read(output);std::array<uint8_t,384> result;
    for(int i=0;i<384;++i)result[i]=pixels[(4*width+(period>0?300:408)+i)*4];return result;
}
template<class Renderer>
void frozenPyramidEquivalence(Renderer& renderer,const char* backend) {
    int cases=0,maximum=0;double error=0;size_t values=0;
    for(int scale:{2,3})for(bool dark:{false,true})for(bool transparent:{false,true})
    for(float angle:{0.f,13.f})for(bool longShape:{false,true})for(bool aligned:{false,true}) {
        const NXSize size=longShape?NXSize{320,60}:NXSize{88,48};
        UIGlassEffect glass;const auto effect=glass.backdropEffect(size,GlassMaterialTestHarness::traits(scale,dark,.5f));
        const auto runtime=BackdropShader::compile(effect);std::vector<uint8_t> a,b;
        for(bool frozen:{false,true}) {
            auto surface=renderer.surface(420*scale,220*scale);auto* canvas=surface->getCanvas();
            canvas->clear(transparent?0x40305080:0xff305080);canvas->scale(scale,scale);
            SkPaint source;source.setColor(0xa0f04020);
            for(int x=0;x<420;x+=11)canvas->drawRect(SkRect::MakeXYWH(x,0,4,220),source);
            source.setColor(0x8010d080);canvas->drawRect(SkRect::MakeXYWH(30,25,240,70),source);
            canvas->translate(aligned?25.f:25.25f,aligned?45.f:45.5f);canvas->rotate(angle);
            auto filter=BackdropShader::makeFilter(effect,runtime,size,24,CALayerCornerCurve::continuous,scale,0,{},frozen?canvas:nullptr,frozen);
            canvas->scale(1.f/scale,1.f/scale);
            auto rect=SkRect::MakeWH(size.width*scale,size.height*scale);
            rect.outset(effect.outputOutset()*scale,effect.outputOutset()*scale);canvas->clipRect(rect);
            SkPaint paint;paint.setBlendMode(SkBlendMode::kSrc);paint.setImageFilter(filter);
            canvas->saveLayer(SkCanvas::SaveLayerRec(&rect,&paint,SkCanvas::kInitWithPrevious_SaveLayerFlag));canvas->restore();
            (frozen?b:a)=renderer.read(surface);
        }
        int caseMax=0;
        for(size_t i=0;i<a.size();++i){const auto delta=std::abs(int(a[i])-int(b[i]));if(delta>caseMax){caseMax=delta;}maximum=std::max(maximum,delta);error+=delta;}
        values+=a.size();++cases;
    }
    std::cout<<backend<<" frozen-pyramid equivalence "<<cases<<" cases: max="<<maximum<<" MAE="<<error/values<<std::endl;
    if(maximum>12||error/values>.01)throw std::runtime_error("Frozen pyramid exceeded image error bounds");
}
template<class Renderer>
void circleFieldEquivalence(Renderer& renderer,const char* backend) {
    const std::string legacy=R"sksl(
float3 legacySample(float2 p,float2 halfDim,float r) {
    if(cornerCurve>.5 && r>0)return continuousCornerSample(p,halfDim);
    float2 q=abs(p)-halfDim+r;
    float2 direction=float2(p.x>=0?1:-1,p.y>=0?1:-1);
    float2 normal;
    if(max(q.x,q.y)>0)normal=direction*max(q,0)/max(length(max(q,0)),.000001);
    else normal=q.x>q.y?float2(direction.x,0):float2(0,direction.y);
    return float3(length(max(q,0))+min(max(q.x,q.y),0)-r,normal);
}
)sksl";
    int maximum=0;
    for(int scale:{1,2,3})for(float curve:{0.f,1.f}) {
        std::vector<uint8_t> a,b;
        for(bool old:{true,false}) {
            auto source=std::string("uniform float cornerCurve;\n")+layerCornerShaderUniforms+layerCornerShaderSource()+legacy
                +"half4 main(float2 p){float3 s="+(old?"legacySample":"roundedBoxSample")
                +"(p-float2(50.5),float2(22.5),22.5);return half4(half(s.x/100+.5),half2(s.yz*.5+.5),1);}";
            SkRuntimeShaderBuilder builder(compile(source));builder.uniform("cornerCurve")=curve;
            const auto data=layerCornerShaderData({45,45},22.5f);
            for(size_t i=0;i<data.size();++i)builder.uniform(layerCornerShaderNames[i]).set(data[i].data(),4);
            auto surface=renderer.surface(101*scale,101*scale);auto* canvas=surface->getCanvas();canvas->scale(scale,scale);
            SkPaint paint;paint.setShader(builder.makeShader());canvas->drawRect(SkRect::MakeWH(101,101),paint);
            (old?a:b)=renderer.read(surface);
        }
        for(size_t i=0;i<a.size();++i)maximum=std::max(maximum,std::abs(int(a[i])-int(b[i])));
    }
    std::cout<<backend<<" circle distance/normal equivalence: max="<<maximum<<std::endl;
    if(maximum>1)throw std::runtime_error("Circle shortcut changed distance or normals");
}
}
int main(int argc,char** argv) {@autoreleasepool{try {
    GPU gpu;
    const auto prepared=gpu.cache->statistics();
    if(argc==2&&std::string_view(argv[1])=="--switch-only") {
        switchMetalEquivalence(gpu);
        const auto rendered=gpu.cache->statistics();
        if(std::getenv("NXKIT_METAL_BUNDLED_ONLY")&&(rendered.libraryCompiles!=prepared.libraryCompiles||rendered.pipelineCompiles!=prepared.pipelineCompiles))throw std::runtime_error("Bundled switch replay compiled shaders");
        return 0;
    }
    if(argc==3&&std::string_view(argv[1])=="--lens-state-output"){movingLensOutput(gpu,argv[2]);return 0;}
    if(argc==2&&std::string_view(argv[1])=="--slider-only") {
        sliderMetalEquivalence(gpu);
        sliderMotionMetalEquivalence(gpu);
        if(std::getenv("NXKIT_METAL_BUNDLED_ONLY")) {
            const auto rendered=gpu.cache->statistics();
            if(rendered.libraryCompiles!=prepared.libraryCompiles || rendered.pipelineCompiles!=prepared.pipelineCompiles)
                throw std::runtime_error("Bundled slider replay compiled shaders during rendering");
            std::cout<<"Bundled slider replay: zero library/pipeline compilations during rendering"<<std::endl;
        }
        return 0;
    }
    if(argc==3&&std::string_view(argv[1])=="--mask-output") {
        const auto pixels=maskPrefixSemantics(gpu);std::ofstream output(argv[2],std::ios::binary);
        output.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());if(!output)throw std::runtime_error("Mask fixture output failed");return 0;
    }
    if(argc==2&&std::string_view(argv[1])=="--directional-only") {
        directionalFilterContracts();Raster raster;directionalBlurEquivalence(raster,"Raster");directionalBlurEquivalence(gpu,"Metal");return 0;
    }
    if(argc==2&&std::string_view(argv[1])=="--press-only") {
        Raster raster;
        const auto resolve=[](const auto& material,NXSize size,bool dark,int scale,float amount) {
            return material.backdropEffect(size,GlassMaterialTestHarness::traits(scale,dark,amount));
        };
        glassPressCoverageFixture(raster,resolve);glassPressCoverageFixture(gpu,resolve);return 0;
    }
    const auto clearPixels=metalClearSemantics(gpu);
    if(argc==3&&std::string_view(argv[1])=="--clear-output") {
        std::ofstream output(argv[2],std::ios::binary);output.write(reinterpret_cast<const char*>(clearPixels.data()),clearPixels.size());
        if(!output)throw std::runtime_error("Clear fixture output failed");return 0;
    }
    if(!(argc==2&&std::string_view(argv[1])=="--lens-only"))maskPrefixSemantics(gpu);
    Raster raster;emptyLensForegroundPreservesBackdrop(raster,"Raster");emptyLensForegroundPreservesBackdrop(gpu,"Metal");boundedLensEquivalence(raster,"Raster");
    boundedLensEquivalence(gpu,"Metal");
    liftedLensMetalEquivalence(gpu);
    if(std::getenv("NXKIT_METAL_BUNDLED_ONLY")) {
        const auto rendered=gpu.cache->statistics();
        if(rendered.libraryCompiles!=prepared.libraryCompiles || rendered.pipelineCompiles!=prepared.pipelineCompiles)
            throw std::runtime_error("Bundled lens replay compiled shaders during rendering");
        std::cout<<"Bundled lens replay: zero library/pipeline compilations during rendering"<<std::endl;
    }
    if(argc==2&&std::string_view(argv[1])=="--lens-only"){std::cout<<"Lens isolation tests passed"<<std::endl;return 0;}
    directionalFilterContracts();directionalBlurEquivalence(raster,"Raster");directionalBlurEquivalence(gpu,"Metal");
    circleFieldEquivalence(raster,"Raster");captureEquivalence(raster,"Raster");standaloneOutputBounds(raster,"Raster");coveredBodyEquivalence(raster,"Raster");frozenPyramidEquivalence(raster,"Raster");
    circleFieldEquivalence(gpu,"Metal");captureEquivalence(gpu,"Metal");standaloneOutputBounds(gpu,"Metal");coveredBodyEquivalence(gpu,"Metal");frozenPyramidEquivalence(gpu,"Metal");
    for(bool frozen:{false,true}) {
    double step=0,stripe=0;size_t stepValues=0,stripeValues=0;
    for(const auto& ref:nativeBlurReferences){const auto pixels=nativeControl(gpu,ref.radius,ref.scale,0,frozen);
        for(size_t i=0;i<pixels.size();++i)step+=std::abs(int(pixels[i])-int(ref.samples[i]));stepValues+=pixels.size();}
    for(const auto& ref:nativeBlurStripes){const auto pixels=nativeControl(gpu,ref.radius,ref.scale,ref.period,frozen);
        for(size_t i=0;i<pixels.size();++i)stripe+=std::abs(int(pixels[i])-int(ref.samples[i]));stripeValues+=pixels.size();}
    std::cout<<(frozen?"Frozen":"Lazy")<<" native step MAE="<<step/stepValues<<"; stripe MAE="<<stripe/stripeValues<<std::endl;
    if(step/stepValues>=.5||stripe/stripeValues>=1.8)throw std::runtime_error("Native blur accuracy regressed");
    }
    std::cout<<"Simulator glass blur tests passed"<<std::endl;return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<std::endl;return 1;}}}
