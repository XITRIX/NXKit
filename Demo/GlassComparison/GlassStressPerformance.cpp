// Full, unclipped application frames and SDL input replay. CPU/raster timings
// deliberately include optical shading; they are not device GPU timings.
#include "../app/Screens/GlassDemoViewController/GlassStressView.hpp"
#include "../../UIKitCore/lib/RenderPerformance.hpp"
#include "../../UIKitCore/lib/platforms/apple/AppleSystemFont.hpp"
#include <UIApplication.h>
#include <CADisplayLink.h>
#include <SDL3/SDL.h>
#include "include/core/SkBitmap.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*, std::map<std::string,std::any>) { return true; }
}
namespace {
class Context final : public SkiaCtx {
public:
    NXKit::NXSize size{320,640};
    float density = 1;
    bool dark = false;
    int presents = 0;
    sk_sp<SkSurface> surface;
    Context() { fontMgr = SkFontMgr_New_CoreText(nullptr); }
    NXKit::NXSize getSize() override { return size; }
    float getScaleFactor() override { return density; }
    UIEdgeInsets deviceSafeAreaInsets() override { return {24,0,16,0}; }
    UIUserInterfaceStyle getThemeMode() override { return dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light; }
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat points=17) const override { return Internal::appleSystemTypeface(style,points); }
    sk_sp<SkSurface> getBackbufferSurface() override { return surface; }
    void swapBuffers() override { ++presents; }
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
void touch(Uint32 type, NXKit::NXPoint point, NXKit::NXSize size) {
    SDL_Event event{}; event.type=type; event.tfinger.fingerID=725;
    event.tfinger.x=point.x/size.width; event.tfinger.y=point.y/size.height;
    if (!SDL_PushEvent(&event)) throw std::runtime_error(SDL_GetError());
}
void ppm(const std::string& path, SkSurface* surface) {
    SkPixmap pixels;
    if (!surface->peekPixels(&pixels)) return;
    std::ofstream file(path,std::ios::binary);
    file << "P6\n" << pixels.width() << ' ' << pixels.height() << "\n255\n";
    for (int y=0;y<pixels.height();++y) for (int x=0;x<pixels.width();++x) {
        const auto c=pixels.getColor(x,y);
        const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};
        file.write(reinterpret_cast<const char*>(rgb),3);
    }
}
}
int main(int argc, char** argv) {
    // This executable measures/replays every frame, including unchanged ones.
    setenv("NXKIT_CONTINUOUS_RENDERING", "1", 1);
    const std::string prefix = argc>1 ? argv[1] : "/tmp/glass-stress";
    bool recording = false, snapshots = false;
    auto context=std::make_shared<Context>();
    for (int i=2;i<argc;++i) {
        const std::string arg=argv[i];
        if (arg=="--record-only") recording=true;
        else if (arg=="--snapshots-only") snapshots=true;
        else if (arg=="--dark") context->dark=true;
        else if (arg=="--2x") context->density=2;
        else { std::cerr << "Unknown option: " << arg << '\n'; return 2; }
    }
    const auto info=SkImageInfo::MakeN32Premul(int(context->size.width*context->density),int(context->size.height*context->density));
    context->surface=recording ? SkSurfaces::Null(info.width(),info.height()) : SkSurfaces::Raster(info);
    if (!context->surface || !SDL_InitSubSystem(SDL_INIT_EVENTS)) return 2;
    SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();
    UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    auto window=new_shared<UIWindow>(); auto controller=new_shared<UIViewController>();
    auto screen=new_shared<GlassStressView>(); controller->setView(screen);
    window->setRootViewController(controller); window->makeKeyAndVisible();
    CADisplayLink active([] { CALayer::requestFramerate(60); });
    std::ofstream report(prefix+".jsonl");
    if (!report) { std::cerr << "Cannot open report: " << prefix << ".jsonl\n"; return 2; }
    int frames=0, failures=0;
    auto frame=[&](const std::string& phase) {
        // SDL drops motion samples separated by <=5 ms. Replay respects the
        // production clock; this input delay is outside measured frame time.
        SDL_Delay(10);
        Internal::RenderPerformance stats;
        Internal::RenderPerformance::current=&stats;
        const auto start=std::chrono::steady_clock::now();
        const bool running=applicationRunLoop();
        const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        Internal::RenderPerformance::current=nullptr;
        if (!running) ++failures;
        report << "{\"frame\":" << frames++ << ",\"phase\":\"" << phase
               << "\",\"backend\":\"" << (recording?"null-recording":"cpu-raster")
               << "\",\"density\":" << context->density << ",\"dark\":" << (context->dark?"true":"false")
               << ",\"frameMs\":" << ms << ",\"layoutMs\":" << stats.layoutMs
               << ",\"renderMs\":" << stats.renderMs << ",\"glassMs\":" << stats.glassMs
               << ",\"sortMs\":" << stats.sortMs << ",\"submitMs\":" << stats.submitMs
               << ",\"views\":" << stats.views << ",\"layers\":" << stats.layers
               << ",\"rejected\":" << stats.rejected << ",\"sorts\":" << stats.sorts
               << ",\"glass\":" << stats.glass << "}\n"; report.flush();
        std::cout << frames << ' ' << phase << ' ' << ms << " ms / " << stats.glass << " glass passes" << std::endl;
    };
    auto drag=[&](const std::string& phase, NXKit::NXPoint from, NXKit::NXPoint to) {
        touch(SDL_EVENT_FINGER_DOWN,from,context->size); frame(phase+"-begin");
        for (int i=1;i<=3;++i) {
            touch(SDL_EVENT_FINGER_MOTION,from+(to-from)*(NXFloat(i)/3),context->size); frame(phase+"-move");
        }
        touch(SDL_EVENT_FINGER_UP,to,context->size); frame(phase+"-end");
    };
    frame("cold"); frame("idle");
    ppm(prefix+"-initial.ppm",context->surface.get());
    if (!snapshots) {
    const auto initialY=screen->verticalScroll()->contentOffset().y;
    drag("vertical",{280,500},{280,250});
    if (screen->verticalScroll()->contentOffset().y<=initialY) { std::cerr<<"Vertical drag did not scroll\n"; ++failures; }
    for (const auto& rail : screen->horizontalScrolls())
        if (rail->contentOffset().x!=0) { std::cerr<<"Vertical drag moved a horizontal rail\n"; ++failures; }
    for (int row=0;row<GlassStressView::railCount;++row) {
        screen->resetOffsets();
        const auto feed = screen->verticalScroll();
        const auto maximumY = std::max(0.0f,feed->contentSize().height-feed->bounds().height());
        const auto feedY = std::min(NXFloat(row*248),maximumY);
        feed->setContentOffset({0,feedY},false);
        const auto contactY = 264+row*248-feedY;
        frame("rail-"+std::to_string(row)+"-position");
        drag("horizontal-"+std::to_string(row),{260,contactY},{70,contactY});
        if (screen->horizontalScrolls()[row]->contentOffset().x<=0) { std::cerr<<"Horizontal drag did not scroll rail "<<row<<'\n'; ++failures; }
        if (std::abs(screen->verticalScroll()->contentOffset().y-feedY)>0.5f) { std::cerr<<"Horizontal drag moved the vertical feed\n"; ++failures; }
    }
    screen->resetOffsets(); frame("reset");
    touch(SDL_EVENT_FINGER_DOWN,{90,264},context->size); frame("card-press");
    touch(SDL_EVENT_FINGER_UP,{90,264},context->size); frame("card-release");
    if (screen->activations()!=1) { std::cerr<<"Card action was not delivered exactly once\n"; ++failures; }
    touch(SDL_EVENT_FINGER_DOWN,{85,104},context->size); frame("toolbar-press");
    touch(SDL_EVENT_FINGER_UP,{85,104},context->size); frame("toolbar-release");
    if (screen->backdropChanges()!=1) { std::cerr<<"Toolbar action was not delivered exactly once\n"; ++failures; }
    }
    // Drive all rails while the feed moves and the toolbar samples live content.
    for (int i=0;i<4;++i) {
        screen->verticalScroll()->setContentOffset({0,NXFloat(160+i*104)},false);
        for (int row=0;row<GlassStressView::railCount;++row)
            screen->horizontalScrolls()[row]->setContentOffset({NXFloat(70+i*58+row*11),0},false);
        frame("simultaneous");
    }
    ppm(prefix+"-scrolled.ppm",context->surface.get());
    if (context->presents!=frames) { std::cerr<<"A frame was skipped\n"; ++failures; }
    std::cout<<frames<<" presented frames, "<<screen->activations()<<" actions, "<<failures<<" failures\n";
    active.invalidate(); window->setRootViewController(nullptr);
    UIApplication::shared.reset(); SkiaCtx::_main.reset(); SDL_QuitSubSystem(SDL_INIT_EVENTS);
    return failures?1:0;
}
