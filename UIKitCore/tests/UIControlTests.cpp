#include <UIControl.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <UIScrollView.h>
#include <iostream>
#include <stdexcept>
using namespace NXKit;
namespace NXKit {
class UIGestureRecognizerTestHarness {
public:
 static void send(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch,UITouchPhase phase,NXKit::NXPoint p) {
  touch->_window=window;touch->_phase=phase;touch->updateAbsoluteLocation(p);
  auto event=new_shared<UIEvent>();event->_allTouches.insert(touch);window->sendEvent(event);
 }
};
}
namespace {
int failures=0;
void expect(bool c,const char* m){if(!c){++failures;std::cerr<<"FAIL: "<<m<<'\n';}}
struct Target: UIView {
 int calls=0;std::shared_ptr<UIResponder> sender;std::shared_ptr<UIEvent> event;
 bool canPerformAction(const std::string& selector,const std::shared_ptr<UIResponder>&) override{return selector=="changed:";}
 bool performAction(const std::string& selector,const std::shared_ptr<UIResponder>& s,const std::shared_ptr<UIEvent>& e) override {
  if(selector!="changed:")return false;++calls;sender=s;event=e;return true;
 }
};
struct Probe: UIControl {
 int begins=0,moves=0,ends=0,cancels=0;bool accepts=true,continues=true;
 bool beginTracking(const std::shared_ptr<UITouch>&,const std::shared_ptr<UIEvent>&)override{++begins;return accepts;}
 bool continueTracking(const std::shared_ptr<UITouch>&,const std::shared_ptr<UIEvent>&)override{++moves;return continues;}
 void endTracking(const std::shared_ptr<UITouch>&,const std::shared_ptr<UIEvent>&)override{++ends;}
 void cancelTracking(const std::shared_ptr<UIEvent>& e)override{if(isTracking())++cancels;UIControl::cancelTracking(e);}
};
void registry() {
 auto control=new_shared<UIControl>();
 expect(control->isEnabled()&&!control->isSelected()&&!control->isHighlighted()&&control->state()==UIControlState::normal,"UIKit state defaults");
 control->setSelected(true);control->setHighlighted(true);control->setEnabled(false);
 expect(control->state()==(UIControlState::disabled|UIControlState::selected|UIControlState::highlighted),"disabling preserves manually assigned state flags (native state7)");
 int primaryCalls=0;UIAction primary("Primary","initializer-primary",[&](const UIAction&){++primaryCalls;});
 auto initialized=new_shared<UIControl>(NXRect(),primary);initialized->setEnabled(false);initialized->performPrimaryAction();
 expect(primaryCalls==1,"explicit primary dispatch works while disabled");
 initialized->removeAction(primary,UIControlEvent::primaryActionTriggered);initialized->performPrimaryAction();
 expect(primaryCalls==1,"initializer primary action participates in the action registry");
 int a=0,b=0;std::shared_ptr<UIResponder> sender;
 UIAction original("original","same",[&](const UIAction& action){++a;sender=action.sender();});
 UIAction replacement("replacement","same",[&](const UIAction& action){++b;sender=action.sender();});
 control->addAction(original,UIControlEvent::touchDown|UIControlEvent::valueChanged);
 control->sendActions(UIControlEvent::valueChanged);
 expect(a==1&&sender==control,"programmatic send works while disabled and UIAction receives its sender");
 control->addAction(replacement,UIControlEvent::valueChanged);
 control->sendActions(UIControlEvent::touchDown|UIControlEvent::valueChanged);
 expect(a==1&&b==1,"same action identifier replaces handler and combined event dispatch is unique");
 control->removeAction(replacement,UIControlEvent::valueChanged);control->sendActions(UIControlEvent::valueChanged);
 expect(b==1,"partial action removal removes only requested events");
 control->sendActions(UIControlEvent::touchDown);expect(b==2,"unremoved event keeps replacement handler");
 control->removeAction("same",UIControlEvent::allEvents);
 expect(std::uint32_t(control->allControlEvents())==0,"all-events action removal");
 auto target=new_shared<Target>();control->addTarget(target,"changed:",UIControlEvent::valueChanged);
 control->addTarget(target,"changed:",UIControlEvent::valueChanged);
 control->sendActions(UIControlEvent::valueChanged);
 expect(target->calls==1&&target->sender==control,"deduplicated weak target dispatch includes sender");
 expect(control->allTargets().size()==1&&control->actionsForTarget(target,UIControlEvent::valueChanged)==std::vector<std::string>{"changed:"},"target/action inspection");
 target->sender.reset();std::weak_ptr<Target> lifetime=target;target.reset();
 expect(lifetime.expired()&&control->allTargets().empty(),"target-action registry does not retain targets");
 control->sendActions(UIControlEvent::valueChanged);
 auto parent=new_shared<Target>();parent->addSubview(control);control->setEnabled(true);
 control->addTarget(nullptr,"changed:",UIControlEvent::valueChanged);control->sendActions(UIControlEvent::valueChanged);
 expect(parent->calls==1&&parent->sender==control,"nil target resolves a responder-chain selector at dispatch time");parent->sender.reset();
 control->removeTarget(nullptr,std::nullopt,UIControlEvent::allEvents);
 int first=0,second=0,newlyAdded=0;
 UIAction next("next",[&]{++newlyAdded;});UIAction tail("tail",[&]{++second;});
 UIAction mutation("mutation",[&]{++first;control->removeAction(tail,UIControlEvent::allEvents);control->addAction(next,UIControlEvent::valueChanged);});
 control->addAction(mutation,UIControlEvent::valueChanged);control->addAction(tail,UIControlEvent::valueChanged);
 control->sendActions(UIControlEvent::valueChanged);
 expect(first==1&&second==1&&newlyAdded==0,"dispatch snapshots tolerate callback registration/removal");
 int enumerated=0;
 control->enumerateEventHandlers([&](const auto&,const auto&,const auto&,auto events,bool& stop){++enumerated;expect(std::uint32_t(events)!=0,"enumeration never returns empty events");control->removeAction(next,UIControlEvent::allEvents);stop=true;});
 expect(enumerated==1,"event enumeration supports mutation and stopping");
 control->setContentHorizontalAlignment(UIControlContentHorizontalAlignment::trailing);
 expect(control->effectiveContentHorizontalAlignment()==UIControlContentHorizontalAlignment::right,"portable leading/trailing resolution");
 expect(control->applyXMLAttribute("enabled","false")&&!control->isEnabled()&&!control->applyXMLAttribute("contentHorizontalAlignment","invalid"),"XML control properties validate values");
}
void tracking() {
 auto window=new_shared<UIWindow>();window->setFrame({0,0,400,600});auto parent=new_shared<UIView>();parent->setFrame(window->bounds());window->addSubview(parent);
 auto c=new_shared<Probe>();c->setFrame({10,10,100,80});parent->addSubview(c);
 std::vector<std::string> events;
 for(const auto& [event,name]:std::vector<std::pair<UIControlEvent,std::string>>{{UIControlEvent::touchDown,"down"},{UIControlEvent::touchDragInside,"inside"},{UIControlEvent::touchDragOutside,"outside"},{UIControlEvent::touchDragEnter,"enter"},{UIControlEvent::touchDragExit,"exit"},{UIControlEvent::touchUpInside,"up"},{UIControlEvent::touchUpOutside,"upOutside"},{UIControlEvent::touchCancel,"cancel"}})
  c->addAction(UIAction(name,[&,name]{events.push_back(name);}),event);
 int primary=0;c->addAction(UIAction("primary",[&]{++primary;}),UIControlEvent::primaryActionTriggered);
 auto t=new_shared<UITouch>(1,NXKit::NXPoint{30,30},Timer());
 auto send=[&](UITouchPhase phase,NXKit::NXPoint p){UIGestureRecognizerTestHarness::send(window,t,phase,p);};
 send(UITouchPhase::began,{30,30});expect(c->isTracking()&&c->isTouchInside()&&c->isHighlighted(),"touch down starts tracking and highlight");
 send(UITouchPhase::moved,{180,30});expect(c->isTracking()&&!c->isTouchInside()&&!c->isHighlighted(),"drag outside preserves tracking, clears inside/highlight");
 send(UITouchPhase::moved,{30,30});send(UITouchPhase::ended,{30,30});
 expect(c->begins==1&&c->moves==2&&c->ends==1&&!c->isTracking()&&!c->isTouchInside()&&!c->isHighlighted(),"tracking hooks and terminal flags");
 expect(events==std::vector<std::string>{"down","outside","exit","inside","enter","up"},"touch down/drag exit/enter/up event delivery");
 expect(primary==1,"primary action is delivered once");
 for(int variant=0;variant<5;++variant) {
  events.clear();t=new_shared<UITouch>(10+variant,NXKit::NXPoint{30,30},Timer());send(UITouchPhase::began,{30,30});
  if(variant==0)c->setEnabled(false);if(variant==1)parent->setHidden(true);if(variant==2)parent->setUserInteractionEnabled(false);if(variant==3)parent->setAlpha(0);if(variant==4)c->removeFromSuperview();
  expect(!c->isTracking()&&!c->isHighlighted()&&events==std::vector<std::string>{"down","cancel"},"disable/ancestor visibility/interaction/opacity/removal cancel once");
  send(UITouchPhase::ended,{30,30});expect(primary==1,"cancelled contacts never fire an action on up");
  c->setEnabled(true);parent->setHidden(false);parent->setUserInteractionEnabled(true);parent->setAlpha(1);if(variant==4)parent->addSubview(c);
 }
 events.clear();c->accepts=false;t=new_shared<UITouch>(22,NXKit::NXPoint{30,30},Timer());send(UITouchPhase::began,{30,30});send(UITouchPhase::ended,{30,30});
 expect(!c->isTracking()&&events.empty(),"beginTracking rejection emits no events");
 c->accepts=true;c->addAction(UIAction("disable",[&]{c->setEnabled(false);}),UIControlEvent::touchDown);
 t=new_shared<UITouch>(23,NXKit::NXPoint{30,30},Timer());send(UITouchPhase::began,{30,30});send(UITouchPhase::ended,{30,30});
 expect(!c->isTracking()&&!c->isHighlighted()&&primary==1,"disabling reentrantly on touchDown leaves no active tracking");
 auto repeated=new_shared<UIControl>(NXRect(200,10,100,80));parent->addSubview(repeated);int repeats=0;
 repeated->addAction(UIAction("Repeat",[&]{++repeats;}),UIControlEvent::touchDownRepeat);
 auto twice=new_shared<UITouch>(91,NXKit::NXPoint(220,30),Timer(),2);
 UIGestureRecognizerTestHarness::send(window,twice,UITouchPhase::began,{220,30});
 UIGestureRecognizerTestHarness::send(window,twice,UITouchPhase::ended,{220,30});
 expect(repeats==1,"tap-count repeat event is delivered through real control tracking");
}
}
int main(){registry();tracking();if(!failures)std::cout<<"UIControl tests passed\n";return failures?1:0;}
