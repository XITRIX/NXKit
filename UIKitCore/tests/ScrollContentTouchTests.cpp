#include <UIScrollView.h>
#include <UIControl.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <DispatchQueue.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace NXKit;
namespace NXKit {
class UIGestureRecognizerTestHarness {
public:
    static void send(const std::shared_ptr<UIWindow>& window, const std::shared_ptr<UITouch>& touch,
                     UITouchPhase phase, NXPoint point) {
        touch->_window = window; touch->_phase = phase; touch->_timestamp = Timer(); touch->updateAbsoluteLocation(point);
        auto event = new_shared<UIEvent>(); event->_allTouches.insert(touch); window->sendEvent(event);
    }
};
class DispatchQueueTestHarness {
public:
    static void drain() { DispatchQueue::main()->performAll(); }
};
}
namespace {
int failures = 0;
void expect(bool result, const char* message) { if (!result) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
class Probe final : public UIView {
public:
    int begins = 0, moves = 0, ends = 0, cancels = 0;
    NXPoint initial;
    void touchesBegan(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override { ++begins; initial = (*touches.begin())->absoluteLocation(); }
    void touchesMoved(std::set<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++moves; }
    void touchesEnded(std::set<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++ends; }
    void touchesCancelled(std::set<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++cancels; }
};
class Scroll final : public UIScrollView {
public:
    bool allowsBegin = true, allowsCancel = true;
    int beginQueries = 0, cancelQueries = 0;
    std::function<void()> onBegin;
    bool touchesShouldBegin(std::set<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>, std::shared_ptr<UIView>) override {
        ++beginQueries; if (onBegin) onBegin(); return allowsBegin;
    }
    bool touchesShouldCancelInContentView(std::shared_ptr<UIView>) override { ++cancelQueries; return allowsCancel; }
};
struct Fixture {
    std::shared_ptr<UIWindow> window = new_shared<UIWindow>();
    std::shared_ptr<Scroll> scroll = new_shared<Scroll>();
    std::shared_ptr<Probe> view = new_shared<Probe>();
    std::shared_ptr<UITouch> touch;
    unsigned nextID = 0;
    Fixture() {
        window->setFrame({0, 0, 300, 500}); scroll->setFrame(window->bounds()); scroll->setContentSize({300, 900});
        view->setFrame({20, 40, 200, 100}); window->addSubview(scroll); scroll->addSubview(view);
    }
    void begin() { touch = new_shared<UITouch>(++nextID, NXPoint(50, 70), Timer()); send(UITouchPhase::began); }
    void send(UITouchPhase phase, NXPoint point = {50, 70}) { UIGestureRecognizerTestHarness::send(window, touch, phase, point); }
};
void defaultsAndTap() {
    Fixture f;
    expect(f.scroll->delaysContentTouches() && f.scroll->canCancelContentTouches(), "content touch policy defaults match UIKit");
    f.begin(); expect(f.view->begins == 0 && f.scroll->beginQueries == 0, "scroll defers responder and admission callback while intent is ambiguous");
    f.send(UITouchPhase::ended);
    expect(f.view->begins == 1 && f.view->ends == 1 && f.view->cancels == 0 && f.scroll->beginQueries == 1,
           "short tap releases began and ended together after scroll pan fails");
    Fixture swiped; swiped.begin(); swiped.send(UITouchPhase::moved, {50, 100}); swiped.send(UITouchPhase::moved, {50, 140});
    expect(swiped.scroll->isDragging() && swiped.view->begins == 0 && swiped.view->cancels == 0 && swiped.scroll->beginQueries == 0,
           "quick vertical swipe scrolls without activating or cancelling never-admitted content");
    swiped.send(UITouchPhase::ended, {50, 140});
}
void stationaryHoldAndRetirement() {
    Fixture f; f.begin();
    std::this_thread::sleep_for(std::chrono::milliseconds(175)); DispatchQueueTestHarness::drain();
    expect(f.view->begins == 1 && f.view->initial == NXPoint(50, 70) && f.scroll->beginQueries == 1,
           "finite150ms timer admits stationary contact at original position without new input");
    f.send(UITouchPhase::moved, {50, 100});
    expect(f.scroll->isDragging() && f.view->cancels == 1 && f.scroll->cancelQueries == 1,
           "admitted ordinary view can be cancelled for later scrolling");
    f.send(UITouchPhase::ended, {50, 100});
    Fixture cancelled; cancelled.begin(); cancelled.view->removeFromSuperview();
    std::this_thread::sleep_for(std::chrono::milliseconds(175)); DispatchQueueTestHarness::drain();
    expect(cancelled.view->begins == 0 && cancelled.view->cancels == 0,
           "expired timer cannot revive a removed pending responder");
    cancelled.send(UITouchPhase::ended);
}
void flagsAndCancellationHooks() {
    Fixture immediate; immediate.scroll->setDelaysContentTouches(false); immediate.begin();
    expect(immediate.view->begins == 1 && immediate.scroll->beginQueries == 1, "disabling delay admits content immediately");
    immediate.scroll->setCanCancelContentTouches(false); immediate.send(UITouchPhase::moved, {50, 100});
    expect(!immediate.scroll->isDragging() && immediate.view->moves == 1 && immediate.view->cancels == 0 && immediate.scroll->cancelQueries == 0,
           "canCancel false preserves admitted content without consulting cancellation override");
    immediate.send(UITouchPhase::ended, {50, 100});
    Fixture pending; pending.scroll->setCanCancelContentTouches(false); pending.begin(); pending.send(UITouchPhase::moved, {50, 100});
    expect(pending.scroll->isDragging() && pending.view->begins == 0, "canCancel false does not prevent scrolling before content admission");
    pending.send(UITouchPhase::ended, {50, 100});
    Fixture veto; veto.scroll->setDelaysContentTouches(false); veto.scroll->allowsCancel = false; veto.begin(); veto.send(UITouchPhase::moved, {50, 100});
    expect(!veto.scroll->isDragging() && veto.view->cancels == 0 && veto.view->moves == 1 && veto.scroll->cancelQueries == 1,
           "touchesShouldCancel veto keeps content responder and fails scrolling"); veto.send(UITouchPhase::ended, {50, 100});
    Fixture toggled; toggled.begin(); toggled.scroll->setDelaysContentTouches(false);
    expect(toggled.view->begins == 1, "turning delay off releases already-buffered content immediately"); toggled.send(UITouchPhase::ended);
    Fixture disabled; disabled.scroll->setScrollEnabled(false); disabled.begin();
    expect(disabled.view->begins == 1, "disabled scrolling never delays content admission"); disabled.send(UITouchPhase::ended);
}
void beginVetoAndPublicDefaults() {
    Fixture f; f.scroll->allowsBegin = false; f.begin(); f.send(UITouchPhase::ended);
    expect(f.view->begins == 0 && f.view->ends == 0 && f.scroll->beginQueries == 1, "touchesShouldBegin veto suppresses entire content sequence");
    auto defaults = new_shared<UIScrollView>(); auto view = new_shared<UIView>(); auto control = new_shared<UIControl>();
    expect(defaults->touchesShouldBegin({}, nullptr, view) && defaults->touchesShouldCancelInContentView(view)
               && !defaults->touchesShouldCancelInContentView(control), "public default hooks permit generic view cancellation and protect UIControl");
    expect(defaults->applyXMLAttribute("delaysContentTouches", "false") && !defaults->delaysContentTouches()
               && defaults->applyXMLAttribute("canCancelContentTouches", "false") && !defaults->canCancelContentTouches(),
           "XML exposes actual touch arbitration policies");
}
void noCompetingPanNeedsNoDelay() {
    Fixture f; f.scroll->setContentSize(f.scroll->bounds().size); f.begin();
    expect(f.view->begins == 1, "non-scrollable no-bounce ancestor never delays content"); f.send(UITouchPhase::ended);
    Fixture bounced; bounced.scroll->setContentSize(bounced.scroll->bounds().size); bounced.scroll->setBounceVertically(true); bounced.begin();
    expect(bounced.view->begins == 0, "bounce-enabled pan still creates genuine ambiguity without content overflow"); bounced.send(UITouchPhase::ended);
    Fixture insets; insets.scroll->setContentSize(insets.scroll->bounds().size); insets.scroll->setContentInset({10, 0, 10, 0}); insets.begin();
    expect(insets.view->begins == 0, "live content inset capacity counts as scrollable content"); insets.send(UITouchPhase::ended);
    Fixture panDisabled; panDisabled.scroll->panGestureRecognizer()->setEnabled(false); panDisabled.begin();
    expect(panDisabled.view->begins == 1, "disabled pan recognizer creates no pickup delay"); panDisabled.send(UITouchPhase::ended);
}
}
int main() {
    defaultsAndTap(); stationaryHoldAndRetirement(); flagsAndCancellationHooks(); beginVetoAndPublicDefaults();
    noCompetingPanNeedsNoDelay();
    std::cout << "Scroll content touch regression failures: " << failures << '\n'; return failures ? 1 : 0;
}
