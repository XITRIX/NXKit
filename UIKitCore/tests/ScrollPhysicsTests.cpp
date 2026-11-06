#include <UIScrollView.h>
#include <UIApplication.h>
#include <UIViewController.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <UIControl.h>
#include <UIAction.h>
#include <SDL3/SDL.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>
#include <SkiaCtx.h>
#include "ScrollDynamics.hpp"
#include <cmath>
#include <iostream>
#include <map>
using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
class UIScrollViewPhysicsTestHarness {
public:
 static auto timer(const std::shared_ptr<UIScrollView>& s){return s->_timerAnimation;}
 static void step(const std::shared_ptr<UIScrollView>& s,double t){auto timer=s->_timerAnimation;if(timer)timer->advanceTo(t);}
 static void stepTimer(const std::shared_ptr<TimerAnimation>& timer,double t){timer->advanceTo(t);}
 static void start(const std::shared_ptr<UIScrollView>& s,NXPoint velocity){s->startScrollMotion(velocity);}
 static void dragFrame(const std::shared_ptr<UIScrollView>& s,double seconds){s->advanceDragSmoothing(seconds);}
 static bool following(const std::shared_ptr<UIScrollView>& s){return bool(s->_dragDisplayLink);}
 static bool indicatorsShown(const std::shared_ptr<UIScrollView>& s){return s->_scrollIndicatorsShown;}
};
class UIGestureRecognizerTestHarness {
public:
 static void update(const std::shared_ptr<UITouch>& t,UITouchPhase phase,NXPoint p,Timer time,const std::shared_ptr<UIWindow>& w){t->_window=w;t->_phase=phase;t->updateAbsoluteLocation(p);t->_timestamp=time;}
 static void append(const std::shared_ptr<UIEvent>& e,const std::shared_ptr<UITouch>& t){e->_allTouches.insert(t);}
};
}
namespace {
int failures=0;
void expect(bool condition,const char* message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
void near(float a,float b,float tolerance,const char* message){if(!std::isfinite(a)||std::abs(a-b)>tolerance){expect(false,message);std::cerr<<"  actual "<<a<<" expected "<<b<<" tolerance "<<tolerance<<'\n';}}
class Context final:public SkiaCtx {
public:
 int requestedFrameRate=0;
 void setTargetFrameRate(int rate)override{requestedFrameRate=rate;SkiaCtx::setTargetFrameRate(rate);}
 NXSize getSize()override{return {1000,1400};}float getScaleFactor()override{return 3;}
 sk_sp<SkSurface> getBackbufferSurface()override{return nullptr;}void swapBuffers()override{}void sleepForNextFrame()override{}bool platformRunLoop(std::function<bool()>)override{return false;}
protected:skgpu::graphite::Context* graphiteContext()override{return nullptr;}skgpu::graphite::Recorder* graphiteRecorder()override{return nullptr;}
};
struct Fixture {
 std::shared_ptr<UIWindow> window=new_shared<UIWindow>();
 std::shared_ptr<UIScrollView> scroll=new_shared<UIScrollView>();
 std::map<int,std::shared_ptr<UITouch>> touches;
 Fixture(){scroll->setDragSmoothingEnabled(false);window->setFrame({0,0,1000,1400});auto controller=new_shared<UIViewController>();window->setRootViewController(controller);window->makeKeyAndVisible();scroll->setFrame({20,100,350,500});scroll->setContentInsetAdjustmentBehavior(UIScrollViewContentInsetAdjustmentBehavior::never);scroll->setContentSize({350,1600});scroll->setBounceVertically(true);controller->view()->addSubview(scroll);window->drawAndLayoutTreeIfNeeded();}
 void send(int id,UITouchPhase phase,NXPoint p,double seconds){Timer stamp;stamp.startTime.tv_sec=1000+long(seconds);stamp.startTime.tv_usec=long((seconds-std::floor(seconds))*1e6);auto& touch=touches[id];if(!touch)touch=new_shared<UITouch>(id,p,stamp);UIGestureRecognizerTestHarness::update(touch,phase,p,stamp,window);auto event=new_shared<UIEvent>();UIGestureRecognizerTestHarness::append(event,touch);window->sendEvent(event);if(phase==UITouchPhase::ended||phase==UITouchPhase::cancelled)touches.erase(id);}
 void begin(int id,float y,double t){send(id,UITouchPhase::began,{150,y},t);}void move(int id,float y,double t){send(id,UITouchPhase::moved,{150,y},t);}void end(int id,float y,double t){send(id,UITouchPhase::ended,{150,y},t);}
};
struct Delegate:UIScrollViewDelegate {
 int began=0,ended=0,scrolled=0;bool decelerate=false;std::function<void()> onScroll;
 void scrollViewWillBeginDragging(std::shared_ptr<UIScrollView>)override{++began;}
 void scrollViewDidEndDragging(std::shared_ptr<UIScrollView>,bool flag)override{++ended;decelerate=flag;}
 void scrollViewDidScroll(std::shared_ptr<UIScrollView>)override{++scrolled;if(onScroll)onScroll();}
};
void multiTouch(){
 Fixture f;auto pan=f.scroll->panGestureRecognizer();auto delegate=std::make_shared<Delegate>();f.scroll->delegate=delegate;
 expect(pan->minimumNumberOfTouches()==1&&pan->maximumNumberOfTouches()==UINT32_MAX,"native pan touch-count defaults");
 f.begin(1,250,0);f.move(1,270,.02);f.move(1,310,.03);f.move(1,350,.04);
 auto tr=pan->translationInView(f.scroll);auto offset=f.scroll->contentOffset();f.begin(2,240,.05);
 near(pan->translationInView(f.scroll).y,tr.y,.001,"adding finger rebases centroid without moving translation");near(f.scroll->contentOffset().y,offset.y,.001,"adding finger preserves elastic position");
 f.move(2,300,.07);near(pan->translationInView(f.scroll).y,tr.y+30,.001,"two touches contribute their centroid delta");
 auto held=pan->translationInView(f.scroll);f.end(1,350,.08);expect(f.scroll->isDragging()&&pan->numberOfTouches()==1,"lifting one contact keeps pan alive");near(pan->translationInView(f.scroll).y,held.y,.001,"removing finger does not jump translation");
 f.move(2,360,.10);near(pan->translationInView(f.scroll).y,held.y+60,.001,"remaining contact continues from rebased centroid");
 int old=2;double time=.12;float position=360;
 for(int i=3;i<43;++i){f.begin(i,240,time);f.move(i,460,time+.02);f.end(old,position,time+.03);old=i;position=460;time+=.04;}
 expect(f.scroll->isDragging()&&f.scroll->contentOffset().y<-300,"alternating overlapping fingers can accumulate deep elastic pull");
 f.end(old,position,time);expect(!f.scroll->isDragging()&&!f.scroll->isTracking()&&f.scroll->isDecelerating(),"last finger begins elastic return");
 expect(delegate->began==1&&delegate->ended==1,"multi-contact drag delivers begin/end once");UIScrollViewPhysicsTestHarness::step(f.scroll,8);near(f.scroll->contentOffset().y,0,.001,"deep pull returns exactly to edge");
}
void catching(){
 Fixture f;f.scroll->setContentOffset({0,300},false);UIScrollViewPhysicsTestHarness::start(f.scroll,{0,1500});UIScrollViewPhysicsTestHarness::step(f.scroll,.15);float p=f.scroll->contentOffset().y;
 f.begin(1,350,0);expect(!f.scroll->isDecelerating()&&f.scroll->isTracking(),"touch catches existing inertia immediately");near(f.scroll->contentOffset().y,p,.001,"catch keeps visible offset");f.end(1,350,1);expect(!f.scroll->isDecelerating(),"stationary catch release does not restart inertia");UIScrollViewPhysicsTestHarness::step(f.scroll,10);near(f.scroll->contentOffset().y,p,.001,"stationary catch has no residual drift");
 f.begin(2,500,2);f.move(2,470,2.02);f.move(2,400,2.04);f.move(2,399,2.4);f.move(2,398,2.6);f.move(2,397,2.8);f.end(2,397,3);expect(!f.scroll->isDecelerating(),"slow release discards previous fling speed");
}
void regrabAndGeometry(){
 Fixture held;held.begin(1,250,0);held.move(1,280,.02);held.move(1,350,.04);held.move(1,450,.06);
 held.scroll->setFrame({20,100,350,300});held.scroll->layoutIfNeeded();held.end(1,450,.5);
 expect(!held.scroll->isDecelerating(),"lifting after a viewport clamp cannot revive the pre-resize swipe velocity");
 near(held.scroll->contentOffset().y,0,.001,"a held viewport clamp stays at its edge on release");
 Fixture f;f.scroll->setContentOffset({0,-110},false);UIScrollViewPhysicsTestHarness::start(f.scroll,{0,-200});UIScrollViewPhysicsTestHarness::step(f.scroll,.05);auto oldTimer=UIScrollViewPhysicsTestHarness::timer(f.scroll);float p=f.scroll->contentOffset().y;
 f.begin(1,300,0);near(f.scroll->contentOffset().y,p,.001,"bounce catch preserves stretched position");f.move(1,301,.02);near(f.scroll->contentOffset().y,p,.7,"first move does not apply rubber band twice");f.move(1,360,.04);expect(f.scroll->contentOffset().y<p-10,"regrab continues smoothly outward");float continued=f.scroll->contentOffset().y;UIScrollViewPhysicsTestHarness::stepTimer(oldTimer,6);near(f.scroll->contentOffset().y,continued,.001,"cancelled spring cannot overwrite a new drag");
 f.scroll->setFrame({20,100,350,300});f.scroll->layoutIfNeeded();near(f.scroll->contentOffset().y,0,.001,"viewport resize clamps elastic drag like UIKit");f.move(1,361,.06);near(f.scroll->contentOffset().y,0,.7,"post-resize raw gesture anchor follows new geometry");f.end(1,361,.1);UIScrollViewPhysicsTestHarness::step(f.scroll,8);near(f.scroll->contentOffset().y,0,.001,"resized pull settles");
 f.scroll->setContentOffset({0,1800},false);UIScrollViewPhysicsTestHarness::start(f.scroll,{0,200});UIScrollViewPhysicsTestHarness::step(f.scroll,.05);f.scroll->setFrame({20,100,350,700});f.scroll->layoutIfNeeded();UIScrollViewPhysicsTestHarness::step(f.scroll,8);near(f.scroll->contentOffset().y,900,.001,"active return retargets changed bottom limit");expect(!f.scroll->isDecelerating(),"retargeted return finishes");
 f.scroll->setContentOffset({0,800},false);f.scroll->setContentSize({350,900});f.scroll->layoutIfNeeded();near(f.scroll->contentOffset().y,200,.001,"idle content shrink clamps offset");
 f.scroll->setContentOffset({0,-200},false);f.scroll->setContentInset({20,0,0,0});f.scroll->layoutIfNeeded();near(f.scroll->contentOffset().y,-20,.001,"inset change repairs idle elastic offset");
}
void fractionalRestingEdge(){
 Fixture f;f.scroll->setContentInset({20.2f,0,0,0});f.scroll->layoutIfNeeded();f.scroll->setContentOffset({0,-100},false);UIScrollViewPhysicsTestHarness::start(f.scroll,{});UIScrollViewPhysicsTestHarness::step(f.scroll,8);near(f.scroll->contentOffset().y,-20.2f,.001,"resting edge preserves fractional inset instead of rounding out of bounds");expect(!f.scroll->isDecelerating(),"fractional edge return terminates");
}
void timerReentrancy(){
 Fixture f;auto delegate=std::make_shared<Delegate>();f.scroll->delegate=delegate;f.scroll->setContentOffset({0,200},false);bool replaced=false;
 delegate->onScroll=[&]{if(!replaced){replaced=true;f.scroll->setContentOffset({0,600},false);UIScrollViewPhysicsTestHarness::start(f.scroll,{0,-300});}};
 UIScrollViewPhysicsTestHarness::start(f.scroll,{0,1000});auto timer=UIScrollViewPhysicsTestHarness::timer(f.scroll);UIScrollViewPhysicsTestHarness::step(f.scroll,6);expect(f.scroll->isDecelerating(),"old completion cannot cancel replacement created by didScroll");near(f.scroll->contentOffset().y,600,.001,"reentrant offset wins");UIScrollViewPhysicsTestHarness::stepTimer(timer,10);near(f.scroll->contentOffset().y,600,.001,"old timer stays cancelled");UIScrollViewPhysicsTestHarness::step(f.scroll,6);expect(f.scroll->contentOffset().y<500,"replacement timer continues");
 UIScrollViewPhysicsTestHarness::start(f.scroll,{0,1000});auto p=f.scroll->contentOffset();f.scroll->setContentOffset(p,false);expect(!f.scroll->isDecelerating(),"setting same offset cancels inertia");
}
void recognizerSemantics(){
 Fixture f;auto view=new_shared<UIView>();view->setFrame({500,100,300,500});f.window->addSubview(view);
 auto pan=new_shared<UIPanGestureRecognizer>();view->addGestureRecognizer(pan);pan->setMinimumNumberOfTouches(2);
 auto send=[&](int id,UITouchPhase phase,float y,double t){f.send(id,phase,{650,y},t);};
 send(1,UITouchPhase::began,450,0);send(1,UITouchPhase::moved,420,.02);
 expect(pan->state()==UIGestureRecognizerState::possible,"minimum count delays recognition");
 send(2,UITouchPhase::began,450,.03);send(1,UITouchPhase::moved,400,.04);
 expect(pan->state()==UIGestureRecognizerState::began&&pan->numberOfTouches()==2,"minimum two begins with both contacts");
 near(pan->translationInView(view).y,-40,.001,"minimum two retains movement while awaiting second contact");
 send(1,UITouchPhase::ended,400,.05);expect(pan->state()==UIGestureRecognizerState::began&&pan->numberOfTouches()==1,"recognized pan continues below its minimum count");
 auto reference=new_shared<UIView>();reference->setFrame({800,600,100,100});reference->setTransform(NXAffineTransform::scale(2));f.window->addSubview(reference);
 auto velocity=pan->velocityIn(f.window);pan->setTranslation({6,-4},reference);
 near(pan->translationInView(reference).x,6,.001,"translation setter honors scaled coordinate space");near(pan->translationInView(f.window).y,-8,.001,"translation vectors exclude view origin");near(pan->velocityIn(f.window).y,velocity.y,.001,"setTranslation preserves velocity");
 reference->setTransform(NXAffineTransform::rotationBy(90)*NXAffineTransform::scale(2));
 pan->setTranslation({6,-4},reference);near(pan->translationInView(f.window).x,8,.001,"translation respects rotated reference axes");near(pan->translationInView(f.window).y,12,.001,"translation respects composed affine scale and rotation");
 send(2,UITouchPhase::ended,450,.06);expect(pan->state()==UIGestureRecognizerState::possible&&pan->numberOfTouches()==0,"last touch resets recognition attempt");near(pan->translationInView(reference).y,-4,.001,"final translation survives reset");near(pan->velocityIn(reference).y,0,.001,"velocity resets between attempts");
 bool rejected=false;try{pan->setMinimumNumberOfTouches(0);}catch(const std::invalid_argument&){rejected=true;}expect(rejected,"zero touch count rejected");
 pan->setMinimumNumberOfTouches(1);pan->setMaximumNumberOfTouches(1);
 send(3,UITouchPhase::began,450,1);send(3,UITouchPhase::moved,420,1.02);send(4,UITouchPhase::began,450,1.03);
 expect(pan->state()==UIGestureRecognizerState::began&&pan->numberOfTouches()==1,"maximum contact count does not cancel an already recognized pan");
 send(3,UITouchPhase::cancelled,420,1.04);send(4,UITouchPhase::cancelled,450,1.05);
}
void lateContactCancellation(){
 Fixture f;auto control=new_shared<UIControl>();control->setFrame({0,0,350,1600});f.scroll->addSubview(control);int actions=0;control->primaryAction=UIAction("",[&]{++actions;});
 f.begin(1,350,0);f.move(1,380,.02);expect(!control->isHighlighted(),"scroll activation cancels initial child control touch");
 f.begin(2,300,.03);expect(!control->isHighlighted(),"later contact cannot activate a child control during a scroll");f.end(2,300,.04);f.end(1,380,.05);expect(actions==0,"neither finger fires child control action");
}
void inwardReleaseContinuesScrolling(){
 for(bool bottom:{false,true}) {
  Fixture f;const float sign=bottom?-1.f:1.f;const float edge=bottom?1100.f:0.f;
  f.scroll->setContentOffset({0,edge},false);
  auto y=[&](float top){return bottom?700.f-top:top;};
  f.begin(1,y(250),0);f.move(1,y(280),.02);f.move(1,y(320),.04);f.move(1,y(500),.2);
  f.move(1,y(460),.22);f.move(1,y(420),.24);f.move(1,y(380),.26);
  expect(sign*(f.scroll->contentOffset().y-edge)<0,"reverse fling is released while content is still beyond the edge");
  f.end(1,y(380),.27);UIScrollViewPhysicsTestHarness::step(f.scroll,.5);
  expect(sign*(f.scroll->contentOffset().y-edge)>20,"inward fling escapes elastic return and scrolls into content");
  float entered=f.scroll->contentOffset().y;UIScrollViewPhysicsTestHarness::step(f.scroll,1);
  expect(sign*(f.scroll->contentOffset().y-entered)>0,"inward inertia continues rather than returning to the original edge");
 }
 Internal::ScrollAxisMotion a,b;a.begin(-40,2400,0,1100,true,2,.001);b=a;a.advance(.3);for(int i=0;i<30;++i)b.advance(.01);
 expect(!a.spring&&a.position>0&&a.velocity>0,"crossing spring hands off inward position and velocity to free flight");
 near(a.position,b.position,.001,"elastic-to-free handoff is independent of frame subdivision");near(a.velocity,b.velocity,.001,"elastic-to-free handoff preserves frame-independent velocity");
}
void outwardReleaseKeepsMomentum(){
 for(bool horizontal:{false,true})for(bool trailing:{false,true}) {
  Fixture f;f.scroll->setContentSize({1600,1600});f.scroll->setBounceHorizontally(true);f.scroll->layoutIfNeeded();
  const float boundary=trailing?(horizontal?1250.f:1100.f):0.f;
  const float direction=trailing?1.f:-1.f;
  auto point=[&](float value){return horizontal?NXPoint{value,200}:NXPoint{150,value};};
  f.scroll->setContentOffset(horizontal?NXPoint{boundary-direction*50,300}:NXPoint{300,boundary-direction*50},false);
  const float origin=horizontal?190:350;
  f.send(1,UITouchPhase::began,point(origin),0);
  for(int i=1;i<=5;++i)f.send(1,UITouchPhase::moved,point(origin-direction*30*i),.02*i);
  const auto stretch=[&]{return direction*((horizontal?f.scroll->contentOffset().x:f.scroll->contentOffset().y)-boundary);};
  const auto released=stretch();expect(released>0&&released<50,"fast swipe reaches modest overscroll before lift-off");
  f.send(1,UITouchPhase::ended,point(origin-direction*150),.11);
  float peak=released;
  for(int i=1;i<=120;++i){UIScrollViewPhysicsTestHarness::step(f.scroll,i/120.);peak=std::max(peak,stretch());}
  expect(peak>released+10,"outward release preserves momentum beyond every edge instead of hitting a wall");
  UIScrollViewPhysicsTestHarness::step(f.scroll,3);
  near(stretch(),0,.001,"outward fling returns exactly to its own edge");
  expect(!f.scroll->isDecelerating(),"outward edge fling terminates after its return");
 }
 for(bool horizontal:{false,true})for(bool trailing:{false,true})for(bool bounces:{false,true}) {
  Fixture f;f.scroll->setContentSize({1600,1600});f.scroll->setBounceHorizontally(bounces);f.scroll->setBounceVertically(bounces);f.scroll->layoutIfNeeded();
  const float boundary=trailing?(horizontal?1250.f:1100.f):0.f,direction=trailing?1.f:-1.f;
  auto point=[&](float value){return horizontal?NXPoint{value,200}:NXPoint{150,value};};
  const float origin=horizontal?190:350;
  f.scroll->setContentOffset(horizontal?NXPoint{boundary+direction*100,300}:NXPoint{300,boundary+direction*100},false);
  f.send(1,UITouchPhase::began,point(origin),0);
  for(int i=1;i<=5;++i)f.send(1,UITouchPhase::moved,point(origin-direction*10*i),.02*i);
  const auto stretch=[&]{return direction*((horizontal?f.scroll->contentOffset().x:f.scroll->contentOffset().y)-boundary);};
  const auto released=stretch();f.send(1,UITouchPhase::ended,point(origin-direction*50),.6);
  if(!bounces)expect(!f.scroll->isDecelerating(),"a non-bouncing release at the boundary stops immediately");
  for(int i=1;i<=120;++i){UIScrollViewPhysicsTestHarness::step(f.scroll,i/120.);expect(stretch()<=released+.5f,"a gentle deep pull stays passive instead of reviving its cached velocity");}
 }
}
void axisDynamics(){
 Internal::ScrollAxisMotion a,b;a.begin(20,1600,0,100,true,2,.001);b=a;a.advance(1);for(int i=0;i<100;++i)b.advance(.01);near(a.position,b.position,.001,"edge impact and return are independent of frame subdivision");near(a.velocity,b.velocity,.001,"spring velocity independent of frame subdivision");
 a.begin(-150,0,0,100,true,2,.001);a.advance(10);near(a.position,0,.001,"spring has exact terminal edge");expect(!a.active,"spring terminates");
 a.begin(20,-1600,0,100,false,2,.001);a.advance(.5);near(a.position,0,.001,"non-bouncing axis stops at collision");expect(!a.active,"non-bouncing collision terminates");
 Internal::ScrollMotion motion;motion.x.begin(90,1000,0,100,true,2,.001);motion.y.begin(100,100,0,1000,true,2,.001);motion.advanceTo(.1);expect(motion.x.spring&&!motion.y.spring&&motion.y.position>100,"diagonal collision does not bounce other axis");
}

void dragFollower() {
 DragSmoother a,b;a.reset({});a.setTarget({4,0});b=a;
 const auto first=a.advance(1.0/120);
 expect(first.x>0&&first.x<4,"coarse sample produces fractional visual motion");
 a.advance(1.0/120);b.advance(1.0/60);
 near(a.position().x,b.position().x,.0001,"following is independent of frame subdivision");
 auto last=a.position().x;
 for(int i=0;i<60;++i){auto p=a.advance(1.0/120);expect(p.x>=last&&p.x<=4,"follower never overshoots or reverses");last=p.x;}
 expect(a.isSettled()&&a.position().x==4,"stationary finger converges exactly without prediction drift");
 a.setTarget({-4,0});expect(a.advance(.01).x<4,"direction reversal responds on next frame");
 DragSmoother slow,fast;slow.reset({});fast.reset({});slow.setTarget({100,0},{});fast.setTarget({100,0},{1000,0});
 expect(fast.advance(.01).x>slow.advance(.01).x,"fast motion uses less latency");
 bool invalid=false;try{a.advance(-1);}catch(const std::invalid_argument&){invalid=true;}expect(invalid,"invalid frame duration rejected");
 a.reset({10000000,0});a.setTarget({10000004,0});for(int i=0;i<120;++i)a.advance(1.0/120);
 expect(a.isSettled(),"large content coordinates converge despite float precision");
 const auto large=std::numeric_limits<NXFloat>::max();a.reset({-large,0});a.setTarget({large,0});
 expect(std::isfinite(a.advance(.01).x),"finite endpoint interpolation cannot overflow");
}

void smoothScrollDrag() {
 Fixture f;f.scroll->setDragSmoothingEnabled(true);f.scroll->setContentOffset({0,300},false);
 f.begin(1,350,0);f.move(1,330,.02);f.move(1,326,.04);f.move(1,322,.08);
 near(f.scroll->contentOffset().y,300,.001,"raw touch target does not jump displayed content");
 near(f.scroll->panGestureRecognizer()->translationInView(f.scroll).y,-4,.001,"smoothing preserves measured pan translation");
 UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,1.0/120);
 const auto visible=f.scroll->contentOffset().y;
 expect(visible>300&&visible<304&&std::abs(visible-std::round(visible))>.001,"scroll follows at subpixel positions");
 f.end(1,322,.10);
 expect(!f.scroll->isDragging()&&!f.scroll->isDecelerating(),"slow release does not invent fling velocity");
 UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,.5);
 near(f.scroll->contentOffset().y,304,.001,"slow release finishes pending measured distance smoothly");
 expect(!UIScrollViewPhysicsTestHarness::following(f.scroll),"settled drag releases frame callback");
 expect(!UIScrollViewPhysicsTestHarness::indicatorsShown(f.scroll),"settled release hides scroll indicators");

