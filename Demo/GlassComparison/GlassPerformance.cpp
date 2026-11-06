// Repeatable CPU/raster measurements, not GPU or physical thermal estimates.
#include "../app/Screens/GlassDemoViewController/GlassScreenView.hpp"
#include "../app/Screens/GlassDemoViewController/GlassDemoScene.hpp"
#include "../app/Screens/GlassDemoViewController/LensDemoStage.hpp"
#include "../../UIKitCore/lib/platforms/apple/AppleSystemFont.hpp"
#include <UIApplication.h>
#include <CADisplayLink.h>
#include <SDL3/SDL.h>
#include "include/core/SkSurface.h"
#include "include/core/SkPixmap.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
}
namespace {
class Context final:public SkiaCtx {
public:
    NXKit::NXSize size{402,874}; float density=2; bool dark=false;
    int acquisitions=0,presents=0;
    sk_sp<SkSurface> surface;
    Context() {fontMgr=SkFontMgr_New_CoreText(nullptr);}
    void resetSurface() {surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(int(size.width*density),int(size.height*density)));}
    NXKit::NXSize getSize() override {return size;}
    float getScaleFactor() override {return density;}
    UIEdgeInsets deviceSafeAreaInsets() override {return {62,0,34,0};}
    UIUserInterfaceStyle getThemeMode() override {return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat points=17) const override {return Internal::appleSystemTypeface(style,points);}
    sk_sp<SkSurface> getBackbufferSurface() override {++acquisitions;return surface;}
    void swapBuffers() override {++presents;}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override {return false;}
protected:
    skgpu::graphite::Context* graphiteContext() override {return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder() override {return nullptr;}
};
double milliseconds(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
}
void ppm(const std::string& path,SkSurface* surface) {
    SkPixmap pixels;if(!surface->peekPixels(&pixels))return;
    std::ofstream file(path,std::ios::binary);file<<"P6\n"<<pixels.width()<<' '<<pixels.height()<<"\n255\n";
    for(int y=0;y<pixels.height();++y)for(int x=0;x<pixels.width();++x) {
        const auto c=pixels.getColor(x,y);const unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};
        file.write(reinterpret_cast<const char*>(rgb),3);
    }
}
}
int main(int argc,char**argv) {
    const std::string prefix=argc>1?argv[1]:"/tmp/glass-performance";
    const bool quick=argc>2&&std::string(argv[2])=="--quick";
    auto context=std::make_shared<Context>();SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();SDL_InitSubSystem(SDL_INIT_EVENTS);
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();window->setRootViewController(controller);window->makeKeyAndVisible();
    std::ofstream report(prefix+".json");report<<"[\n";bool first=true;
    auto measure=[&](const std::string& name,const std::shared_ptr<UIView>& stage) {
        context->resetSurface();controller->view()->setBackgroundColor(context->dark?UIColor::black:UIColor::white);
        controller->view()->addSubview(stage);stage->setFrame({NXKit::NXPoint(),context->size});
        std::cout<<"Preparing "<<name<<std::endl;
        // Measure paced-loop CPU preparation without running raster shaders.
        // Rendering is now continuous, so all 60 ticks should present.
        context->surface->getCanvas()->save();context->surface->getCanvas()->clipRect(SkRect::MakeEmpty());
        for(int i=0;i<4;++i)applicationRunLoop();
        const auto idleStart=std::chrono::steady_clock::now();const int before=context->presents;
        for(int i=0;i<60;++i)applicationRunLoop();
        const auto idleMs=milliseconds(idleStart);const int idleFrames=context->presents-before;
        context->surface->getCanvas()->restore();
        std::vector<double> samples;
        for(int i=0;i<(quick?1:5);++i) {
            auto* canvas=context->surface->getCanvas();canvas->clear(SK_ColorBLACK);canvas->save();canvas->scale(context->density,context->density);
            UITraitCollection::setCurrent(window->traitCollection());
            auto start=std::chrono::steady_clock::now();window->layer()->presentationOrSelf()->skiaRender(canvas);samples.push_back(milliseconds(start));canvas->restore();
        }
        std::sort(samples.begin(),samples.end());ppm(prefix+"-"+name+".ppm",context->surface.get());
        if(!first)report<<",\n";first=false;
        report<<"{\"name\":\""<<name<<"\",\"density\":"<<context->density<<",\"clipped60TicksMs\":"<<idleMs<<",\"clippedPresents\":"<<idleFrames<<",\"rasterMedianMs\":"<<samples[samples.size()/2]<<"}";report.flush();
        std::cout<<name<<" clipped="<<idleFrames<<"/60 frames, raster="<<samples[samples.size()/2]<<"ms\n"<<std::flush;
        stage->removeFromSuperview();
    };
    for(bool dark:{false,true}) {
        context->dark=dark;context->density=2;context->size={402,874};
        const std::string theme=dark?"dark":"light";
        auto screen=new_shared<GlassScreenView>();
        measure("screen-"+theme,screen);
        screen->cycleBackdrop();measure("screen-blue-"+theme,screen);
        // Navigation/tab material benchmarks live in their component fixtures;
        // this stage now measures only the retained list content.
    }
    if(!quick) {
        context->density=2;context->size={720,440};context->dark=false;
        for(int id=0;id<GlassDemoCount;++id) {
            auto scene=new_shared<GlassDemoScene>(GlassDemoID(id),nullptr);scene->setBackdrop(1);scene->setProgress(.6f);
            measure("gallery-"+std::to_string(id),scene);
        }
        for(bool lifted:{false,true}) {
            auto stage=new_shared<LensDemoStage>();stage->setBackdrop(1);stage->setLifted(lifted);
            measure(lifted?"lenses-held":"lenses-rest",stage);
        }
    }
    report<<"\n]\n";
    window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);
}
