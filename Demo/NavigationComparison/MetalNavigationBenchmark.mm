// Graphite/Metal CPU submission and synchronized completion timings; no display pacing.
#include <UIKit.h>
#include <UIApplication.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <UIPanGestureRecognizer.h>
#include <SkiaCtx.h>
#include "platforms/apple/AppleSystemFont.hpp"
#include "platforms/apple/MetalPipelineCache.hpp"
#include "UINavigationBarInternal.hpp"
#include "NavigationBarMotion.hpp"
#include "../app/Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include "../app/DemoPerformanceReplay.hpp"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/effects/SkImageFilters.h"
#import <Metal/Metal.h>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
// Observe completed Metal work separately from synchronous benchmark latency.
@interface NXBenchmarkQueue : NSProxy {
    id<MTLCommandQueue> _queue;
}
@property(nonatomic,strong) NSMutableArray<id<MTLCommandBuffer>>* buffers;
- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue;
@end
@implementation NXBenchmarkQueue
- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue { _queue=queue;_buffers=[NSMutableArray array];return self; }
- (NSMethodSignature*)methodSignatureForSelector:(SEL)s {return [(id)_queue methodSignatureForSelector:s];}
- (void)forwardInvocation:(NSInvocation*)v {[v invokeWithTarget:_queue];}
- (id<MTLCommandBuffer>)commandBufferWithDescriptor:(MTLCommandBufferDescriptor*)d {auto b=[_queue commandBufferWithDescriptor:d];[_buffers addObject:b];return b;}
- (id<MTLCommandBuffer>)commandBuffer {auto b=[_queue commandBuffer];[_buffers addObject:b];return b;}
- (id<MTLCommandBuffer>)commandBufferWithUnretainedReferences {auto b=[_queue commandBufferWithUnretainedReferences];[_buffers addObject:b];return b;}
@end
using namespace NXKit;
namespace NXKit {
std::unique_ptr<SkiaCtx> MakeSkiaCtx(){throw std::runtime_error("Metal benchmark supplies its own context");}
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class DispatchQueueTestHarness {public:static void pump(){DispatchQueue::main()->performAll();}};
class UIGestureRecognizerTestHarness {
public:
    static void notify(const std::shared_ptr<UIGestureRecognizer>& gesture){gesture->notifyStateChanged();}
    std::shared_ptr<UIWindow> window;std::shared_ptr<UITouch> touch;std::shared_ptr<UIEvent> event;Timer origin;
    explicit UIGestureRecognizerTestHarness(const std::shared_ptr<UIWindow>& w):window(w){touch=new_shared<UITouch>(492,NXPoint{2,300},origin);event=new_shared<UIEvent>();event->_allTouches.insert(touch);touch->_window=window;window->sendEvent(event);}
    void send(UITouchPhase phase,NXPoint point,double milliseconds){touch->updateAbsoluteLocation(point);touch->_timestamp=origin;const auto us=origin.startTime.tv_usec+long(milliseconds*1000);touch->_timestamp.startTime.tv_sec+=us/1000000;touch->_timestamp.startTime.tv_usec=us%1000000;touch->_phase=phase;window->sendEvent(event);}
};
}
namespace {
class Context final:public SkiaCtx {
public:
    bool dark=true;
    bool full=false;float density=3;
    NXBenchmarkQueue* diagnosticQueue;
    std::shared_ptr<Internal::MetalPipelineCache> cache;
    std::unique_ptr<skgpu::graphite::Context> gpu;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    sk_sp<SkSurface> surface;
    Context(bool fullFrame,float scale):full(fullFrame),density(scale){
        auto device=MTLCreateSystemDefaultDevice();if(!device)throw std::runtime_error("Metal device unavailable");
        skgpu::graphite::MtlBackendContext backend;backend.fDevice=sk_ret_cfp((CFTypeRef)device);diagnosticQueue=[[NXBenchmarkQueue alloc] initWithQueue:[device newCommandQueue]];backend.fQueue=sk_ret_cfp((CFTypeRef)diagnosticQueue);
        cache=std::make_shared<Internal::MetalPipelineCache>(backend.fDevice.get());backend.fPipelineCache=cache;
        skgpu::graphite::ContextOptions options;options.fRequireOrderedRecordings=true;
        if (const auto rasterPaths=std::getenv("NXKIT_RASTER_PATHS");rasterPaths&&rasterPaths[0]=='1')
            options.fInternalMultisampleCount=skgpu::graphite::SampleCount::k1;cache->configure(backend,options);
        gpu=skgpu::graphite::ContextFactory::MakeMetal(backend,options);if(!gpu)throw std::runtime_error("Graphite context unavailable");
        recorder=createGraphiteRecorder(gpu.get());
        surface=SkSurfaces::RenderTarget(recorder.get(),SkImageInfo::Make(int(402*density),int((full?874:80)*density),kBGRA_8888_SkColorType,kPremul_SkAlphaType));
        if(!surface)throw std::runtime_error("Graphite target unavailable");fontMgr=SkFontMgr_New_CoreText(nullptr);
    }
    ~Context() override {surface.reset();finishGraphiteWork(gpu.get(),recorder.get());recorder.reset();gpu.reset();cache->save();}
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
    std::array<NXFloat,4> windowCornerRadii()override{return {62,62,62,62};}
    NXKit::NXSize getSize()override{return {402,874};}float getScaleFactor()override{return density;}
    UIEdgeInsets deviceSafeAreaInsets()override{return {62,0,34,0};}
    UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface()override{return surface;}
    void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return gpu.get();}skgpu::graphite::Recorder* graphiteRecorder()override{return recorder.get();}
};
void tick(const std::shared_ptr<CALayer>& layer,Timer time){const auto children=layer->sublayers();layer->animateAt(time);for(const auto& child:children)tick(child,time);}
}
int main(int argc,char** argv){@autoreleasepool{try {
    const std::string path=argc>1?argv[1]:"/tmp/navigation-metal.csv";
    int frameStep=1,passes=3;std::string snapshots,isolation="none";bool scrolling=false,liveAppearance=false;
    bool all=false,full=false,light=false,interactive=false,large=false,inkBlur=false;float density=3;
    for(int i=2;i<argc;++i){const std::string arg=argv[i];if(arg=="--quick")frameStep=4;else if(arg=="--live-appearance")liveAppearance=true;else if(arg=="--scroll"){scrolling=true;full=true;}else if(arg=="--isolation"&&i+1<argc)isolation=argv[++i];else if(arg=="--passes"&&i+1<argc)passes=std::stoi(argv[++i]);else if(arg=="--snapshots"&&i+1<argc)snapshots=argv[++i];else if(arg=="--ink-blur")inkBlur=true;else if(arg=="--large-titles")large=true;else if(arg=="--catalog")all=true;else if(arg=="--interactive")interactive=true;else if(arg=="--full")full=true;else if(arg=="--light")light=true;else if(arg=="--2x")density=2;else throw std::invalid_argument("Unknown benchmark option");}
    if(isolation!="none"&&isolation!="glass"&&isolation!="navigation"&&isolation!="tabs"&&isolation!="pockets"&&isolation!="effects")throw std::invalid_argument("Invalid isolation");
    if(passes<1||passes>10)throw std::invalid_argument("Invalid pass count");
    auto ctx=std::make_shared<Context>(full,density);ctx->dark=!light;SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    auto window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;
    auto demo=new_shared<GlassScreenViewController>();window->setRootViewController(demo);window->makeKeyAndVisible();window->drawAndLayoutTreeIfNeeded();
    auto nav=demo->selectedNavigationController();auto root=nav->topViewController();auto bar=nav->navigationBar();
    if(isolation=="navigation")DemoPerformance::isolate(bar,true,false);
    else if(isolation=="tabs")DemoPerformance::isolate(demo->tabBar(),true,false);
    else DemoPerformance::isolate(demo->view(),isolation=="glass"||isolation=="effects",isolation=="pockets"||isolation=="effects");
    std::ofstream out(path);out<<"pass,direction,arrangement,frame,cpu_ms,library_compiles,pipeline_compiles,archive_hits,total_ms,gpu_ms,command_buffers,bounded_lenses,bounded_lens_groups,cropped_lens_sources,lens_source_pixels,uniform_mask_prefixes,bounded_masks\n";
    SkBitmap alphaMask;alphaMask.allocPixels(SkImageInfo::MakeA8(128,64));alphaMask.eraseColor(SK_ColorTRANSPARENT);
    for(int y=12;y<52;++y)for(int x=12;x<116;++x)*alphaMask.getAddr8(x,y)=((x/12+y/12)%2)?255:64;
    auto maskImage=SkImages::RasterFromBitmap(alphaMask);float forcedBlur=0;
    uint64_t warmMisses=0,lastMisses=0;
    auto draw=[&](int pass,const char* direction,int arrangement,int frame){
        [ctx->diagnosticQueue.buffers removeAllObjects];
        Internal::RenderPerformance sample;
        struct RestoreMetrics {Internal::RenderPerformance* previous;~RestoreMetrics(){Internal::RenderPerformance::current=previous;}} restoreMetrics{Internal::RenderPerformance::current};
        if(frame==0||frame==120||frame==240)Internal::RenderPerformance::current=&sample;
        auto start=std::chrono::steady_clock::now();window->drawAndLayoutTreeIfNeeded();auto canvas=ctx->surface->getCanvas();canvas->clear(ctx->dark?SK_ColorBLACK:SkColorSetRGB(242,242,247));canvas->save();canvas->scale(density,density);if(!full)canvas->translate(0,12-bar->frame().minY());UITraitCollection::setCurrent(window->traitCollection());(full?window->layer():bar->layer())->presentationOrSelf()->skiaRender(canvas);canvas->restore();
        if(inkBlur&&forcedBlur>0) {
            // Alpha-only intermediates are used by glyph/shape masks. Cover
            // their clamped and unclamped Gaussian paths as well as RGBA ink.
            SkPaint paint;paint.setImageFilter(SkImageFilters::Blur(forcedBlur,forcedBlur,nullptr));
            canvas->save();canvas->scale(density,density);canvas->clipRect(SkRect::MakeXYWH(0,200,300,80));
            canvas->drawImage(maskImage,32,210,SkSamplingOptions(SkFilterMode::kLinear),&paint);canvas->restore();
        }
        ctx->flushAndSubmit(ctx->surface);
        auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();const auto stats=ctx->cache->statistics();
        if(stats.failures)throw std::runtime_error("Required Metal pipeline preparation failed");
        ctx->gpu->submit(skgpu::graphite::SyncToCpu::kYes);
        auto total=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        out<<pass<<','<<direction<<','<<arrangement<<','<<frame<<','<<ms<<','<<stats.libraryCompiles<<','<<stats.pipelineCompiles<<','<<stats.archiveHits<<','<<total;
        double gpuMS=0;for(id<MTLCommandBuffer> buffer in ctx->diagnosticQueue.buffers)
            if(buffer.GPUStartTime>0 && buffer.GPUEndTime>=buffer.GPUStartTime)gpuMS+=(buffer.GPUEndTime-buffer.GPUStartTime)*1000;
        out<<','<<gpuMS<<','<<ctx->diagnosticQueue.buffers.count<<','<<sample.boundedLenses<<','<<sample.boundedLensGroups<<','<<sample.croppedLensSources<<','<<sample.lensSourcePixels<<','<<sample.uniformMaskPrefixes<<','<<sample.boundedMasks<<'\n';out.flush();
        if(!snapshots.empty()&&pass==0&&arrangement>=0&&
            (std::string_view(direction)=="rest" || ((std::string_view(direction)=="push"||std::string_view(direction)=="pop")&&frame==60))) {
            std::unique_ptr<const SkImage::AsyncReadResult> pixels;
            const auto info=SkImageInfo::Make(ctx->surface->width(),ctx->surface->height(),kRGBA_8888_SkColorType,kPremul_SkAlphaType);
            ctx->gpu->asyncRescaleAndReadPixels(ctx->surface.get(),info,SkIRect::MakeWH(info.width(),info.height()),SkImage::RescaleGamma::kSrc,SkImage::RescaleMode::kNearest,
                [](void* context,std::unique_ptr<const SkImage::AsyncReadResult> value){*static_cast<std::unique_ptr<const SkImage::AsyncReadResult>*>(context)=std::move(value);},&pixels);
            ctx->gpu->submit(skgpu::graphite::SyncToCpu::kYes);ctx->gpu->checkAsyncWorkCompletion();
            if(!pixels)throw std::runtime_error("Snapshot readback failed");
            std::ofstream image(snapshots+"-"+direction+"-"+std::to_string(arrangement)+".ppm",std::ios::binary);
            image<<"P6\n"<<info.width()<<' '<<info.height()<<"\n255\n";
            for(int y=0;y<info.height();++y)for(int x=0;x<info.width();++x)
                image.write(static_cast<const char*>(pixels->data(0))+y*pixels->rowBytes(0)+x*4,3);
        }
    };
    draw(-1,"initial",-1,0);
    for(int pass=0;pass<passes;++pass) {
        const auto before=ctx->cache->statistics();
        if(scrolling) {
            if(liveAppearance){UIView::completePendingAnimations();ctx->gpu->checkAsyncWorkCompletion();DispatchQueueTestHarness::pump();}
            const auto list=demo->selectedList();const auto scroll=list->scrollView();
            const auto inset=scroll->adjustedContentInset();
            const auto distance=std::max(0.f,scroll->contentSize().height-scroll->bounds().height()+inset.top+inset.bottom);
            for(int i=0;i<=240;i+=frameStep) {
                const auto t=NXFloat(i)/240;
                list->setScrollPosition(distance*(.5f-.5f*std::cos(t*2*3.141592653589793f)));
                if(liveAppearance){ctx->gpu->checkAsyncWorkCompletion();DispatchQueueTestHarness::pump();}
                draw(pass,"scroll",0,i);
            }
        } else if(inkBlur) {
            UIView::completePendingAnimations();DispatchQueueTestHarness::pump();
            auto& state=Internal::NavigationBarAccess::state(*bar);
            // Exercise kernel boundaries deterministically. Wall-clock-driven
            // morphs can skip a small blur radius between consecutive frames.
            for(int i=1;i<=128;++i) {
                forcedBlur=i/16.f;
                for(const auto* side:{&state.left,&state.right})for(const auto& button:side->buttons) {
                    Internal::navigationInkBlur(button->ink,i/16.f,i/16.f,1);
                    button->ink->layer()->animateAt(Timer());
                }
                draw(pass,"ink-blur",-1,i);
            }
        }else if(large) {
            nav->setViewControllers({root},false);window->drawAndLayoutTreeIfNeeded();
            demo->selectedList()->action("large-title-demo")->performPrimaryAction();
            auto animate=[&](const char* direction,int mode){const auto start=Timer();for(int i=0;i<=120;i+=frameStep){auto now=start;const auto us=start.startTime.tv_usec+i*1000000L/120;now.startTime.tv_sec+=us/1000000;now.startTime.tv_usec=us%1000000;tick(window->layer(),now);DispatchQueueTestHarness::pump();draw(pass,direction,mode,i);}UIView::completePendingAnimations();DispatchQueueTestHarness::pump();};
            animate("large-entry",-1);
            auto source=nav->topViewController();auto scroll=std::dynamic_pointer_cast<UIScrollView>(source->view());
            if(!scroll)throw std::runtime_error("Large-title Demo requires a scroll view");
            const auto top=scroll->adjustedContentInset().top;
            for(int i=0;i<=120;i+=frameStep){scroll->setContentOffset({0,-top+NXFloat(i)},false);draw(pass,"large-scroll",-1,i);}
            scroll->setContentOffset({0,-top},false);
            for(int mode=0;mode<3;++mode) {
                auto control=std::dynamic_pointer_cast<UIControl>(scroll->subviews().at(mode));control->performPrimaryAction();animate("large-push",mode);
                nav->popViewController(true);animate("large-pop",mode);
                for(bool commit:{false,true}) {
                    control->performPrimaryAction();UIView::completePendingAnimations();DispatchQueueTestHarness::pump();draw(pass,"large-interactive-rest",mode,0);
                    UIGestureRecognizerTestHarness input(window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{15,300},60);
                    auto pan=std::static_pointer_cast<UIPanGestureRecognizer>(nav->interactivePopGestureRecognizer());
                    if(pan->state()!=UIGestureRecognizerState::changed)throw std::runtime_error("Large-title interactive gesture failed");
                    for(int i=0;i<=75;++i){pan->setTranslation({NXFloat((commit?1.:.45)*402*i/75),0},nav->view());UIGestureRecognizerTestHarness::notify(pan);draw(pass,"large-interactive",mode,i);}
                    if(!commit)pan->setEnabled(false);input.send(UITouchPhase::ended,{15,300},1500);animate(commit?"large-commit":"large-cancel",mode);pan->setEnabled(true);
                    if(!commit){nav->popViewController(false);window->drawAndLayoutTreeIfNeeded();}
                }
            }
        }
        if(!scrolling&&interactive&&!large&&!inkBlur)for(int arrangement=0;arrangement<8;++arrangement)for(bool commit:{false,true}) {
            nav->setViewControllers({root,new_shared<GlassNavigationDemoViewController>(arrangement),new_shared<GlassNavigationDemoViewController>((arrangement+1)%8)},false);
            draw(pass,"interactive-rest",arrangement,0);
            UIGestureRecognizerTestHarness input(window);input.send(UITouchPhase::moved,{14,300},50);input.send(UITouchPhase::moved,{15,300},60);
            auto pan=std::static_pointer_cast<UIPanGestureRecognizer>(nav->interactivePopGestureRecognizer());
            if(pan->state()!=UIGestureRecognizerState::changed)throw std::runtime_error("Interactive benchmark gesture failed");
            for(int i=0;i<=120;i+=frameStep){const auto progress=(commit?1.:.72)*std::min(1.,i/75.);pan->setTranslation({NXFloat(progress*402),0},nav->view());UIGestureRecognizerTestHarness::notify(pan);draw(pass,commit?"interactive-commit":"interactive-cancel",arrangement,i);}
            if(!commit)pan->setEnabled(false);
            input.send(UITouchPhase::ended,{15,300},1500);const auto start=Timer();
            for(int i=0;i<=120;i+=frameStep){auto now=start;const auto us=start.startTime.tv_usec+i*1000000L/120;now.startTime.tv_sec+=us/1000000;now.startTime.tv_usec=us%1000000;tick(window->layer(),now);DispatchQueueTestHarness::pump();draw(pass,commit?"commit-settle":"cancel-settle",arrangement,i);}
            UIView::completePendingAnimations();DispatchQueueTestHarness::pump();pan->setEnabled(true);
        }
        if(!scrolling&&!inkBlur&&!large&&(all||!interactive))for(int arrangement=all?0:5;arrangement<(all?8:6);++arrangement) {
            auto source=new_shared<GlassNavigationDemoViewController>(all?(arrangement+7)%8:4);
            nav->setViewControllers({root,source},false);draw(pass,"rest",arrangement,0);
            for(bool pop:{false,true}) {
                const auto start=Timer();if(pop)nav->popViewController(true);else nav->pushViewController(new_shared<GlassNavigationDemoViewController>(arrangement),true);
                for(int i=0;i<=120;i+=frameStep){auto time=start;const long us=time.startTime.tv_usec+i*1000000L/120;time.startTime.tv_sec+=us/1000000;time.startTime.tv_usec=us%1000000;tick(window->layer(),time);DispatchQueueTestHarness::pump();draw(pass,pop?"pop":"push",arrangement,i);}
                UIView::completePendingAnimations();DispatchQueueTestHarness::pump();
            }
        }
        const auto after=ctx->cache->statistics();const auto misses=after.pipelineCompiles-before.pipelineCompiles;
        std::cout<<"Pass "<<pass<<": "<<misses<<" pipeline compilations, "<<after.archiveHits-before.archiveHits<<" binary hits\n"<<std::flush;
        if(pass>0)warmMisses+=misses;
        lastMisses=misses;
    }
    const auto finalStats=ctx->cache->statistics();
    window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();UIApplication::shared.reset();SkiaCtx::_main.reset();ctx->cache->save();ctx.reset();
    if(finalStats.failures){std::cerr<<"Metal preparation failed "<<finalStats.failures<<" times\n";return 3;}
    // Catalog mode learns finite blur-kernel boundary variants as well. Its
    // last pass must be warm; strict archive runs forbid compilation outright.
    if(passes>1&&((all||interactive||large)?lastMisses:warmMisses)){std::cerr<<"Repeated transitions compiled "<<((all||interactive||large)?lastMisses:warmMisses)<<" pipelines\n";return 1;}return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}}}
