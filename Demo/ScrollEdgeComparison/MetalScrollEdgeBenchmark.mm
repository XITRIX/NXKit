// Real Graphite/Metal coverage and frame timings for the public comparison scene.
// One process covers one immutable scale/theme profile; harvest all six profiles.
#define NXKIT_SCROLL_EDGE_REFERENCE_NO_MAIN
#include "PortableReference.cpp"
#include "platforms/apple/MetalPipelineCache.hpp"
#include "NXMetalShaderABI.hpp"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#import <Metal/Metal.h>

#include <chrono>
#include <stdexcept>

namespace NXKit {
std::unique_ptr<SkiaCtx> MakeSkiaCtx() {
    throw std::runtime_error("The scroll-edge Metal benchmark supplies its own context");
}
}

namespace {
using namespace NXKit;
using Clock = std::chrono::steady_clock;
constexpr const char* usage="Usage: MetalScrollEdgeBenchmark output.csv [--scale 1|2|3] [--light|--dark] [--passes N] [--steps N] [--allow-cold]";
double milliseconds(Clock::duration elapsed) {
    return std::chrono::duration<double, std::milli>(elapsed).count();
}
double percentile(std::vector<double> values, double fraction) {
    if (values.empty()) return 0;
    std::sort(values.begin(), values.end());
    return values[std::min(values.size()-1, std::size_t(std::ceil(fraction*values.size())-1))];
}
class MetalContext final : public SkiaCtx {
public:
    const bool dark;
    const int density;
    std::shared_ptr<Internal::MetalPipelineCache> cache;
    std::unique_ptr<skgpu::graphite::Context> gpu;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    sk_sp<SkSurface> surface;
    MetalContext(bool isDark, int scale) : dark(isDark), density(scale) {
        auto device = MTLCreateSystemDefaultDevice();
        if (!device) throw std::runtime_error("Metal device unavailable");
        skgpu::graphite::MtlBackendContext backend;
        backend.fDevice = sk_ret_cfp((CFTypeRef)device);
        backend.fQueue = sk_ret_cfp((CFTypeRef)[device newCommandQueue]);
        cache = std::make_shared<Internal::MetalPipelineCache>(backend.fDevice.get());
        backend.fPipelineCache = cache;
        skgpu::graphite::ContextOptions options;
        options.fRequireOrderedRecordings = true;
        cache->configure(backend, options);
        gpu = skgpu::graphite::ContextFactory::MakeMetal(backend, options);
        if (!gpu) throw std::runtime_error("Graphite Metal context unavailable");
        recorder = createGraphiteRecorder(gpu.get());
        surface = SkSurfaces::RenderTarget(recorder.get(), SkImageInfo::Make(
            402*density, 874*density, kBGRA_8888_SkColorType, kPremul_SkAlphaType));
        if (!surface) throw std::runtime_error("Graphite render target unavailable");
        fontMgr = SkFontMgr_New_CoreText(nullptr);
    }
    ~MetalContext() override {
        surface.reset();
        finishGraphiteWork(gpu.get(), recorder.get());
        recorder.reset();
        gpu.reset();
        cache->save();
    }
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(), NXFloat size=17) const override {
        return Internal::appleSystemTypeface(style, size);
    }
    NXKit::NXSize getSize() override { return {402,874}; }
    float getScaleFactor() override { return density; }
    UIEdgeInsets deviceSafeAreaInsets() override { return {62,0,34,0}; }
    std::array<NXFloat,4> windowCornerRadii() override { return {62,62,62,62}; }
    UIUserInterfaceStyle getThemeMode() override {
        return dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
    }
    sk_sp<SkSurface> getBackbufferSurface() override { return surface; }
    void swapBuffers() override {}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return gpu.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() override { return recorder.get(); }
};
struct Pass {
    std::uint64_t frames=0, libraries=0, pipelines=0, archiveHits=0;
    std::vector<double> cpu, total;
};
struct Options {
    std::filesystem::path output;
    bool dark=true, allowCold=false;
    int scale=3, passes=3, steps=6;
};
Options options(int argc, char** argv) {
    if (argc < 2 || std::string_view(argv[1]).starts_with('-')) throw std::invalid_argument(usage);
    Options result; result.output=argv[1];
    for (int index=2; index<argc; ++index) {
        const std::string arg=argv[index];
        const auto integer=[&] {
            if (++index==argc) throw std::invalid_argument("Missing benchmark option value");
            std::size_t consumed=0; const std::string value=argv[index];
            const int parsed=std::stoi(value, &consumed);
            if (consumed!=value.size()) throw std::invalid_argument("Invalid integer option");
            return parsed;
        };
        if (arg=="--scale") result.scale=integer();
        else if (arg=="--passes") result.passes=integer();
        else if (arg=="--steps") result.steps=integer();
        else if (arg=="--light") result.dark=false;
        else if (arg=="--dark") result.dark=true;
        else if (arg=="--allow-cold") result.allowCold=true;
        else throw std::invalid_argument("Unknown benchmark option: "+arg);
    }
    if (result.scale<1 || result.scale>3 || result.passes<1 || result.passes>10 || result.steps<1 || result.steps>120)
        throw std::invalid_argument("Expected scale1..3, passes1..10, and steps1..120");
    return result;
}
}