 f.begin(2,350,1);f.move(2,330,1.02);f.move(2,326,1.04);f.move(2,300,1.08);
 f.scroll->setContentOffset({0,700},false);UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,.5);
 near(f.scroll->contentOffset().y,700,.001,"programmatic offset cancels pending drag target");
 f.move(2,296,1.12);UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,.5);
 near(f.scroll->contentOffset().y,704,.001,"pan anchor rebases after programmatic move");
 f.move(2,292,1.16);auto delegate=std::make_shared<Delegate>();f.scroll->delegate=delegate;
 delegate->onScroll=[&]{f.scroll->setContentOffset({0,600},false);};
 UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,.01);delegate->onScroll={};
 UIScrollViewPhysicsTestHarness::dragFrame(f.scroll,.5);
 near(f.scroll->contentOffset().y,600,.001,"didScroll replacement cancels old follower reentrantly");
 f.move(2,288,1.20);f.scroll->setScrollEnabled(false);
 expect(!UIScrollViewPhysicsTestHarness::following(f.scroll),"disabling scrolling cancels follower");

 Fixture removed;removed.scroll->setDragSmoothingEnabled(true);
 removed.begin(1,250,0);removed.move(1,270,.02);removed.move(1,274,.04);removed.move(1,278,.08);
 auto before=removed.scroll->contentOffset();removed.scroll->removeFromSuperview();
 UIScrollViewPhysicsTestHarness::dragFrame(removed.scroll,.5);
 expect(removed.scroll->contentOffset()==before&&!UIScrollViewPhysicsTestHarness::following(removed.scroll),"removed scroll view stops pending motion");

 Fixture fling;fling.scroll->setDragSmoothingEnabled(true);fling.scroll->setContentOffset({0,300},false);
 fling.begin(1,350,0);fling.move(1,330,.02);fling.move(1,326,.04);fling.move(1,200,.06);
 const auto releaseVelocity=fling.scroll->panGestureRecognizer()->velocityIn(fling.scroll);
 UIScrollViewPhysicsTestHarness::dragFrame(fling.scroll,.001);
 const auto releaseOffset=fling.scroll->contentOffset().y;
 near(fling.scroll->panGestureRecognizer()->velocityIn(fling.scroll).y,releaseVelocity.y,.001,"visual interpolation leaves release velocity unchanged");
 fling.end(1,200,.07);
 expect(fling.scroll->isDecelerating()&&!UIScrollViewPhysicsTestHarness::following(fling.scroll),"fast release hands off to physics without a competing follower");
 expect(fling.scroll->contentOffset().y>=releaseOffset&&fling.scroll->contentOffset().y<426,"inertia begins from displayed content rather than snapping to raw drag target");

 Fixture resized;resized.scroll->setDragSmoothingEnabled(true);
 resized.begin(1,250,0);resized.move(1,270,.02);resized.move(1,274,.04);resized.move(1,278,.08);
 resized.scroll->setFrame({20,100,350,400});resized.scroll->layoutIfNeeded();
 expect(!UIScrollViewPhysicsTestHarness::following(resized.scroll),"viewport change discards stale drag target");
 expect(resized.scroll->applyXMLAttribute("dragSmoothingEnabled","false")&&!resized.scroll->isDragSmoothingEnabled(),"XML exposes smoothing policy");
}

