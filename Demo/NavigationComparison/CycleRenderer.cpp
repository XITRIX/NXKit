#include <UIKit.h>
#include <UIApplication.h>
#include <SkiaCtx.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include "UINavigationBarInternal.hpp"
#include "NavigationBarMotion.hpp"
#include "../app/Screens/GlassDemoViewController/GlassScreenViewController.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "platforms/apple/AppleSystemFont.hpp"
#include "include/core/SkSurface.h"
#include "include/core/SkBitmap.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/encode/SkPngEncoder.h"
#include "include/core/SkStream.h"
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace NXKit;
namespace NXKit {
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>){return true;}
class DispatchQueueTestHarness {public:static void pump(){DispatchQueue::main()->performAll();}};
class UIGestureRecognizerTestHarness {public:static void clearFocus(const std::shared_ptr<UIWindow>& window){auto t=new_shared<UITouch>(9988,NXPoint{200,400},Timer());auto e=new_shared<UIEvent>();e->_allTouches.insert(t);t->_window=window;t->_phase=UITouchPhase::began;window->sendEvent(e);t->_phase=UITouchPhase::ended;window->sendEvent(e);}};
}

namespace {
class Context final:public SkiaCtx {
public:
 bool dark=false;Context(){fontMgr=SkFontMgr_New_CoreText(nullptr);}
 sk_sp<SkTypeface> getDefaultTypeface(const SkFontStyle& style=SkFontStyle(),NXFloat size=17)const override{return Internal::appleSystemTypeface(style,size);}
 NXKit::NXSize getSize()override{return {402,874};}float getScaleFactor()override{return 3;}
 UIEdgeInsets deviceSafeAreaInsets()override{return {62,0,34,0};}
 UIUserInterfaceStyle getThemeMode()override{return dark?UIUserInterfaceStyle::dark:UIUserInterfaceStyle::light;}
 sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
void animate(const std::shared_ptr<CALayer>& layer,Timer time){const auto children=layer->sublayers();layer->animateAt(time);for(const auto& child:children)animate(child,time);}
void rect(std::ostream& out,NXRect r){out<<'['<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height()<<']';}
NXRect frame(const std::shared_ptr<UIView>& view,const std::shared_ptr<UIView>& root){
 const auto bounds=view->layer()->presentationOrSelf()->bounds();NXKit::NXPoint points[]={{bounds.minX(),bounds.minY()},{bounds.maxX(),bounds.minY()},{bounds.minX(),bounds.maxY()},{bounds.maxX(),bounds.maxY()}};
 for(auto current=view;current&&current!=root;current=current->superview().lock()){const auto p=current->layer()->presentationOrSelf();for(auto& point:points)point=(point-p->bounds().origin-NXKit::NXPoint{p->bounds().width()*p->anchorPoint().x,p->bounds().height()*p->anchorPoint().y}).applying(NXTransform3DGetAffineTransform(p->transform()))+p->position();}
 NXFloat x=points[0].x,y=points[0].y,right=x,bottom=y;for(auto p:points){x=std::min(x,p.x);y=std::min(y,p.y);right=std::max(right,p.x);bottom=std::max(bottom,p.y);}return{x,y,right-x,bottom-y};
}
}

int main(int argc,char**argv) {
 if(argc<2)return 2;const std::filesystem::path output=argv[1];std::filesystem::create_directories(output);
 const std::string prefix=argc>2?argv[2]:"";const int shard=argc>3?std::stoi(argv[3]):0,shards=argc>4?std::stoi(argv[4]):1;
 const bool denseSequence=argc>5&&std::string(argv[5])=="--dense-sequence";
 const bool everyFrame=argc>5&&std::string(argv[5])=="--every-frame",sequence=argc>6&&std::string(argv[6])=="--sequence";
 auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
 auto quote=[](std::ostream& out,const std::string& value){out<<std::quoted(value);};
 auto identifier=[](const auto& view){return std::to_string(reinterpret_cast<std::uintptr_t>(view.get()));};
 std::ofstream report(output/"cycle.json");report<<"{\"scale\":3,\"imageBarOriginY\":12,\"clips\":[";bool comma=false;int index=0;
 for(bool dark:{false,true})for(bool pop:{false,true})for(int from=-1;from<8;++from)for(int to=-1;to<8;++to) {
  if((pop&&from<0)||(!pop&&to<0))continue;
  const std::string name=std::string(pop?"pop-":"push-")+std::to_string(from)+"-"+std::to_string(to)+(dark?"-dark":"-light");
  const int ordinal=index++;const int shardKey=denseSequence?(ordinal^(ordinal>>2)):ordinal;if(shardKey%shards!=shard||!name.starts_with(prefix)||(sequence&&to!=from+(pop?-1:1)))continue;
  if(argc>7&&std::string(argv[6])=="--target"&&to!=std::stoi(argv[7]))continue;
  const bool captureAll=everyFrame||(denseSequence&&to==from+(pop?-1:1));
  ctx->dark=dark;auto demo=new_shared<GlassScreenViewController>();auto window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;
  window->setRootViewController(demo);window->makeKeyAndVisible();window->drawAndLayoutTreeIfNeeded();UIGestureRecognizerTestHarness::clearFocus(window);
  auto nav=demo->selectedNavigationController();auto root=nav->topViewController();
  const std::shared_ptr<UIViewController> source=from<0?root:new_shared<GlassNavigationDemoViewController>(from),target=to<0?root:new_shared<GlassNavigationDemoViewController>(to);
  std::vector<std::shared_ptr<UIViewController>> stack{root};if(pop){if(target!=root)stack.push_back(target);stack.push_back(source);}else if(source!=root)stack.push_back(source);
  nav->setViewControllers(stack,false);window->drawAndLayoutTreeIfNeeded();UIView::completePendingAnimations();DispatchQueueTestHarness::pump();
  auto bar=nav->navigationBar();auto& state=Internal::NavigationBarAccess::state(*bar);
  std::vector<std::shared_ptr<Internal::NavigationButton>> buttons;auto collect=[&]{for(const auto* side:{&state.left,&state.right})for(const auto& entry:side->buttons)if(std::find(buttons.begin(),buttons.end(),entry)==buttons.end())buttons.push_back(entry);};collect();
  std::filesystem::create_directories(output/name);if(comma)report<<',';comma=true;report<<"{\"name\":";quote(report,name);report<<",\"source\":"<<from<<",\"target\":"<<to<<",\"pop\":"<<(pop?"true":"false")<<",\"dark\":"<<(dark?"true":"false")<<",\"frames\":[";
  auto origin=Timer();int checkpoint=0;const int pictures[]={0,6,12,18,24,30,36,48,72,90};
  auto opacity=[&](std::shared_ptr<UIView> view){NXFloat alpha=1;for(;view;view=view->superview().lock()){if(view->isHidden())return 0.f;alpha*=view->layer()->presentationOrSelf()->opacity();if(view==bar)break;}return alpha;};
  for(int i=0;i<=90;++i) {
   if(i==1){origin=Timer();if(pop)nav->popViewController(true);else nav->pushViewController(target,true);window->drawAndLayoutTreeIfNeeded();collect();}
   auto time=origin;const long us=time.startTime.tv_usec+long(i*1000000/120);time.startTime.tv_sec+=us/1000000;time.startTime.tv_usec=us%1000000;animate(window->layer(),time);DispatchQueueTestHarness::pump();window->drawAndLayoutTreeIfNeeded();
   if(i)report<<',';report<<"{\"t\":"<<i/120.;
   if(captureAll||(checkpoint<10&&i==pictures[checkpoint])) {
    auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,240));auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SkColorSetRGB(242,242,247));canvas->scale(3,3);canvas->translate(0,12-bar->frame().minY());UITraitCollection::setCurrent(window->traitCollection());window->layer()->presentationOrSelf()->skiaRender(canvas);
    SkBitmap bitmap;bitmap.allocN32Pixels(1206,240);surface->readPixels(bitmap,0,0);char file[40];snprintf(file,sizeof(file),"frame-%03d.png",checkpoint++);SkFILEWStream stream((output/name/file).c_str());if(!SkPngEncoder::Encode(&stream,bitmap.pixmap(),{}))return 3;report<<",\"image\":";quote(report,name+"/"+file);
   }
   auto content=[&](const auto& controller){report<<'[';if(controller->viewIsLoaded()&&controller->view()->window()){const auto r=frame(controller->view(),window);report<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height();}report<<']';};
   report<<",\"sourceContent\":";content(source);report<<",\"targetContent\":";content(target);report<<",\"groups\":[";bool d=false;
   std::function<void(const std::shared_ptr<UIView>&)> visit=[&](const auto& view){if(view->isHidden()||opacity(view)<.001f)return;if(std::dynamic_pointer_cast<BackdropEffectView>(view)){if(d)report<<',';d=true;report<<"{\"id\":";quote(report,identifier(view));report<<",\"frame\":";rect(report,frame(view,bar));report<<",\"alpha\":"<<opacity(view)<<'}';}for(const auto& child:view->subviews())visit(child);};visit(bar);report<<"],\"buttons\":[";d=false;
   for(const auto& entry:buttons) {
    if(entry->spacer||!entry->control->isDescendantOf(bar)||opacity(entry->control)<.001f)continue;
    if(d)report<<',';d=true;report<<"{\"id\":";quote(report,identifier(entry->control));report<<",\"role\":";quote(report,entry->back?"back":entry->item->identifier().value_or("close"));report<<",\"frame\":";rect(report,frame(entry->control,bar));report<<",\"inkFrame\":";rect(report,frame(entry->ink,bar));report<<",\"alpha\":"<<opacity(entry->control)<<",\"inkAlpha\":"<<opacity(entry->ink);
    const auto ink=std::dynamic_pointer_cast<Internal::NavigationInkLayer>(entry->ink->layer()->presentationOrSelf());report<<",\"blur\":"<<(ink?ink->blur():0)<<'}';
   }
   report<<"],\"titles\":[";d=false;std::function<void(const std::shared_ptr<UIView>&)> titles=[&](const auto& view){if(view->isHidden()||opacity(view)<.001f)return;if(const auto label=std::dynamic_pointer_cast<UILabel>(view);label&&label->superview().lock()&&label->superview().lock()->superview().lock()==bar&&std::dynamic_pointer_cast<Internal::NavigationInkView>(label->superview().lock())){if(d)report<<',';d=true;report<<"{\"id\":";quote(report,identifier(label));report<<",\"text\":";quote(report,label->text());report<<",\"frame\":";rect(report,frame(label,bar));report<<",\"alpha\":"<<opacity(label)<<'}';}for(const auto& child:view->subviews())titles(child);};titles(bar);
   report<<"]}";
  }
  report<<"]}";report.flush();UIView::completePendingAnimations();DispatchQueueTestHarness::pump();window->setRootViewController(nullptr);UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();
 }
 report<<"]}\n";UIApplication::shared.reset();SkiaCtx::_main.reset();return 0;
}