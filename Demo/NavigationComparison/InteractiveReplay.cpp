// Replay native content-progress samples through the real portable gesture and
// containers. Progress matching isolates bar morphs from platform input latency.
#include <UIKit.h>
#include <UIApplication.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <UIPanGestureRecognizer.h>
#include <SkiaCtx.h>
#include "UINavigationBarInternal.hpp"
#include "../app/Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include "platforms/apple/AppleSystemFont.hpp"
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/encode/SkPngEncoder.h"
#include "include/core/SkStream.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <unordered_set>
using namespace NXKit;
namespace NXKit {
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class DispatchQueueTestHarness {public:static void pump(){DispatchQueue::main()->performAll();}};
class UIGestureRecognizerTestHarness {
public:
    static void notify(const std::shared_ptr<UIGestureRecognizer>& gesture){gesture->notifyStateChanged();}
    std::shared_ptr<UIWindow> window;std::shared_ptr<UITouch> touch;std::shared_ptr<UIEvent> event;Timer origin;
    explicit UIGestureRecognizerTestHarness(const std::shared_ptr<UIWindow>& w):window(w) {
        touch=new_shared<UITouch>(811,NXPoint{2,300},origin);event=new_shared<UIEvent>();event->_allTouches.insert(touch);touch->_window=window;window->sendEvent(event);
    }
    void send(UITouchPhase phase,NXPoint p,double ms) {
        touch->updateAbsoluteLocation(p);touch->_timestamp=origin;const auto us=origin.startTime.tv_usec+long(ms*1000);touch->_timestamp.startTime.tv_sec+=us/1000000;touch->_timestamp.startTime.tv_usec=us%1000000;touch->_phase=phase;window->sendEvent(event);
    }
};
}
namespace {
class Context:public SkiaCtx {
public:
    bool dark=false;Context(){fontMgr=SkFontMgr_New_CoreText(nullptr);}
    sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
    NXKit::NXSize getSize()override{return {402,874};}float getScaleFactor()override{return 3;}
    UIEdgeInsets deviceSafeAreaInsets()override{return {62,0,34,0};}
    UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
    sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
void tick(const std::shared_ptr<CALayer>& layer,Timer now){const auto children=layer->sublayers();layer->animateAt(now);for(const auto& child:children)tick(child,now);}
Timer offset(Timer origin,double seconds){const auto us=origin.startTime.tv_usec+long(seconds*1e6);origin.startTime.tv_sec+=us/1000000;origin.startTime.tv_usec=us%1000000;return origin;}
void settle(){for(int i=0;i<6;++i){UIView::completePendingAnimations();DispatchQueueTestHarness::pump();}}
NXRect visibleFrame(const std::shared_ptr<UIView>& view,const std::shared_ptr<UIView>& root) {
    const auto p=view->layer()->presentationOrSelf();auto result=NXRect{-p->bounds().width()*.5f,-p->bounds().height()*.5f,p->bounds().width(),p->bounds().height()}.applying(p->transform());result.origin+=p->position();
    for(auto parent=view->superview().lock();parent&&parent!=root;parent=parent->superview().lock()){const auto layer=parent->layer()->presentationOrSelf();result.origin-=NXKit::NXPoint{layer->bounds().width()*.5f,layer->bounds().height()*.5f};result=result.applying(layer->transform());result.origin+=layer->position();}return result;
}
}
int main(int argc,char**argv){if(argc<3||argc>4)return 2;const bool geometryOnly=argc==4&&std::string(argv[3])=="--geometry-only";const std::filesystem::path directory=argv[2];std::filesystem::create_directories(directory);
    auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    std::ifstream schedule(argv[1]);std::ofstream report(directory/"replay.json");report<<"[";std::string line,current;bool comma=false,released=false;double releaseTime=0;Timer releaseOrigin;
    std::shared_ptr<UIWindow> window;std::shared_ptr<GlassScreenViewController> demo;std::shared_ptr<UINavigationController> nav;std::shared_ptr<UIPanGestureRecognizer> pan;std::unique_ptr<UIGestureRecognizerTestHarness> input;
    while(std::getline(schedule,line)) {
        std::replace(line.begin(),line.end(),',',' ');std::istringstream row(line);std::string name;int source,target,dark,index,active,picture;double time,progress;row>>name>>source>>target>>dark>>index>>time>>progress>>active>>picture;if(!row)return 3;
        if(name!=current) {
            settle();if(window)window->setRootViewController(nullptr);current=name;ctx->dark=dark;released=false;
            demo=new_shared<GlassScreenViewController>();window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;window->setRootViewController(demo);window->makeKeyAndVisible();nav=demo->selectedNavigationController();
            nav->setViewControllers({nav->topViewController(),new_shared<GlassNavigationDemoViewController>(target),new_shared<GlassNavigationDemoViewController>(source)},false);window->drawAndLayoutTreeIfNeeded();settle();
            input=std::make_unique<UIGestureRecognizerTestHarness>(window);input->send(UITouchPhase::moved,{14,300},50);input->send(UITouchPhase::moved,{15,300},60);pan=std::static_pointer_cast<UIPanGestureRecognizer>(nav->interactivePopGestureRecognizer());
            if(pan->state()!=UIGestureRecognizerState::changed){std::cerr<<"Gesture failed: "<<name<<'\n';return 4;}
            std::filesystem::create_directories(directory/name);
        }
        if(active&&!released){pan->setTranslation({NXFloat(std::clamp(progress,0.,1.)*402),0},nav->view());UIGestureRecognizerTestHarness::notify(pan);}
        else if(!released){releaseTime=time;input->send(UITouchPhase::ended,{15,300},time*1000+500);releaseOrigin=Timer();released=true;}
        if(released)tick(window->layer(),offset(releaseOrigin,time-releaseTime));
        window->drawAndLayoutTreeIfNeeded();DispatchQueueTestHarness::pump();auto bar=nav->navigationBar();
        if(comma)report<<',';comma=true;report<<"{\"name\":"<<std::quoted(name)<<",\"index\":"<<index<<",\"t\":"<<time<<",\"progress\":"<<progress<<",\"active\":"<<active<<",\"stack\":"<<nav->viewControllers().size()<<",\"groups\":[";
        std::unordered_set<const UIView*> carriers;for(const auto* side:{&Internal::NavigationBarAccess::state(*bar).left,&Internal::NavigationBarAccess::state(*bar).right})for(const auto& group:side->groups)carriers.insert(group.view.get());
        bool delimiter=false;std::function<void(const std::shared_ptr<UIView>&)> visit=[&](const auto& view){if(view->isHidden()||view->layer()->presentationOrSelf()->opacity()<.001)return;if(std::dynamic_pointer_cast<BackdropEffectView>(view)||carriers.contains(view.get())) {const auto f=visibleFrame(view,bar);if(delimiter)report<<',';delimiter=true;report<<'['<<f.minX()<<','<<f.minY()<<','<<f.width()<<','<<f.height()<<']';}for(const auto& child:view->subviews())visit(child);};visit(bar);report<<"]}";
        if(picture&&!geometryOnly) {
            auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(804,160));const auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SkColorSetRGB(242,242,247));canvas->scale(2,2);canvas->translate(0,12-bar->frame().minY());UITraitCollection::setCurrent(window->traitCollection());window->layer()->presentationOrSelf()->skiaRender(canvas);
            SkBitmap bitmap;bitmap.allocN32Pixels(804,160);surface->readPixels(bitmap,0,0);char file[40];snprintf(file,sizeof(file),"%04d.png",index);SkFILEWStream stream((directory/name/file).c_str());if(!SkPngEncoder::Encode(&stream,bitmap.pixmap(),{}))return 5;
        }
    }
    settle();report<<"]\n";if(window)window->setRootViewController(nullptr);input.reset();demo.reset();nav.reset();window.reset();UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();UIApplication::shared.reset();SkiaCtx::_main.reset();return 0;
}
