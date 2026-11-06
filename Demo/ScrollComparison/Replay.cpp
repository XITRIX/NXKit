// Replay captured native OS touch coordinates/timestamps through NXKit.
#include <UIScrollView.h>
#include <UIApplication.h>
#include <UIViewController.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <SDL3/SDL.h>
#include <SkiaCtx.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <cmath>
using namespace NXKit;
namespace NXKit {
class UIScrollViewPhysicsTestHarness {
public:
 static auto pan(const std::shared_ptr<UIScrollView>& scroll){return scroll->_panGestureRecognizer;}
 static auto animation(const std::shared_ptr<UIScrollView>& scroll){return scroll->_timerAnimation;}
 static double duration(const std::shared_ptr<TimerAnimation>& timer){return timer->duration;}
 static void advance(const std::shared_ptr<TimerAnimation>& timer,double time){timer->advanceTo(time);}
};
class UIGestureRecognizerTestHarness {
public:
 static void update(const std::shared_ptr<UITouch>& touch, int phase, NXPoint p,Timer time,const std::shared_ptr<UIWindow>& window){touch->_window=window;touch->_phase=phase==0?UITouchPhase::began:phase==1?UITouchPhase::moved:phase==3?UITouchPhase::ended:UITouchPhase::cancelled;touch->updateAbsoluteLocation(p);touch->_timestamp=time;}
 static void append(const std::shared_ptr<UIEvent>& event,const std::shared_ptr<UITouch>& touch){event->_allTouches.insert(touch);}
 static void timestamp(const std::shared_ptr<UIEvent>& event,Timer time){event->_timestamp=time;}
};
}
class Context final:public SkiaCtx {
public:
 NXSize getSize()override{return {1000,1400};}float getScaleFactor()override{return 3;}
 sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
int main(int argc,char**argv){
 if(argc!=3){std::cerr<<"INPUT OUTPUT\n";return 2;}std::ifstream in(argv[1]);std::ofstream out(argv[2]);
 int isScroll,reset;unsigned long long minTouches,maxTouches;float initial,rx,ry,w,h,ox,oy;in>>isScroll>>initial>>minTouches>>maxTouches>>reset>>rx>>ry>>w>>h>>ox>>oy;
 SkiaCtx::_main=std::make_shared<Context>();UIApplication::shared=new_shared<UIApplication>();SDL_InitSubSystem(SDL_INIT_EVENTS);
 auto window=new_shared<UIWindow>();window->setFrame({0,0,1000,1400});auto controller=new_shared<UIViewController>();window->setRootViewController(controller);window->makeKeyAndVisible();
 std::shared_ptr<UIScrollView> scroll;std::shared_ptr<UIView> target;std::shared_ptr<UIPanGestureRecognizer> pan;
 if(isScroll){scroll=new_shared<UIScrollView>();scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);scroll->setContentSize({350,1600});scroll->setBounceVertically(true);target=scroll;pan=UIScrollViewPhysicsTestHarness::pan(scroll);}else{target=new_shared<UIView>();pan=new_shared<UIPanGestureRecognizer>();target->addGestureRecognizer(pan);}
 pan->setMinimumNumberOfTouches(minTouches);pan->setMaximumNumberOfTouches(maxTouches);
 target->setFrame({ox,oy,w,h});controller->view()->addSubview(target);window->drawAndLayoutTreeIfNeeded();if(scroll)scroll->setContentOffset({0,initial},false);
 // Keep one clock origin: Timer(double) is relative to now, not an absolute timestamp.
 const auto stamp=[](double t){Timer time;time.startTime.tv_sec=1000+long(t);time.startTime.tv_usec=long((t-std::floor(t))*1e6);return time;};
 double now=0,timerElapsed=0;std::shared_ptr<TimerAnimation> lastTimer;
 auto advance=[&](double t){double dt=std::max(0.0,t-now);now=t;for(int loops=0;scroll&&dt>1e-9&&loops<8;++loops){auto timer=UIScrollViewPhysicsTestHarness::animation(scroll);if(!timer)break;if(timer!=lastTimer){lastTimer=timer;timerElapsed=0;}double step=std::min(dt,std::max(0.0,UIScrollViewPhysicsTestHarness::duration(timer)-timerElapsed));timerElapsed+=step;UIScrollViewPhysicsTestHarness::advance(timer,timerElapsed);dt-=step;if(step==0&&UIScrollViewPhysicsTestHarness::animation(scroll)==timer)break;}};
 if(!isScroll)pan->onStateChanged=[&](auto recognizer){if(reset&&recognizer->state()==UIGestureRecognizerState::began)pan->setTranslation({rx,ry},target);};
 std::map<unsigned long,std::shared_ptr<UITouch>> touches;char kind;
 while(in>>kind){double t;in>>t;advance(t);
  if(kind=='E'){int count;in>>count;auto event=new_shared<UIEvent>();UIGestureRecognizerTestHarness::timestamp(event,stamp(t));for(int i=0;i<count;++i){unsigned long id;int phase;double timestamp;float x,y;in>>id>>phase>>timestamp>>x>>y;if(phase==2)continue;auto& touch=touches[id];if(!touch)touch=new_shared<UITouch>(id,NXPoint(x,y),stamp(timestamp));UIGestureRecognizerTestHarness::update(touch,phase,{x,y},stamp(timestamp),window);UIGestureRecognizerTestHarness::append(event,touch);}window->sendEvent(event);for(auto i=touches.begin();i!=touches.end();)if(i->second->phase()==UITouchPhase::ended||i->second->phase()==UITouchPhase::cancelled)i=touches.erase(i);else++i;
  }else if(kind=='F'){float width,height;in>>width>>height;if(target->bounds().size!=NXSize(width,height)){target->setFrame({ox,oy,width,height});target->layoutIfNeeded();}}
  auto translation=pan->translationInView(target),velocity=pan->velocityIn(target);auto offset=scroll?scroll->contentOffset():NXPoint();
  out<<kind<<' '<<t<<' '<<offset.x<<' '<<offset.y<<' '<<translation.x<<' '<<translation.y<<' '<<velocity.x<<' '<<velocity.y<<' '<<int(pan->state())<<' '<<(scroll&&scroll->isDecelerating())<<'\n';
  auto timer=scroll?UIScrollViewPhysicsTestHarness::animation(scroll):nullptr;if(timer!=lastTimer){lastTimer=timer;timerElapsed=0;}
 }
 window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);
}