void queuedTouchSamples() {
 struct Samples: UIView {
  std::vector<Timer> times;
  void touchesMoved(std::set<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>)override {
   times.push_back((*touches.begin())->timestamp());
  }
 };
 Fixture f;auto samples=new_shared<Samples>();samples->setFrame({500,100,300,600});f.window->addSubview(samples);
 SDL_Delay(20);const auto now=SDL_GetTicksNS();
 SDL_Event event{};event.type=SDL_EVENT_FINGER_DOWN;event.tfinger.fingerID=777;
 event.tfinger.x=.65f;event.tfinger.y=.25f;event.tfinger.timestamp=now-16000000;
 SDL_PushEvent(&event);
 for(int i=0;i<3;++i){event.type=SDL_EVENT_FINGER_MOTION;event.tfinger.y-=.004f;event.tfinger.timestamp=now-(12000000-i*4000000);SDL_PushEvent(&event);}
 UIApplication::shared->handleEventsIfNeeded();
 expect(samples->times.size()==3,"queued motion packets are all delivered, including intervals below 5ms");
 for(const auto& time:samples->times)expect(time.startTime.tv_usec>=0&&time.startTime.tv_usec<1000000,"sample timestamps keep canonical timeval fields");
 if(samples->times.size()==3)near(float(samples->times[2]-samples->times[1]),4,.5,"velocity uses sample timing instead of event-drain timing");
 event.type=SDL_EVENT_FINGER_UP;event.tfinger.timestamp=now;SDL_PushEvent(&event);UIApplication::shared->handleEventsIfNeeded();
}
void slowReleaseCadence() {
 Fixture f;f.scroll->setDragSmoothingEnabled(true);f.scroll->setContentOffset({0,300},false);
 f.begin(1,350,0);f.move(1,330,.02);f.move(1,326,.04);f.move(1,322,.08);f.end(1,322,.10);
 expect(UIScrollViewPhysicsTestHarness::following(f.scroll),"slow release retains a pending measured position");
 applicationRunLoop();
 expect(std::static_pointer_cast<Context>(SkiaCtx::main())->requestedFrameRate==120,"slow-release follower keeps high refresh after active touches end");
}
}
int main(){SDL_SetMainReady();SkiaCtx::_main=std::make_shared<Context>();UIApplication::shared=new_shared<UIApplication>();if(!SDL_InitSubSystem(SDL_INIT_EVENTS)){std::cerr<<SDL_GetError()<<'\n';return 1;}multiTouch();catching();regrabAndGeometry();fractionalRestingEdge();timerReentrancy();recognizerSemantics();lateContactCancellation();inwardReleaseContinuesScrolling();outwardReleaseKeepsMomentum();axisDynamics();dragFollower();smoothScrollDrag();queuedTouchSamples();slowReleaseCadence();UIApplication::shared.reset();SkiaCtx::_main.reset();SDL_QuitSubSystem(SDL_INIT_EVENTS);std::cout<<"Scroll physics failures: "<<failures<<'\n';return failures?1:0;}
