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
int main(int argc,char** argv) {
    if(argc<6||argc>10||(argc>=9&&std::string(argv[8])!="--fixed-optics")||(argc==10&&std::string(argv[9])!="--normal-ink")) { std::cerr<<"OUTPUT_PREFIX DARK R G B [SCALE [SELECTION_POSITION [--fixed-optics [--normal-ink]]]]\n";return 2; }
    auto context=std::make_shared<Context>();context->dark=std::stoi(argv[2]);if(argc>=7)context->density=std::stoi(argv[6]);SkiaCtx::_main=context;
    if(context->density<1||context->density>3)return 2;
    UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();SDL_InitSubSystem(SDL_INIT_EVENTS);
    auto window=new_shared<UIWindow>();auto controller=new_shared<UIViewController>();
    controller->view()->setBackgroundColor(UIColor(std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5])));
    window->setRootViewController(controller);window->makeKeyAndVisible();
    auto bar=new_shared<LensTabBarView>(false,4);bar->setFrame({21,120,360,62});controller->view()->addSubview(bar);bar->setSelectedIndex(2);
    if(argc==10)bar->lens()->setContentVibrancyEnabled(false);
    for(int i=0;i<4;++i)applicationRunLoop();bar->layoutIfNeeded();
    if(argc>=8)bar->setSelectionPosition(std::stof(argv[7]));
    const auto stagePoint=[&](NXKit::NXPoint point,std::shared_ptr<UIView> view) {
        while(view&&view!=controller->view()) {
            const auto layer=view->layer()->presentationOrSelf();
            if(view==bar->glass())point=point.applying(bar->glass()->interactionTransform());
            point-=NXKit::NXPoint(layer->bounds().width()*layer->anchorPoint().x,layer->bounds().height()*layer->anchorPoint().y);
            point=point.applying(layer->affineTransform())+layer->position();
            view=view->superview().lock();if(view)point-=view->bounds().origin;
        }
        return point;
    };
    auto save=[&](const char* phase) {
        const int scale=int(context->density),width=402*scale,height=300*scale;
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width,height));auto canvas=surface->getCanvas();canvas->scale(scale,scale);canvas->clear(SK_ColorBLACK);
        UITraitCollection::setCurrent(window->traitCollection());window->layer()->skiaRender(canvas);
        SkBitmap pixels;pixels.allocN32Pixels(width,height);surface->readPixels(pixels,0,0);
        const auto name=std::string(argv[1])+"-"+phase;
        std::ofstream out(name+".ppm",std::ios::binary);out<<"P6\n"<<width<<' '<<height<<"\n255\n";
        for(int y=0;y<height;++y)for(int x=0;x<width;++x){auto c=pixels.getColor(x,y);unsigned char rgb[]={static_cast<unsigned char>(SkColorGetR(c)),static_cast<unsigned char>(SkColorGetG(c)),static_cast<unsigned char>(SkColorGetB(c))};out.write(reinterpret_cast<char*>(rgb),3);}
        const auto lens=bar->lens()->frame(); const auto motion=GlassInteractionTestHarness::motion(bar->glass()); const auto affine=motion.transform(bar->bounds().size);
        std::ofstream meta(name+".json");meta<<"{\"lens\":["<<21+lens.minX()<<','<<120+lens.minY()<<','<<lens.width()<<','<<lens.height()<<"],\"platter\":[21,120,360,62],\"platterScale\":["<<affine.m11<<','<<affine.m22<<"],\"glow\":"<<motion.glow()<<",\"symbols\":[";
        bool first=true;for(const auto& control:bar->controls()) for(const auto& child:control->subviews().front()->subviews()) if(auto image=std::dynamic_pointer_cast<UIImageView>(child);image&&image->image()) {
            const auto point=stagePoint({},image);if(!first)meta<<',';first=false;meta<<'['<<point.x<<','<<point.y<<','<<image->bounds().width()<<','<<image->bounds().height()<<']';
        }
        const auto writeCaption=[&](const std::shared_ptr<UILabel>& label,bool& first) {
            if(!first)meta<<',';first=false;
            const auto point=stagePoint({},label);
            const auto end=stagePoint({label->bounds().width(),label->bounds().height()},label);
            const auto fits=label->sizeThatFits({1000,1000});
            meta<<"{\"frame\":["<<point.x<<','<<point.y<<','<<end.x-point.x<<','<<end.y-point.y
                <<"],\"points\":"<<label->fontSize()<<",\"weight\":"<<label->fontWeight()
                <<",\"fits\":["<<fits.width<<','<<fits.height<<"]}";
        };
        meta<<"],\"captions\":[";first=true;
        for(const auto& control:bar->controls()) for(const auto& child:control->subviews())
            if(auto label=std::dynamic_pointer_cast<UILabel>(child))writeCaption(label,first);
        meta<<"],\"selectedCaptions\":[";first=true;
        auto captions=[&](const auto& self,const std::shared_ptr<UIView>& view)->void {
            if(auto label=std::dynamic_pointer_cast<UILabel>(view))writeCaption(label,first);
            for(const auto& child:view->subviews())self(self,child);
        };
        captions(captions,bar->lens()->liftedContentView());
        meta<<"],\"selectedSymbols\":[";first=true;
        auto symbols=[&](const auto& self,const std::shared_ptr<UIView>& view)->void {
            if(auto image=std::dynamic_pointer_cast<UIImageView>(view);image&&image->image()) {
                const auto origin=stagePoint({},image);
                const auto end=stagePoint({image->bounds().width(),image->bounds().height()},image);
                if(!first)meta<<',';first=false;meta<<'['<<origin.x<<','<<origin.y<<','<<end.x-origin.x<<','<<end.y-origin.y<<']';
            }
            for(const auto& child:view->subviews())self(self,child);
        };
        symbols(symbols,bar->lens()->liftedContentView());
        meta<<"],\"sourceScales\":[";first=true;
        for(const auto& item:bar->lens()->liftedContentView()->subviews()) {
            if(!first)meta<<',';first=false;meta<<item->transform().m11;
        }
        meta<<"]}\n";
    };
    save("rest");
    auto [touch,event]=UIGestureRecognizerTestHarness::begin(window,{argc>=8?21+bar->lens()->frame().midX():243.612f,151});
    auto tick=[&](const auto& self,const std::shared_ptr<CALayer>& layer,Timer time)->void { layer->animateAt(time);for(const auto& child:layer->sublayers())self(self,child,time); };
    for(int i=1;i<=120;++i){tick(tick,bar->layer(),Timer(i*1000.0/120));GlassInteractionTestHarness::advance(bar->glass(),1.0/120);LensViewTestHarness::advance(bar->lens(),1.0/120);}
    if(argc>=9) {
        // Optical comparison isolates sampling from wall-clock animation phase.
        // Keep the already settled platter feedback, fix the requested lens
        // position and remove only presentation/motion history in this fixture.
        bar->setSelectionPosition(std::stof(argv[7]));
        bar->lens()->setInteractive(false);bar->lens()->setLifted(true,false);
        auto clear=[](const auto& self,const std::shared_ptr<CALayer>& layer)->void {
            layer->removeAllAnimations();for(const auto& child:layer->sublayers())self(self,child);
        };
        clear(clear,bar->layer());
    }
    save("held");UIGestureRecognizerTestHarness::end(window,touch,event);
    for(int i=0;i<240;++i){tick(tick,bar->layer(),Timer(5000));GlassInteractionTestHarness::advance(bar->glass(),1.0/120);LensViewTestHarness::advance(bar->lens(),1.0/120);}
    bar->removeFromSuperview();
    window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);
}
