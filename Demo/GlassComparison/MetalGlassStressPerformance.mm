// Unpaced native Metal stress workload. Synchronization is measured separately
// from recording/submission; total time includes GPU completion, not presentation.
#include "../app/Screens/GlassDemoViewController/GlassStressView.hpp"
#include "RenderPerformance.hpp"
#include "platforms/apple/AppleSystemFont.hpp"
#include "platforms/apple/MetalPipelineCache.hpp"
#include "NXMetalShaderABI.hpp"
#include <UIApplication.h>
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#import <Metal/Metal.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>

using namespace NXKit;
namespace NXKit {
std::unique_ptr<SkiaCtx> MakeSkiaCtx() { throw std::runtime_error("Benchmark supplies its context"); }
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*, std::map<std::string,std::any>) { return true; }
class SizeClassTestHarness {
public:
    static void refresh(const std::shared_ptr<UIWindow>& window) { window->updateTraitsFromPlatform(); }
};
}
namespace {
using Clock = std::chrono::steady_clock;
double ms(Clock::duration time) { return std::chrono::duration<double,std::milli>(time).count(); }
class Context final : public SkiaCtx {
public:
    int density;
    bool dark;
    std::shared_ptr<Internal::MetalPipelineCache> cache;
    std::unique_ptr<skgpu::graphite::Context> gpu;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    sk_sp<SkSurface> surface;
    Context(int scale, bool isDark) : density(scale), dark(isDark) {
        auto device=MTLCreateSystemDefaultDevice();
        if (!device) throw std::runtime_error("Metal unavailable");
        skgpu::graphite::MtlBackendContext backend;
        backend.fDevice=sk_ret_cfp((CFTypeRef)device);
        backend.fQueue=sk_ret_cfp((CFTypeRef)[device newCommandQueue]);
        cache=std::make_shared<Internal::MetalPipelineCache>(backend.fDevice.get());
        backend.fPipelineCache=cache;
        skgpu::graphite::ContextOptions options; options.fRequireOrderedRecordings=true;
        cache->configure(backend,options);
        gpu=skgpu::graphite::ContextFactory::MakeMetal(backend,options);
        if (!gpu) throw std::runtime_error("Graphite unavailable");
        recorder=createGraphiteRecorder(gpu.get());
        surface=SkSurfaces::RenderTarget(recorder.get(),SkImageInfo::Make(
            402*density,874*density,kBGRA_8888_SkColorType,kPremul_SkAlphaType));
        if (!surface) throw std::runtime_error("Render target unavailable");
        fontMgr=SkFontMgr_New_CoreText(nullptr);
    }
    ~Context() override {
        surface.reset(); finishGraphiteWork(gpu.get(),recorder.get());
        recorder.reset(); gpu.reset(); cache->save();
    }
    NXKit::NXSize getSize() override { return {402,874}; }
    float getScaleFactor() override { return density; }
    UIEdgeInsets deviceSafeAreaInsets() override { return {62,0,34,0}; }
    UIUserInterfaceStyle getThemeMode() override { return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; }
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17) const override { return Internal::appleSystemTypeface(style,size); }
    sk_sp<SkSurface> getBackbufferSurface() override { return surface; }
    void swapBuffers() override {}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return gpu.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() override { return recorder.get(); }
};
}
int main(int argc,char** argv) { @autoreleasepool { try {
    if (argc<2) throw std::invalid_argument("output.csv [--scale N] [--light] [--frames N] [--passes N] [--glass-tint 0...1 | --glass-tint-sweep]");
    int scale=3,frames=160,passes=3; bool dark=true;
    bool glassTintSweep=false;
    std::optional<NXFloat> glassTintAmount;
    for(int i=2;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--light") dark=false;
        else if(arg=="--glass-tint-sweep") glassTintSweep=true;
        else if(arg=="--glass-tint"&&i+1<argc) glassTintAmount=std::stof(argv[++i]);
        else if((arg=="--scale"||arg=="--frames"||arg=="--passes")&&i+1<argc) {
            const int value=std::stoi(argv[++i]);
            if(arg=="--scale")scale=value;else if(arg=="--frames")frames=value;else passes=value;
        } else throw std::invalid_argument("Unknown option: "+arg);
    }
    if(scale<1||scale>3||frames<8||frames>10000||passes<1||passes>100)throw std::invalid_argument("Invalid workload size");
    std::ofstream out(argv[1]);if(!out)throw std::runtime_error("Cannot open report");
    out<<"pass,frame,phase,scale,dark,layout_ms,render_ms,submit_ms,sync_ms,total_ms,views,layers,glass,library_compiles,pipeline_compiles\n"<<std::setprecision(10);
    auto context=std::make_shared<Context>(scale,dark);SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    UIApplication::shared->setOverrideGlassTintAmount(glassTintAmount);
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();
    auto screen=new_shared<GlassStressView>();controller->setView(screen);
    window->setRootViewController(controller);window->makeKeyAndVisible();
    window->drawAndLayoutTreeIfNeeded();
    const auto maximum=std::max(0.f,screen->verticalScroll()->contentSize().height-screen->bounds().height());
    for(int pass=0;pass<passes;++pass)for(int frame=0;frame<frames;++frame) { @autoreleasepool {
        if(glassTintSweep) {
            UIApplication::shared->setOverrideGlassTintAmount(NXFloat(frame)/NXFloat(frames-1));
            SizeClassTestHarness::refresh(window);
        }
        const bool idle=frame<frames/4;
        const float progress=float(frame-frames/4)/float(frames-frames/4-1);
        const float travel=1-std::abs(2*progress-1);
        screen->verticalScroll()->setContentOffset({0,idle?0:maximum*travel},false);
        for(int row=0;row<GlassStressView::railCount;++row)screen->horizontalScrolls()[row]->setContentOffset({idle?0.f:(1200+row*41)*travel,0},false);
        Internal::RenderPerformance stats;Internal::RenderPerformance::current=&stats;
        const auto start=Clock::now();
        auto begin=start;window->drawAndLayoutTreeIfNeeded();stats.layoutMs=ms(Clock::now()-begin);
        auto* canvas=context->surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SK_ColorWHITE);
        canvas->save();canvas->scale(scale,scale);UITraitCollection::setCurrent(window->traitCollection());
        begin=Clock::now();window->layer()->presentationOrSelf()->skiaRender(canvas);stats.renderMs=ms(Clock::now()-begin);canvas->restore();
        begin=Clock::now();context->flushAndSubmit(context->surface);stats.submitMs=ms(Clock::now()-begin);
        const auto submitted=Clock::now();context->gpu->submit(skgpu::graphite::SyncToCpu::kYes);
        const auto complete=Clock::now();context->gpu->checkAsyncWorkCompletion();
        Internal::RenderPerformance::current=nullptr;
        const auto cache=context->cache->statistics();if(cache.failures)throw std::runtime_error("Metal pipeline failure");
        out<<pass<<','<<frame<<','<<(idle?"idle":"scroll")<<','<<scale<<','<<dark<<','<<stats.layoutMs<<','<<stats.renderMs<<','<<stats.submitMs<<','<<ms(complete-submitted)<<','<<ms(complete-start)<<','<<stats.views<<','<<stats.layers<<','<<stats.glass<<','<<cache.libraryCompiles<<','<<cache.pipelineCompiles<<'\n';
        if(frame==frames-1)std::cout<<"Pass "<<pass<<", "<<frames<<" frames, "<<cache.libraryCompiles<<" library / "<<cache.pipelineCompiles<<" pipeline compiles\n"<<std::flush;
    }}
    context->cache->save();std::cout<<"ABI "<<NXKIT_METAL_SHADER_ABI<<'\n';
    window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();context.reset();
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 2; } } }
