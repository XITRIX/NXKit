#include <UIEvent.h>
#include <UIGestureRecognizer.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <UIScrollView.h>

#include <functional>
#include <iostream>
#include <string>

using namespace NXKit;

namespace NXKit {
static Timer deliveryTestTime(double seconds) {
    Timer value; value.startTime.tv_sec=1000+static_cast<long>(seconds);
    value.startTime.tv_usec=static_cast<long>((seconds-static_cast<long>(seconds))*1000000);
    return value;
}
class UIGestureRecognizerTestHarness {
public:
    static void send(const std::shared_ptr<UIWindow>& window,
                     const std::shared_ptr<UITouch>& touch, UITouchPhase phase,
                     NXPoint point, double time, const std::shared_ptr<UITouch>& other = nullptr) {
        touch->_window = window;
        touch->_phase = phase;
        touch->_timestamp = deliveryTestTime(time);
        touch->updateAbsoluteLocation(point);
        auto event = new_shared<UIEvent>();
        event->_timestamp = touch->_timestamp;
        event->_allTouches.insert(touch);
        if (other) event->_allTouches.insert(other);
        window->sendEvent(event);
    }
};
}

namespace {
int failures = 0;
void expect(bool condition, const std::string& message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

class DeliveryProbe final: public UIView {
public:
    struct Sample {
        UITouchPhase phase;
        NXPoint point;
        Timer time;
        std::shared_ptr<UITouch> touch;
        std::size_t eventTouches;
        bool matchesEvent;
    };
    std::vector<Sample> samples;
    std::function<void(UITouchPhase)> callback;
    void record(UITouchPhase phase, const std::set<std::shared_ptr<UITouch>>& touches,
                const std::shared_ptr<UIEvent>& event) {
        for (const auto& touch: touches) {
            samples.push_back({phase, touch->absoluteLocation(), touch->timestamp(), touch,
                               event->allTouches().size(), event->allTouches().contains(touch)
                                   && touch->phase() == phase && event->timestamp() - touch->timestamp() == 0});
        }
        if (callback) callback(phase);
    }
    void touchesBegan(std::set<std::shared_ptr<UITouch>> t, std::shared_ptr<UIEvent> e) override { record(UITouchPhase::began,t,e); }
    void touchesMoved(std::set<std::shared_ptr<UITouch>> t, std::shared_ptr<UIEvent> e) override { record(UITouchPhase::moved,t,e); }
    void touchesEnded(std::set<std::shared_ptr<UITouch>> t, std::shared_ptr<UIEvent> e) override { record(UITouchPhase::ended,t,e); }
    void touchesCancelled(std::set<std::shared_ptr<UITouch>> t, std::shared_ptr<UIEvent> e) override { record(UITouchPhase::cancelled,t,e); }
};

struct Fixture {
    std::shared_ptr<UIWindow> window = new_shared<UIWindow>();
    std::shared_ptr<DeliveryProbe> view = new_shared<DeliveryProbe>();
    std::shared_ptr<UIGestureRecognizer> recognizer = new_shared<UIGestureRecognizer>();
    std::shared_ptr<UITouch> touch = new_shared<UITouch>(1,NXPoint(20,20),deliveryTestTime(1));
    Fixture() {
        window->setFrame({0,0,320,480}); view->setFrame(window->bounds());
        window->addSubview(view); view->addGestureRecognizer(recognizer);
    }
    void send(UITouchPhase phase, NXPoint point = {20,20}, double time = 1) {
        UIGestureRecognizerTestHarness::send(window,touch,phase,point,time);
    }
};

void defaultsAndImmediateDelivery() {
    Fixture f;
    expect(!f.recognizer->delaysTouchesBegan() && f.recognizer->delaysTouchesEnded(),
           "recognizer touch delay defaults follow UIKit headers");
    f.send(UITouchPhase::began); f.send(UITouchPhase::moved,{25,20},2); f.send(UITouchPhase::ended,{25,20},3);
    expect(f.view->samples.size()==3,"ordinary failed recognizer delivers begin, move, end");
}

void delayedBeginSnapshotAndTimerRelease() {
    Fixture f; f.recognizer->setDelaysTouchesBegan(true);
    f.send(UITouchPhase::began,{20,20},1);
    f.send(UITouchPhase::moved,{45,30},2);
    expect(f.view->samples.empty(),"possible recognizer buffers begin and following moves");
    // This is outside sendEvent, as with a scroll-view's delayed-touch timer.
    f.recognizer->setState(UIGestureRecognizerState::failed);
    expect(f.view->samples.size()==2,"timer-driven failure immediately flushes buffered touch phases");
    if (f.view->samples.size()==2) {
        const auto& begin=f.view->samples[0]; const auto& move=f.view->samples[1];
        expect(begin.point==NXPoint(20,20) && begin.time-deliveryTestTime(1)==0
               && move.point==NXPoint(45,30) && move.time-deliveryTestTime(2)==0,"delayed delivery preserves original point and timestamp");
        expect(begin.touch==f.touch && move.touch==f.touch && begin.matchesEvent && move.matchesEvent,
               "delayed events preserve contact identity and matching allTouches");
    }
    expect(f.touch->phase()==UITouchPhase::moved && f.touch->absoluteLocation()==NXPoint(45,30)
           && f.touch->timestamp()-deliveryTestTime(2)==0,"snapshot replay restores current raw touch state");
    f.send(UITouchPhase::ended,{45,30},3);
    expect(f.view->samples.size()==3,"released begin continues ordinary delivery");
}

void multipleDelayersAndPropertyChanges() {
    Fixture f;
    auto other = new_shared<UIGestureRecognizer>();
    f.view->addGestureRecognizer(other);
    f.recognizer->setDelaysTouchesBegan(true); other->setDelaysTouchesBegan(true);
    f.send(UITouchPhase::began);
    f.recognizer->setState(UIGestureRecognizerState::failed);
    expect(f.view->samples.empty(),"all delaying recognizers must resolve before responder admission");
    other->setDelaysTouchesBegan(false);
    expect(f.view->samples.size()==1,"disabling the last delay flag releases contact immediately");
    f.send(UITouchPhase::ended,{20,20},2);
    expect(f.view->samples.size()==2,"property change does not duplicate buffered begin or end");
}

void reentrantReleaseOfAnotherContact() {
    Fixture f; f.view->setFrame({0,0,150,480}); f.recognizer->setDelaysTouchesBegan(true);
    auto otherView = new_shared<DeliveryProbe>(); otherView->setFrame({160,0,150,480});
    auto otherRecognizer = new_shared<UIGestureRecognizer>(); otherRecognizer->setDelaysTouchesBegan(true);
    otherView->addGestureRecognizer(otherRecognizer); f.window->addSubview(otherView);
    auto otherTouch = new_shared<UITouch>(2,NXPoint(180,20),deliveryTestTime(2));
    f.send(UITouchPhase::began);
    UIGestureRecognizerTestHarness::send(f.window,otherTouch,UITouchPhase::began,{180,20},2);
    otherView->callback = [&](auto phase) {
        if(phase==UITouchPhase::began) f.recognizer->setState(UIGestureRecognizerState::failed);
    };
    otherRecognizer->setDelaysTouchesBegan(false);
    expect(f.view->samples.size()==1 && otherView->samples.size()==1,
           "responder callback releasing another buffered contact flushes both without another input event");
    f.send(UITouchPhase::ended,{20,20},3);
    UIGestureRecognizerTestHarness::send(f.window,otherTouch,UITouchPhase::ended,{180,20},4);
}

void cancellationAndRecognitionOrdering() {
    for (const bool delayed: {false,true}) {
        Fixture f; f.recognizer->setDelaysTouchesBegan(delayed);
        f.send(UITouchPhase::began);
        std::size_t samplesAtAction=100;
        f.recognizer->onStateChanged=[&](auto recognizer) {
            if (recognizer->state()==UIGestureRecognizerState::began) samplesAtAction=f.view->samples.size();
        };
        f.recognizer->setState(UIGestureRecognizerState::began);
        f.recognizer->setState(UIGestureRecognizerState::changed);
        f.send(UITouchPhase::moved,{40,20},2); f.send(UITouchPhase::ended,{40,20},3);
        expect(f.view->samples.size()==(delayed?0:2),"recognition suppresses undelivered begin or cancels delivered begin once");
        expect(samplesAtAction==(delayed?0:2),"recognizer cancellation precedes its public action callback");
        if (!delayed && f.view->samples.size()==2)
            expect(f.view->samples[1].phase==UITouchPhase::cancelled && f.view->samples[1].matchesEvent,
                   "cancellation is represented consistently in callback and event touch");
    }
    Fixture f; f.recognizer->setDelaysTouchesBegan(true); f.send(UITouchPhase::began);
    f.send(UITouchPhase::cancelled,{25,25},2);
    expect(f.view->samples.empty(),"system cancellation suppresses a never-delivered contact");
}

void nonCancellingRecognitionAndAdmission() {
    Fixture f; f.recognizer->setDelaysTouchesBegan(true); f.recognizer->setCancelsTouchesInView(false);
    f.send(UITouchPhase::began); f.recognizer->setState(UIGestureRecognizerState::began);
    expect(f.view->samples.size()==1,"non-cancelling recognition releases a delayed begin");
    f.send(UITouchPhase::ended,{20,20},2);
    expect(f.view->samples.size()==2,"non-cancelling recognition preserves responder end");
    struct Reject: UIGestureRecognizerDelegate {
        bool gestureRecognizerShouldReceiveTouch(std::shared_ptr<UIGestureRecognizer>,std::shared_ptr<UITouch>) override {return false;}
    };
    Fixture rejected; auto delegate=std::make_shared<Reject>(); rejected.recognizer->delegate=delegate;
    rejected.recognizer->setDelaysTouchesBegan(true); rejected.send(UITouchPhase::began);
    expect(rejected.view->samples.size()==1,"a recognizer rejecting the contact cannot delay it");
}

void delayedEndAndMultipleContacts() {
    for (const bool delayEnd: {true,false}) {
        Fixture f; f.recognizer->setDelaysTouchesEnded(delayEnd);
        auto second=new_shared<UITouch>(2,NXPoint(80,20),deliveryTestTime(2));
        f.send(UITouchPhase::began);
        UIGestureRecognizerTestHarness::send(f.window,second,UITouchPhase::began,{80,20},2,f.touch);
        const auto before=f.view->samples.size();
        f.send(UITouchPhase::ended,{20,20},3);
        expect(f.view->samples.size()==before+(delayEnd?0:1),"ended delay waits while recognizer still tracks another contact");
        f.recognizer->setState(UIGestureRecognizerState::failed);
        expect(f.view->samples.size()==before+1,"timer failure releases an ended contact exactly once");
        UIGestureRecognizerTestHarness::send(f.window,second,UITouchPhase::ended,{80,20},4);
    }
}

void unavailableAndReentrantTargets() {
    for (int variant=0; variant<5; ++variant) {
        Fixture f; f.recognizer->setDelaysTouchesBegan(true); f.send(UITouchPhase::began);
        if(variant==0) f.view->removeFromSuperview();
        if(variant==1) f.view->setHidden(true);
        if(variant==2) f.view->setUserInteractionEnabled(false);
        if(variant==3) f.view->setAlpha(0);
        if(variant==4) { f.view->removeFromSuperview(); f.window->addSubview(f.view); }
        f.recognizer->setState(UIGestureRecognizerState::failed);
        f.send(UITouchPhase::ended,{20,20},3);
        expect(f.view->samples.empty(),"removal/hidden/disabled/transparent/reparented target suppresses delayed contact");
    }
    Fixture f; f.recognizer->setDelaysTouchesBegan(true);
    f.send(UITouchPhase::began); f.send(UITouchPhase::moved,{40,20},2);
    f.view->callback=[&](auto phase) { if(phase==UITouchPhase::began) f.view->removeFromSuperview(); };
    f.recognizer->setState(UIGestureRecognizerState::failed);
    expect(f.view->samples.size()<=2 && f.view->samples.front().phase==UITouchPhase::began,
           "delayed begin callback may remove target safely");
    for(const auto& sample:f.view->samples)
        expect(sample.phase!=UITouchPhase::moved && sample.phase!=UITouchPhase::ended,
               "reentrant target removal prevents buffered move/end delivery");
    Fixture removed; removed.recognizer->setDelaysTouchesBegan(true); removed.send(UITouchPhase::began);
    std::weak_ptr<UITouch> lifetime = removed.touch;
    removed.view->removeFromSuperview(); removed.view.reset();
    removed.send(UITouchPhase::ended,{20,20},2); removed.touch.reset();
    expect(lifetime.expired(),"ended contact releases its buffer even after target deallocation");
}

void scrollAdmission() {
    class Gate final: public UIScrollView {
    public:
        bool accepts = true;
        int calls = 0;
        std::function<void()> callback;
        bool touchesShouldBegin(std::set<std::shared_ptr<UITouch>> touches,
                                std::shared_ptr<UIEvent> event, std::shared_ptr<UIView> target) override {
            ++calls;
            expect(touches.size()==1 && event->allTouches().contains(*touches.begin()) && target,
                   "scroll admission receives original contact, event and content target");
            if(callback) callback();
            return accepts;
        }
    };
    for(int variant=0; variant<3; ++variant) {
        Fixture f; auto scroll = new_shared<Gate>(); scroll->setFrame(f.window->bounds());
        scroll->setDelaysContentTouches(false); f.window->addSubview(scroll); scroll->addSubview(f.view);
        f.recognizer->setDelaysTouchesBegan(true);
        scroll->accepts = variant != 1;
        if(variant==2) scroll->callback=[&]{f.view->removeFromSuperview();};
        f.send(UITouchPhase::began); f.send(UITouchPhase::moved,{21,20},2);
        expect(scroll->calls==0,"scroll admission callback waits for actual responder begin");
        f.recognizer->setState(UIGestureRecognizerState::failed);
        expect(scroll->calls==1,"scroll admission callback executes once per contact");
        expect(f.view->samples.size()==(variant==0?2:0),"scroll admission veto or reentrant removal suppresses queued delivery");
        f.send(UITouchPhase::ended,{21,20},3);
        expect(scroll->calls==1,"terminal delivery does not repeat scroll admission");
    }
}
}

int main() {
    defaultsAndImmediateDelivery(); delayedBeginSnapshotAndTimerRelease(); multipleDelayersAndPropertyChanges();
    reentrantReleaseOfAnotherContact();
    cancellationAndRecognitionOrdering(); nonCancellingRecognitionAndAdmission();
    delayedEndAndMultipleContacts(); unavailableAndReentrantTargets(); scrollAdmission();
    if(!failures) std::cout << "Touch delivery tests passed\n";
    return failures?1:0;
}
