#include <UIKit.h>
#include <UIApplication.h>
#include <SkiaCtx.h>
#include <UIWindow.h>
#include <UIEvent.h>
#include <UITouch.h>
#include "UINavigationBarInternal.hpp"
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
auto items(const std::string& kind){
 auto icon=[](const char* symbol,const char* id){auto i=std::make_shared<UIBarButtonItem>(UIImage::systemImageNamed(symbol,20,400,3),UIBarButtonItemStyle::plain);i->setIdentifier(id);return i;};
 auto a=icon(kind.starts_with("trash")?"trash":"plus",kind=="trash-new"?"replacement":"a"),b=icon("ellipsis","b");
 if(kind=="single"||kind.starts_with("trash"))return std::vector{a};
 if(kind=="accent")a->setStyle(UIBarButtonItemStyle::prominent);
 if(kind=="split")return std::vector{a,UIBarButtonItem::fixedSpaceItemOfWidth(0),b};
 if(kind=="three")return std::vector{a,b,icon("trash","c")};
 if(kind=="text"){a=std::make_shared<UIBarButtonItem>("Edit",UIBarButtonItemStyle::plain);a->setIdentifier("a");b=std::make_shared<UIBarButtonItem>("Save",UIBarButtonItemStyle::plain);b->setIdentifier("b");}
 return std::vector{a,b};
}
void animate(const std::shared_ptr<CALayer>& layer,Timer time){const auto children=layer->sublayers();layer->animateAt(time);for(const auto& child:children)animate(child,time);}
void rect(std::ostream& out,NXRect r){out<<'['<<r.minX()<<','<<r.minY()<<','<<r.width()<<','<<r.height()<<']';}
NXRect frame(const std::shared_ptr<UIView>& view,const std::shared_ptr<UIView>& root){
 const auto bounds=view->layer()->presentationOrSelf()->bounds();NXKit::NXPoint points[]={{bounds.minX(),bounds.minY()},{bounds.maxX(),bounds.minY()},{bounds.minX(),bounds.maxY()},{bounds.maxX(),bounds.maxY()}};
 for(auto current=view;current&&current!=root;current=current->superview().lock()){const auto p=current->layer()->presentationOrSelf();for(auto& point:points)point=(point-p->bounds().origin-NXKit::NXPoint{p->bounds().width()*p->anchorPoint().x,p->bounds().height()*p->anchorPoint().y}).applying(NXTransform3DGetAffineTransform(p->transform()))+p->position();}
 NXFloat x=points[0].x,y=points[0].y,right=x,bottom=y;for(auto p:points){x=std::min(x,p.x);y=std::min(y,p.y);right=std::max(right,p.x);bottom=std::max(bottom,p.y);}return{x,y,right-x,bottom-y};
}
}
int main(int argc,char**argv){
 if(argc<2)return 2;const std::filesystem::path output=argv[1];std::filesystem::create_directories(output);
 auto ctx=std::make_shared<Context>();SkiaCtx::_main=ctx;UIApplication::shared=new_shared<UIApplication>();UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
 std::ofstream report(output/"motion.json");report<<"{\"scale\":3,\"imageBarOriginY\":12,\"clips\":[";bool comma=false;
 struct Case{const char* name;const char* from;const char* to;bool pop=false;};
 for(bool dark:{false,true})for(const auto c:{Case{"split","pair","split"},Case{"join","split","pair"},Case{"grow","single","pair"},Case{"shrink","pair","single"},Case{"replace","single","trash"},Case{"replace-unmatched","single","trash-new"},Case{"text","pair","text"},Case{"accent","pair","accent"},Case{"pop-split","pair","split",true},Case{"pop-join","split","pair",true}}){
  if(argc>2&&std::string(argv[2])!=c.name)continue;ctx->dark=dark;auto screen=[&](const char* kind){auto v=new_shared<UIViewController>();v->view()->setBackgroundColor(dark?UIColor::black:UIColor::systemGroupedBackground);v->navigationItem()->setRightBarButtonItems(items(kind));return v;};
  auto source=screen(c.from),target=screen(c.to);auto nav=new_shared<UINavigationController>(c.pop?target:source);if(c.pop)nav->pushViewController(source,false);
  auto window=new_shared<UIWindow>();UIApplication::shared->delegate->window=window;window->setRootViewController(nav);window->makeKeyAndVisible();window->drawAndLayoutTreeIfNeeded();UIGestureRecognizerTestHarness::clearFocus(window);UIView::completePendingAnimations();
  const auto name=std::string(c.name)+(dark?"-dark":"-light");std::filesystem::create_directories(output/name);auto bar=nav->navigationBar();
  if(comma)report<<',';comma=true;report<<"{\"name\":\""<<name<<"\",\"frames\":[";
  auto origin=Timer();
  for(int i=0;i<=63;++i){if(i==1){origin=Timer();if(c.pop)nav->popViewController(true);else nav->pushViewController(target,true);window->drawAndLayoutTreeIfNeeded();}auto time=origin;const long us=time.startTime.tv_usec+long(i*1000000/60);time.startTime.tv_sec+=us/1000000;time.startTime.tv_usec=us%1000000;animate(window->layer(),time);
   auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1206,240));auto canvas=surface->getCanvas();canvas->clear(dark?SK_ColorBLACK:SkColorSetRGB(242,242,247));canvas->scale(3,3);canvas->translate(0,-50);UITraitCollection::setCurrent(window->traitCollection());bar->layer()->presentationOrSelf()->skiaRender(canvas);
   SkBitmap bitmap;bitmap.allocN32Pixels(1206,240);surface->readPixels(bitmap,0,0);char filename[32];snprintf(filename,sizeof(filename),"frame-%03d.png",i);SkFILEWStream stream((output/name/filename).c_str());if(!SkPngEncoder::Encode(&stream,bitmap.pixmap(),{}))return 3;
   if(i)report<<',';report<<"{\"t\":"<<i/60.<<",\"image\":\""<<name<<'/'<<filename<<"\",\"groups\":[";bool d=false;
   std::function<void(const std::shared_ptr<UIView>&)> visit=[&](const auto& view){if(view->isHidden()||view->layer()->presentationOrSelf()->opacity()<.001f)return;if(std::dynamic_pointer_cast<BackdropEffectView>(view)){if(d)report<<',';d=true;rect(report,frame(view,bar));}for(const auto& child:view->subviews())visit(child);};visit(bar);report<<"]}";
  }report<<"]}";window->setRootViewController(nullptr);UIView::completePendingAnimations();
 }
 report<<"]}\n";UIApplication::shared->delegate->window.reset();UIApplication::shared->keyWindow.reset();UIApplication::shared.reset();SkiaCtx::_main.reset();
}