int main(int argc, char** argv) { @autoreleasepool { try {
    using namespace NXKit;
    using namespace ScrollEdgeReference;
    if (argc==2 && std::string_view(argv[1])=="--help") {std::cout << usage << '\n';return 0;}
    const auto args=options(argc, argv);
    if (!args.output.parent_path().empty()) std::filesystem::create_directories(args.output.parent_path());
    std::ofstream out(args.output);
    if (!out) throw std::runtime_error("Cannot create benchmark CSV");
    out << "pass,scale,dark,case,phase,frame,x,y,cpu_ms,sync_ms,total_ms,library_hits,library_compiles,archive_hits,pipeline_compiles,failures\n";
    out << std::setprecision(10);
    auto context=std::make_shared<MetalContext>(args.dark, args.scale);
    SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();
    UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    std::vector<Pass> passes(args.passes);
    auto draw=[&](int pass, const std::string& name, const char* phase, int frame, Scene& scene) {
        const auto begin=Clock::now();
        scene.window->drawAndLayoutTreeIfNeeded();
        auto* canvas=context->surface->getCanvas();
        canvas->clear(args.dark ? SK_ColorBLACK : SkColorSetRGB(242,242,247));
        canvas->save(); canvas->scale(args.scale,args.scale);
        UITraitCollection::setCurrent(scene.window->traitCollection());
        scene.window->layer()->presentationOrSelf()->skiaRender(canvas);
        canvas->restore();
        context->flushAndSubmit(context->surface);
        const auto submitted=Clock::now();
        context->gpu->submit(skgpu::graphite::SyncToCpu::kYes);
        const auto complete=Clock::now();
        context->gpu->checkAsyncWorkCompletion();
        DispatchQueueTestHarness::pump();
        const auto stats=context->cache->statistics();
        if (stats.failures) throw std::runtime_error("Metal pipeline preparation failed");
        const auto offset=scene.screen->scroll->contentOffset();
        const auto cpu=milliseconds(submitted-begin), sync=milliseconds(complete-submitted), total=milliseconds(complete-begin);
        out << pass << ',' << args.scale << ',' << args.dark << ',' << name << ',' << phase << ',' << frame
            << ',' << offset.x << ',' << offset.y << ',' << cpu << ',' << sync << ',' << total
            << ',' << stats.libraryHits << ',' << stats.libraryCompiles << ',' << stats.archiveHits
            << ',' << stats.pipelineCompiles << ',' << stats.failures << '\n';
        auto& summary=passes[pass]; ++summary.frames; summary.cpu.push_back(cpu); summary.total.push_back(total);
    };
    for (int pass=0; pass<args.passes; ++pass) {
        const auto before=context->cache->statistics();
        for (const auto style : {UIScrollEdgeEffectStyle::automatic, UIScrollEdgeEffectStyle::soft, UIScrollEdgeEffectStyle::hard}) {
            const std::string styleName=style==UIScrollEdgeEffectStyle::automatic ? "automatic" : style==UIScrollEdgeEffectStyle::soft ? "soft" : "hard";
            for (int placement=0; placement<3; ++placement) {
                Scene scene(placement!=0, placement==2, true);
                const auto scroll=scene.screen->scroll;
                const std::array edges {scroll->topEdgeEffect(),scroll->leftEdgeEffect(),scroll->bottomEdgeEffect(),scroll->rightEdgeEffect()};
                for (const auto& edge:edges) edge->setStyle(style);
                const auto inset=scroll->adjustedContentInset();
                const auto size=scroll->contentSize(), viewport=scroll->bounds().size;
                const NXKit::NXPoint start {-inset.left,-inset.top};
                const NXKit::NXPoint end {std::max(start.x,size.width+inset.right-viewport.width),std::max(start.y,size.height+inset.bottom-viewport.height)};
                const NXKit::NXPoint middle {(start.x+end.x)*.5f,(start.y+end.y)*.5f};
                const auto name=styleName+"-"+(placement==0?"discovered":placement==1?"obstructed":"manual");
                int frame=0;
                const auto at=[&](const char* phase, NXKit::NXPoint offset) {scroll->setContentOffset(offset,false);draw(pass,name,phase,frame++,scene);};
                at("overscroll-start",{start.x-16,start.y-16});
                at("start",start); at("subpixel-start",{start.x+.25f,start.y+.5f});
                for (int step=0; step<=args.steps; ++step) {
                    const auto fraction=NXFloat(step)/args.steps;
                    const NXKit::NXPoint point {start.x+(end.x-start.x)*fraction,start.y+(end.y-start.y)*fraction};
                    at("diagonal",point); at("horizontal",{point.x,middle.y}); at("vertical",{middle.x,point.y});
                }
                at("subpixel-end",{end.x-.25f,end.y-.5f}); at("end",end);
                at("overscroll-end",{end.x+16,end.y+16});
                for (const auto& edge:edges) edge->setHidden(true);
                at("hidden",middle);
                for (std::size_t index=0; index<edges.size(); ++index) edges[index]->setHidden(index%2==0);
                at("alternating-hidden",middle);
                for (std::size_t index=0; index<edges.size(); ++index) {
                    edges[index]->setHidden(false);
                    edges[index]->setStyle(index%2 ? UIScrollEdgeEffectStyle::hard : UIScrollEdgeEffectStyle::soft);
                }
                at("mixed-styles",middle);
                auto appMask=new_shared<UIView>(NXRect{0,0,viewport.width,viewport.height*.75f});
                appMask->setBackgroundColor(UIColor::white);scroll->setMask(appMask);at("application-mask",middle);
                scroll->setMask(nullptr);at("application-mask-removed",middle);
                out.flush();
            }
        }
        {
            Scene scene(false,false,true,true);
            const auto scroll=scene.screen->scroll;
            scroll->setContentOffset({100,100},false);
            int frame=0;
            for (const auto style : {UIScrollEdgeEffectStyle::automatic, UIScrollEdgeEffectStyle::soft, UIScrollEdgeEffectStyle::hard}) {
                for (const auto& edge : {scroll->topEdgeEffect(),scroll->leftEdgeEffect(),scroll->bottomEdgeEffect(),scroll->rightEdgeEffect()}) edge->setStyle(style);
                draw(pass,"sparse-content","transparent-gap",frame++,scene);
            }
        }
        {
            Scene scene(false,false,true);
            const auto scroll=scene.screen->scroll;
            scroll->setContentOffset({100,100},false);
            const NXKit::NXRect frame {140,340,120,64};
            auto black=new_shared<UIView>(frame);black->setBackgroundColor(UIColor::black);scroll->addSubview(black);
            UIGlassEffect material(UIGlassEffectStyle::regular);material.setAdaptsAppearanceToBackdrop(true);
            auto glass=new_shared<BackdropEffectView>(material);glass->setFrame(frame);scroll->addSubview(glass);
            auto label=new_shared<UILabel>();label->setText("Observe");label->setFrame({0,0,120,64});glass->addSubview(label);
            for(int frameIndex=0;frameIndex<3;++frameIndex)draw(pass,"adaptive-glass","current-device-observer",frameIndex,scene);
            if(!label->traitCollection()||label->traitCollection()->userInterfaceStyle()!=UIUserInterfaceStyle::dark)
                throw std::runtime_error("Adaptive glass did not observe its black current-device backdrop inside the masked scroll viewport");
        }
        {
            Scene scene(false,false,true);
            scene.screen->scroll->setContentOffset({100,100},false);
            auto probe=new_shared<BackdropEffectView>(UIBlurEffect::effectWithVariableBlurRadius(24,imageMask("ramp")));
            scene.window->addSubview(probe);
            int frame=0;
            for (const auto* mask : {"zero","one","ramp"}) for (const NXFloat radius : {0.f,1.f,8.f,24.f,48.f}) {
                probe->setEffect(UIBlurEffect::effectWithVariableBlurRadius(radius,imageMask(mask)));
                for (int phase=0; phase<2; ++phase) {
                    probe->setFrame({24+.25f*phase,220+.5f*phase,354,256});
                    const auto name=std::string("private-")+mask+"-radius"+std::to_string(int(radius));
                    draw(pass,name,"variable-blur",frame++,scene);
                }
            }
            bool rejected=false;
            try { (void)UIBlurEffect::effectWithVariableBlurRadius(24,imageMask("nil")); }
            catch (const std::invalid_argument&) { rejected=true; }
            if (!rejected) throw std::runtime_error("Variable blur unexpectedly accepted a nil mask");
            probe->removeFromSuperview();draw(pass,"private-mask-removed","baseline",frame,scene);
        }
        const auto after=context->cache->statistics();
        auto& summary=passes[pass];
        summary.libraries=after.libraryCompiles-before.libraryCompiles;
        summary.pipelines=after.pipelineCompiles-before.pipelineCompiles;
        summary.archiveHits=after.archiveHits-before.archiveHits;
        std::cout << "Pass " << pass << ": " << summary.frames << " frames, " << summary.libraries
                  << " library / " << summary.pipelines << " pipeline compilations, cpu p50/p95 "
                  << percentile(summary.cpu,.5) << '/' << percentile(summary.cpu,.95)
                  << " ms, synchronized total p50/p95 " << percentile(summary.total,.5) << '/'
                  << percentile(summary.total,.95) << " ms\n" << std::flush;
    }
    out.close();
    context->cache->save();
    const auto stats=context->cache->statistics();
    auto reportPath=args.output; reportPath.replace_extension(".json");
    std::ofstream report(reportPath);
    report << "{\"abi\":" << std::quoted(NXKIT_METAL_SHADER_ABI) << ",\"scale\":" << args.scale
           << ",\"dark\":" << args.dark << ",\"steps\":" << args.steps << ",\"passes\":[";
    for (std::size_t index=0; index<passes.size(); ++index) {
        if (index) report << ','; const auto& pass=passes[index];
        report << "{\"frames\":" << pass.frames << ",\"libraryCompiles\":" << pass.libraries
               << ",\"pipelineCompiles\":" << pass.pipelines << ",\"archiveHits\":" << pass.archiveHits
               << ",\"cpuP50MS\":" << percentile(pass.cpu,.5) << ",\"cpuP95MS\":" << percentile(pass.cpu,.95)
               << ",\"synchronizedTotalP50MS\":" << percentile(pass.total,.5)
               << ",\"synchronizedTotalP95MS\":" << percentile(pass.total,.95) << '}';
    }
    report << "],\"libraryCompiles\":" << stats.libraryCompiles << ",\"pipelineCompiles\":" << stats.pipelineCompiles
           << ",\"failures\":" << stats.failures << "}\n";
    UIApplication::shared.reset();SkiaCtx::_main.reset();context.reset();
    if (!args.allowCold && (passes.back().libraries || passes.back().pipelines)) {
        std::cerr << "Final repeated pass still compiled shaders or pipelines\n";return 1;
    }
    return 0;
} catch (const std::exception& error) { std::cerr << error.what() << '\n';return 2; } } }
