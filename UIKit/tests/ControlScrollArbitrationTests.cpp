#include <DispatchQueue.h>
#include <LensView.h>
#include <UISlider.h>
#include <UISwitch.h>
#include <UIScrollView.h>
#include <UITouch.h>
#include <UIWindow.h>
#include "RasterTestContext.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

using namespace NXKit;
namespace NXKit {
class UIGestureRecognizerTestHarness {
public:
    static void send(const std::shared_ptr<UIWindow>& window,const std::shared_ptr<UITouch>& touch,
                     UITouchPhase phase,NXKit::NXPoint point,double seconds) {
        touch->_window=window;touch->_phase=phase;touch->updateAbsoluteLocation(point);
        touch->_timestamp.startTime.tv_sec=1000+static_cast<long>(seconds);
        touch->_timestamp.startTime.tv_usec=static_cast<long>((seconds-static_cast<long>(seconds))*1000000);
        auto event=new_shared<UIEvent>();event->_timestamp=touch->_timestamp;event->_allTouches.insert(touch);
        window->sendEvent(event);
    }
};
class DispatchQueueTestHarness {public:static void drain(){DispatchQueue::main()->performAll();}};
class UISwitchTestHarness {
public:
    static auto lens(const std::shared_ptr<UISwitch>& value){return value->_lens;}
    static bool admitted(const std::shared_ptr<UISwitch>& value){return value->_acceptedContact;}
};
class UISliderTestHarness {
public:
    static auto lens(const std::shared_ptr<UISlider>& value){return value->_lens;}
    static NXKit::NXPoint origin(const std::shared_ptr<UISlider>& value){return value->_dragOrigin;}
    static void settle(const std::shared_ptr<UISlider>& value){value->advanceRelease(.5);value->advanceThumb(.6);}
};
}

