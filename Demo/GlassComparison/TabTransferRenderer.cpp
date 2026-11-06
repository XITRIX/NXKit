// Actual retained Demo composition driven by a routed, stationary touch.
#include "../app/Screens/GlassDemoViewController/LensTabBarView.hpp"
#include "../../UIKitCore/lib/platforms/apple/AppleSystemFont.hpp"
#include <UIApplication.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <SDL3/SDL.h>
#include "CABackdropEffectLayer.hpp"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include <fstream>
#include <iostream>
using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
class UIGestureRecognizerTestHarness {
public:
    static auto begin(const std::shared_ptr<UIWindow>& window,NXPoint point) {
        auto touch=new_shared<UITouch>(1,point,Timer()); auto event=new_shared<UIEvent>();
        event->_allTouches.insert(touch); touch->_window=window; window->sendEvent(event);
        return std::pair(touch,event);
    }
    static void end(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>& event) {
        touch->_phase=UITouchPhase::ended; window->sendEvent(event);
    }
};
class GlassInteractionTestHarness {
public:
    static void advance(const std::shared_ptr<BackdropEffectView>& view,double dt) { view->_advanceInteraction(dt); }
    static auto motion(const std::shared_ptr<BackdropEffectView>& view) { return *view->_backdropEffectLayer()->_interaction; }
};
class LensViewTestHarness {
public:
    static void advance(const std::shared_ptr<LensView>& view,double dt) { view->advance(dt); }
};
}
class Context final:public SkiaCtx {
public:
    bool dark=false;
    float density=2;
    Context() { fontMgr=SkFontMgr_New_CoreText(nullptr); }
    NXKit::NXSize getSize() override { return {402,300}; }
    float getScaleFactor() override { return density; }
    UIUserInterfaceStyle getThemeMode() override { return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light; }
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17) const override { return Internal::appleSystemTypeface(style,size); }
    sk_sp<SkSurface> getBackbufferSurface() override { return nullptr; }
    void swapBuffers() override {} void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override { return false; }
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};
class LensTabBarTestHarness {
public:
    static void advance(const std::shared_ptr<LensTabBarView>& bar,double dt) {bar->advanceLift(dt);}
};
int main(int argc,char** argv) {
    if(argc!=3) {std::cerr<<"OUTPUT_PREFIX DARK\n";return 2;}
    auto context=std::make_shared<Context>();context->dark=std::stoi(argv[2]);context->density=3;SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();SDL_InitSubSystem(SDL_INIT_EVENTS);
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();controller->view()->setBackgroundColor(UIColor(195,195,195));
    window->setRootViewController(controller);window->makeKeyAndVisible();
    auto bar=new_shared<LensTabBarView>(false,4);bar->setFrame({21,120,360,62});controller->view()->addSubview(bar);
    for(int i=0;i<4;++i)applicationRunLoop();bar->layoutIfNeeded();
    auto save=[&](int milliseconds) {
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,900));auto canvas=surface->getCanvas();canvas->scale(3,3);canvas->clear(SK_ColorBLACK);
        UITraitCollection::setCurrent(window->traitCollection());window->layer()->skiaRender(canvas);
        SkBitmap pixels;pixels.allocN32Pixels(1206,900);surface->readPixels(pixels,0,0);
        std::ofstream out(std::string(argv[1])+"-"+std::to_string(milliseconds)+".ppm",std::ios::binary);out<<"P6\n1206 900\n255\n";
        for(int y=0;y<900;++y)for(int x=0;x<1206;++x){auto c=pixels.getColor(x,y);unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};out.write(reinterpret_cast<char*>(rgb),3);}
    };
    save(0);
    auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,{329.5f,151});
    const Timer origin;
    auto tick=[&](const auto& self,const std::shared_ptr<CALayer>& layer,Timer time)->void {layer->animateAt(time);for(const auto& child:layer->sublayers())self(self,child,time);};
    for(int i=1;i<=240;++i) {
        Timer now=origin;now.startTime.tv_usec+=int(i*1000000.0/120);tick(tick,bar->layer(),now);
        LensTabBarTestHarness::advance(bar,1.0/120);GlassInteractionTestHarness::advance(bar->glass(),1.0/120);LensViewTestHarness::advance(bar->lens(),1.0/120);
        if(i==8)UIGestureRecognizerTestHarness::end(window,touch,event);
        if(i==12||i==24||i==36||i==60||i==96||i==240)save(int(i*1000.0/120));
    }
    bar->removeFromSuperview();window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);
}