namespace {
int failures=0,cases=0;
void expect(bool condition,const char* message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
void waitForAdmission(){std::this_thread::sleep_for(std::chrono::milliseconds(175));DispatchQueueTestHarness::drain();}
struct Fixture {
    std::shared_ptr<UIWindow> window=new_shared<UIWindow>();
    std::shared_ptr<UIScrollView> scroll=new_shared<UIScrollView>();
    std::shared_ptr<UIView> content=new_shared<UIView>();
    std::shared_ptr<UISwitch> toggle;
    std::shared_ptr<UISlider> slider;
    std::shared_ptr<UITouch> touch;
    unsigned identifier=0;
    int changes=0;
    Fixture(bool useSlider=false,bool nested=true,bool horizontal=false) {
        ++cases;window->setFrame({0,0,400,600});scroll->setFrame(window->bounds());
        scroll->setContentSize(horizontal?NXKit::NXSize(900,600):NXKit::NXSize(400,1000));
        content->setFrame({0,0,horizontal?900.f:400.f,horizontal?600.f:1000.f});
        if(nested){window->addSubview(scroll);scroll->addSubview(content);}else window->addSubview(content);
        if(useSlider){slider=new_shared<UISlider>(NXRect(40,180,300,44));slider->setValue(.5);content->addSubview(slider);slider->layoutSubviews();}
        else {toggle=new_shared<UISwitch>(NXRect(40,180,63,28));content->addSubview(toggle);toggle->layoutSubviews();}
        control()->addAction(UIAction("Count",[this]{++changes;}),UIControlEvent::valueChanged);
    }
    std::shared_ptr<UIControl> control()const{return slider?std::static_pointer_cast<UIControl>(slider):std::static_pointer_cast<UIControl>(toggle);}
    std::shared_ptr<LensView> lens()const{return slider?UISliderTestHarness::lens(slider):UISwitchTestHarness::lens(toggle);}
    NXKit::NXPoint start(bool track=false)const {
        if(!slider)return {55,194};
        const auto rect=slider->thumbRectForBounds(slider->bounds(),slider->trackRectForBounds(slider->bounds()),slider->value());
        return slider->frame().origin+NXKit::NXPoint(track?40.f:rect.midX(),rect.midY());
    }
    void begin(NXKit::NXPoint point){touch=new_shared<UITouch>(++identifier,point,Timer());send(UITouchPhase::began,point,0);}
    void send(UITouchPhase phase,NXKit::NXPoint point,double time=.05){UIGestureRecognizerTestHarness::send(window,touch,phase,point,time);}
    void end(NXKit::NXPoint point,double time=.3){send(UITouchPhase::ended,point,time);if(slider)UISliderTestHarness::settle(slider);DispatchQueueTestHarness::drain();}
};

void immediateEnvironments() {
    class RejectPan final: public UIGestureRecognizerDelegate {
    public:
        bool gestureRecognizerShouldReceiveTouch(std::shared_ptr<UIGestureRecognizer>,std::shared_ptr<UITouch>) override {return false;}
    };
    for(bool slider:{false,true})for(int variant=0;variant<5;++variant){
        Fixture f(slider,variant!=0);
        auto rejectingDelegate=std::make_shared<RejectPan>();
        if(variant==1)f.scroll->setScrollEnabled(false);
        if(variant==2){f.scroll->setContentSize(f.scroll->bounds().size);f.content->setFrame(f.scroll->bounds());}
        if(variant==3)f.scroll->setDelaysContentTouches(false);
        if(variant==4)f.scroll->panGestureRecognizer()->delegate=rejectingDelegate;
        const auto start=f.start();f.begin(start);
        expect(f.lens()->isLifted(),"standalone/disabled-scroll/non-scrollable/delay-disabled/pan-rejected control lifts immediately");
        expect(slider?f.slider->isTracking():UISwitchTestHarness::admitted(f.toggle),"immediate environment admits actual control tracking");
        f.end(start);
    }
    for(bool slider:{false,true}) {
        Fixture f(slider);const auto start=f.start();f.control()->setEnabled(false);f.begin(start);
        expect(!f.lens()->isLifted()&&!(slider?f.slider->isTracking():UISwitchTestHarness::admitted(f.toggle)),
               "disabled control neither lifts nor admits tracking");
        f.end(start);expect(f.changes==0,"disabled control cannot activate through delayed scroll delivery");
    }
}

void switchDefaultAdmission() {
    Fixture f;const auto start=f.start();f.begin(start);
    expect(!f.lens()->isLifted()&&!UISwitchTestHarness::admitted(f.toggle),"nested switch waits without lift or admitted contact");
    waitForAdmission();
    expect(f.lens()->isLifted()&&UISwitchTestHarness::admitted(f.toggle),"real150ms main-queue timer admits and lifts stationary switch");
    expect(!f.toggle->isTracking()&&!f.toggle->isHighlighted(),"modern switch admission preserves native public tracking/highlight flags");
    f.end(start);expect(f.toggle->isOn()&&f.changes==1,"admitted stationary switch toggles exactly once");
}

void switchScrollAndOwnership() {
    for(const bool horizontal:{false,true}){
        Fixture f(false,true,horizontal);const auto start=f.start();f.begin(start);
        const auto destination=start+(horizontal?NXKit::NXPoint(60,0):NXKit::NXPoint(0,60));
        f.send(UITouchPhase::moved,destination);
        expect(f.scroll->isDragging()&&!f.lens()->isLifted()&&!UISwitchTestHarness::admitted(f.toggle),"fast ancestor-axis swipe scrolls before switch admission without lift");
        f.end(destination);waitForAdmission();
        expect(!f.toggle->isOn()&&f.changes==0&&!f.lens()->isLifted(),"scroll-won switch contact and stale timer never toggle or lift");
    }
    Fixture horizontal;auto start=horizontal.start();horizontal.begin(start);
    horizontal.send(UITouchPhase::moved,start+NXKit::NXPoint(30,0));
    expect(!horizontal.lens()->isLifted()&&!horizontal.scroll->isDragging(),"switch horizontal motion remains pending before150ms in vertical scroll");
    waitForAdmission();expect(horizontal.lens()->isLifted()&&!horizontal.scroll->isDragging(),"horizontal switch contact admits after timer and owns without scrolling");
    horizontal.end(start+NXKit::NXPoint(30,0));expect(horizontal.toggle->isOn()&&horizontal.changes==1,"admitted horizontal switch drag commits once");
    Fixture held;start=held.start();held.begin(start);waitForAdmission();
    held.send(UITouchPhase::moved,start+NXKit::NXPoint(0,60),.2);
    expect(held.lens()->isLifted()&&!held.scroll->isDragging()&&UISwitchTestHarness::admitted(held.toggle),"admitted switch retains ownership of subsequent vertical movement");
    held.end(start+NXKit::NXPoint(0,60));
    // Native reference held-vertical switches on despite release outside its
    // original bounds; this intentionally checks the complete observed result.
    expect(held.toggle->isOn()&&held.changes==1,"native held-vertical switch release commits once");
}

void sliderThumbAndTrack() {
    Fixture held(true);auto start=held.start();held.begin(start);
    expect(held.lens()->isLifted()&&!held.slider->isTracking(),"slider thumb lifts immediately while actual tracking remains delayed");
    waitForAdmission();expect(held.slider->isTracking(),"slider thumb stationary hold admits actual tracking after150ms");held.end(start);
    for(const bool horizontalAncestor:{false,true})for(const bool verticalDrag:{false,true}){
        Fixture f(true,true,horizontalAncestor);start=f.start();f.begin(start);
        const auto destination=start+(verticalDrag?NXKit::NXPoint(0,30):NXKit::NXPoint(30,0));
        f.send(UITouchPhase::moved,destination);
        expect(f.slider->isTracking()&&f.lens()->isLifted()&&!f.scroll->isDragging(),"slider thumb>=10pt pickup captures both axes before ancestor scrolling");
        const auto expectedX=verticalDrag?start.x-f.slider->frame().minX():start.x-f.slider->frame().minX()+10;
        expect(std::abs(UISliderTestHarness::origin(f.slider).x-expectedX)<.01f
               &&std::abs(UISliderTestHarness::origin(f.slider).y-(start.y-f.slider->frame().minY()))<.01f,"slider tracking preserves original down point through delayed begin replay");
        expect(verticalDrag?std::abs(f.slider->value()-.5f)<.001f:f.slider->value()>.5f,"vertical thumb movement preserves value while horizontal movement updates from original origin");
        f.end(destination);
    }
    Fixture track(true);start=track.start(true);track.begin(start);
    expect(!track.lens()->isLifted()&&!track.slider->isTracking(),"slider track contact does not lift thumb or begin tracking");
    track.send(UITouchPhase::moved,start+NXKit::NXPoint(0,50));
    expect(track.scroll->isDragging()&&!track.lens()->isLifted(),"slider track vertical swipe scrolls without thumb lift");
    track.end(start+NXKit::NXPoint(0,50));expect(std::abs(track.slider->value()-.5f)<.001f&&track.changes==0,"slider track scrolling never changes slider value");
    Fixture tap(true);start=tap.start(true);tap.begin(start);tap.end(start,.05);
    expect(!tap.slider->isTracking()&&!tap.lens()->isLifted()&&tap.changes==0&&std::abs(tap.slider->value()-.5f)<.001f,"slider track tap is rejected without value or feedback changes");
    // Continuation regression for the portable responder-admission machinery;
    // this hold-then-track-scroll sequence was not separately captured native.
    Fixture rejectedHold(true);start=rejectedHold.start(true);rejectedHold.begin(start);waitForAdmission();
    expect(!rejectedHold.slider->isTracking()&&!rejectedHold.lens()->isLifted(),
           "stationary slider track contact remains rejected after delayed admission");
    rejectedHold.send(UITouchPhase::moved,start+NXKit::NXPoint(0,50),.2);
    expect(rejectedHold.scroll->isDragging()&&!rejectedHold.lens()->isLifted()&&rejectedHold.changes==0
           &&std::abs(rejectedHold.slider->value()-.5f)<.001f,
           "rejected slider track hold still permits subsequent scrolling without value or feedback changes");
    rejectedHold.end(start+NXKit::NXPoint(0,50));
}

void shortTapStaleTimersAndCallbacks() {
    Fixture tap;auto start=tap.start();tap.begin(start);tap.end(start,.05);
    expect(tap.toggle->isOn()&&tap.changes==1,"tap released while scroll pan possible toggles exactly once");
    waitForAdmission();expect(tap.changes==1&&!UISwitchTestHarness::admitted(tap.toggle),"stale admission timer cannot reactivate completed tap");
    for(bool slider:{false,true})for(int variant=0;variant<2;++variant){
        Fixture f(slider);start=f.start();f.begin(start);
        if(variant==0)f.control()->setEnabled(false);else f.control()->removeFromSuperview();
        waitForAdmission();f.end(start);
        expect(!f.lens()->isLifted()&&f.changes==0,"disable/removal before admission cancels early feedback and stale timers without activation");
    }
    Fixture removed;start=removed.start();
    removed.toggle->addAction(UIAction("Remove",[&]{removed.toggle->removeFromSuperview();}),UIControlEvent::valueChanged);
    removed.begin(start);removed.end(start,.05);
    expect(removed.toggle->isOn()&&removed.changes==1&&removed.toggle->superview().expired(),"value callback may remove switch during delayed tap replay safely");
    Fixture sliderRemoved(true);start=sliderRemoved.start();
    sliderRemoved.slider->addAction(UIAction("Remove",[&]{sliderRemoved.slider->removeFromSuperview();}),UIControlEvent::valueChanged);
    sliderRemoved.begin(start);sliderRemoved.send(UITouchPhase::moved,start+NXKit::NXPoint(30,0));
    expect(sliderRemoved.changes==1&&!sliderRemoved.slider->isTracking()&&!sliderRemoved.lens()->isLifted()
           &&sliderRemoved.slider->superview().expired(),"value callback may remove captured slider during delayed move replay safely");
    sliderRemoved.end(start+NXKit::NXPoint(30,0));
    Fixture shortDrag;start=shortDrag.start();shortDrag.begin(start);
    shortDrag.send(UITouchPhase::moved,start+NXKit::NXPoint(70,0),.07);shortDrag.end(start+NXKit::NXPoint(70,0),.12);
    expect(!shortDrag.toggle->isOn()&&shortDrag.changes==0&&!shortDrag.lens()->isLifted(),"native short fast horizontal switch drag before150ms finishes without lift or value change");
    // Local continuation of the captured short-pan result: returning to the
    // down point cannot turn a completed pending pan back into a tap.
    Fixture returned;start=returned.start();returned.begin(start);
    returned.send(UITouchPhase::moved,start+NXKit::NXPoint(70,0),.05);
    returned.send(UITouchPhase::moved,start,.1);returned.end(start,.12);
    expect(!returned.toggle->isOn()&&returned.changes==0&&!returned.lens()->isLifted(),
           "completed pending switch pan returning to its origin never reactivates as a tap");
}
}

int main(){
    SkiaCtx::_main=std::make_shared<Internal::RasterTestContext>();
    immediateEnvironments();switchDefaultAdmission();switchScrollAndOwnership();sliderThumbAndTrack();shortTapStaleTimersAndCallbacks();
    std::cout<<"Control/scroll arbitration cases="<<cases<<" failures="<<failures<<'\n';return failures?1:0;
}
