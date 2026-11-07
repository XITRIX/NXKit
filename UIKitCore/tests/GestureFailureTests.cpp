#include <UIGestureRecognizer.h>
#include <UIPanGestureRecognizer.h>
#include <UITapGestureRecognizer.h>
#include <UITouch.h>
#include <UIEvent.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIScrollView.h>
#include <UIWindow.h>

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <vector>

using namespace NXKit;
namespace NXKit {
class UIGestureRecognizerTestHarness {
public:
    static void touch(const std::shared_ptr<UIWindow>& window, const std::shared_ptr<UITouch>& touch,
                      UITouchPhase phase, NXPoint point) {
        touch->_window = window; touch->_phase = phase; touch->updateAbsoluteLocation(point);
        auto event = new_shared<UIEvent>(); event->_allTouches.insert(touch); window->sendEvent(event);
    }
    static void press(const std::shared_ptr<UIWindow>& window, const std::shared_ptr<UIView>& view,
                      const std::shared_ptr<UIPress>& press, UIPressPhase phase) {
        press->_window = window; press->_responder = view; press->_phase = phase; press->_type = UIPressType::select;
        auto event = std::shared_ptr<UIPressesEvent>(new UIPressesEvent(press)); window->sendEvent(event);
    }
    static void endTogether(const std::shared_ptr<UIWindow>& window, const std::vector<std::shared_ptr<UITouch>>& touches) {
        auto event = new_shared<UIEvent>();
        for (const auto& touch : touches) { touch->_phase = UITouchPhase::ended; event->_allTouches.insert(touch); }
        window->sendEvent(event);
    }
};
}
namespace {
int failures = 0;
void expect(bool result, const char* message) { if (!result) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
struct Fixture {
    std::shared_ptr<UIWindow> window = new_shared<UIWindow>();
    std::shared_ptr<UIScrollView> scroll = new_shared<UIScrollView>();
    std::shared_ptr<UIView> child = new_shared<UIView>();
    std::shared_ptr<UITouch> contact;
    unsigned nextID = 0;
    Fixture() {
        window->setFrame({0, 0, 300, 500}); scroll->setFrame(window->bounds()); scroll->setContentSize({300, 900});
        child->setFrame({20, 40, 200, 100}); window->addSubview(scroll); scroll->addSubview(child);
    }
    void begin() { contact = new_shared<UITouch>(++nextID, NXPoint(50, 70), Timer()); send(UITouchPhase::began); }
    void send(UITouchPhase phase, NXPoint point = {50, 70}) { UIGestureRecognizerTestHarness::touch(window, contact, phase, point); }
};
struct Delegate final : UIGestureRecognizerDelegate {
    bool touchAllowed = true, pressAllowed = true, eventAllowed = true, simultaneous = false, beginAllowed = true;
    bool requiresOther = false, requiredByOther = false;
    int touches = 0, presses = 0, events = 0, begins = 0, requirements = 0, counterparts = 0;
    std::unordered_set<const UIGestureRecognizer*> requirementPeers;
    bool gestureRecognizerShouldBegin(std::shared_ptr<UIGestureRecognizer>) override { ++begins; return beginAllowed; }
    bool gestureRecognizerShouldReceiveTouch(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UITouch>) override { ++touches; return touchAllowed; }
    bool gestureRecognizerShouldReceivePress(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIPress>) override { ++presses; return pressAllowed; }
    bool gestureRecognizerShouldReceiveEvent(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIEvent>) override { ++events; return eventAllowed; }
    bool gestureRecognizerShouldRequireFailureOf(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIGestureRecognizer> other) override { ++requirements; requirementPeers.insert(other.get()); return requiresOther; }
    bool gestureRecognizerShouldBeRequiredToFailBy(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIGestureRecognizer>) override { ++counterparts; return requiredByOther; }
    bool gestureRecognizerShouldRecognizeSimultaneouslyWith(std::shared_ptr<UIGestureRecognizer>, std::shared_ptr<UIGestureRecognizer>) override { return simultaneous; }
};
struct Eager final : UIGestureRecognizer {
    int began = 0, moves = 0, ended = 0, resets = 0;
    bool prevents = true, preventable = true;
    bool canPrevent(const std::shared_ptr<UIGestureRecognizer>&) const override { return prevents; }
    bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const override { return preventable; }
    void touchesBegan(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++began; setState(UIGestureRecognizerState::began); }
    void touchesMoved(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++moves; if (state() == UIGestureRecognizerState::began) setState(UIGestureRecognizerState::changed); }
    void touchesEnded(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++ended; setState(UIGestureRecognizerState::ended); }
    void reset() override { ++resets; }
};
struct EndTap final : UIGestureRecognizer {
    int receives = 0;
    void touchesBegan(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { ++receives; }
    void touchesEnded(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) override { setState(UIGestureRecognizerState::ended); }
};
struct PressTap final : UIGestureRecognizer {
    int receives = 0;
    void pressesBegan(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) override { ++receives; }
    void pressesEnded(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) override { setState(UIGestureRecognizerState::ended); }
};
int& observe(const std::shared_ptr<UIGestureRecognizer>& recognizer, int& actions) {
    recognizer->onStateChanged = [&actions](const auto& value) { if (value->state() == UIGestureRecognizerState::ended) ++actions; }; return actions;
}
void childTapWaitsForPan() {
    Fixture f; auto tap = new_shared<UITapGestureRecognizer>(); int taps = 0; observe(tap, taps);
    tap->requireGestureRecognizerToFail(f.scroll->panGestureRecognizer()); f.child->addGestureRecognizer(tap);
    f.begin(); expect(taps == 0, "child tap remains possible before release"); f.send(UITouchPhase::ended);
    expect(taps == 1, "child tap recognizes after later-dispatched ancestor pan fails on release");
    expect(tap->state() == UIGestureRecognizerState::possible && f.scroll->panGestureRecognizer()->state() == UIGestureRecognizerState::possible,
           "both recognizers reset only after pending recognition resolved");
    f.begin(); f.send(UITouchPhase::moved, {50, 100}); f.send(UITouchPhase::moved, {50, 140});
    expect(f.scroll->isDragging(), "vertical ancestor pan still wins before tap recognition"); f.send(UITouchPhase::ended, {50, 140});
    expect(taps == 1, "recognized scroll suppresses pending child tap");
    f.begin(); f.send(UITouchPhase::ended); expect(taps == 2, "failure requirements survive reset and support repeated contacts");
}
void continuousRequestsAndRemoval() {
    Fixture f; auto control = new_shared<Eager>(); control->requireGestureRecognizerToFail(f.scroll->panGestureRecognizer());
    int begins = 0; control->onStateChanged = [&](const auto& value) { if (value->state() == UIGestureRecognizerState::began) ++begins; };
    f.child->addGestureRecognizer(control); f.begin();
    expect(control->state() == UIGestureRecognizerState::possible && begins == 0, "continuous recognition request waits without delivering began");
    f.send(UITouchPhase::moved, {80, 70});
    expect(control->state() == UIGestureRecognizerState::began && begins == 1, "horizontal rejection of vertical scroll releases continuous recognition");
    f.send(UITouchPhase::ended, {80, 70});
    f.begin(); f.scroll->removeGestureRecognizer(f.scroll->panGestureRecognizer());
    expect(control->state() == UIGestureRecognizerState::began && begins == 2, "removing a pending prerequisite releases dependent without stale input");
    f.send(UITouchPhase::cancelled);
    expect(control->state() == UIGestureRecognizerState::possible, "cancelled pending control resets");
    f.scroll->addGestureRecognizer(f.scroll->panGestureRecognizer()); f.begin();
    f.child->removeFromSuperview();
    expect(begins == 2 && control->state() == UIGestureRecognizerState::possible,
           "removal cancels an unrecognized dependent without briefly recognizing it");
    f.scroll->addSubview(f.child); f.send(UITouchPhase::ended);
    expect(begins == 2, "reparenting does not revive an old pending touch");
    f.begin(); f.send(UITouchPhase::moved, {80, 70}); expect(begins == 3, "reparented recognizer accepts next contact"); f.send(UITouchPhase::ended, {80, 70});
}
void validationAndWeakOwnership() {
    auto a = new_shared<EndTap>(); auto b = new_shared<EndTap>(); auto c = new_shared<EndTap>();
    a->requireGestureRecognizerToFail(b); b->requireGestureRecognizerToFail(c);
    bool cycle = false, self = false, null = false;
    try { c->requireGestureRecognizerToFail(a); } catch (const std::invalid_argument&) { cycle = true; }
    try { a->requireGestureRecognizerToFail(a); } catch (const std::invalid_argument&) { self = true; }
    try { a->requireGestureRecognizerToFail(nullptr); } catch (const std::invalid_argument&) { null = true; }
    expect(cycle && self && null, "failure requirements reject null, self, and transitive cycles");
    std::weak_ptr<UIGestureRecognizer> weak = c; c.reset(); expect(weak.expired(), "requirements never retain other recognizers");
}
void dynamicRequirements() {
    for (bool counterpart : {false, true}) {
        Fixture f; auto tap = new_shared<EndTap>(); int taps = 0; observe(tap, taps);
        auto delegate = std::make_shared<Delegate>();
        if (counterpart) { delegate->requiredByOther = true; f.scroll->panGestureRecognizer()->delegate = delegate; }
        else { delegate->requiresOther = true; tap->delegate = delegate; }
        f.child->addGestureRecognizer(tap); f.begin(); f.send(UITouchPhase::ended);
        expect(taps == 1, "either delegate counterpart can establish a dynamic failure requirement");
        expect(counterpart ? delegate->counterparts == 1 : delegate->requirements > 0 && delegate->requirements == delegate->requirementPeers.size(),
               "dynamic failure delegate is consulted once per attempt and recognizer pair");
        f.begin(); f.send(UITouchPhase::ended); expect(taps == 2, "dynamic requirements are rebuilt on next attempt");
    }
}
void filtering() {
    Fixture f; auto tap = new_shared<EndTap>(); auto delegate = std::make_shared<Delegate>(); tap->delegate = delegate;
    f.child->addGestureRecognizer(tap); int taps = 0; observe(tap, taps);
    delegate->touchAllowed = false; f.begin(); f.send(UITouchPhase::ended);
    expect(tap->receives == 0 && taps == 0 && delegate->touches == 1, "touch filter rejects whole contact before subclass receives it");
    delegate->touchAllowed = true; delegate->eventAllowed = false; f.begin(); f.send(UITouchPhase::ended);
    expect(tap->receives == 0 && delegate->touches == 1 && delegate->events == 2, "event rejection skips per-touch admission");
    delegate->eventAllowed = true; f.begin(); f.send(UITouchPhase::ended);
    expect(tap->receives == 1 && taps == 1, "filtered recognizer remains usable on a later accepted contact");
    delegate->beginAllowed = false; f.begin(); f.send(UITouchPhase::ended); expect(taps == 1, "shouldBegin rejection still suppresses recognized action");
    Fixture filteredDependency; auto eager = new_shared<Eager>(); auto rejection = std::make_shared<Delegate>(); rejection->touchAllowed = false;
    filteredDependency.scroll->panGestureRecognizer()->delegate = rejection;
    eager->requireGestureRecognizerToFail(filteredDependency.scroll->panGestureRecognizer()); filteredDependency.child->addGestureRecognizer(eager);
    filteredDependency.begin(); expect(eager->state() == UIGestureRecognizerState::began,
           "dependency rejected by touch admission does not block accepted recognizer"); filteredDependency.send(UITouchPhase::ended);
}
void simultaneousAndPrevention() {
    for (int policy = 0; policy < 3; ++policy) {
        Fixture f; f.scroll->setScrollEnabled(false);
        auto a = new_shared<Eager>(); auto b = new_shared<Eager>(); auto delegate = std::make_shared<Delegate>();
        if (policy == 0) { delegate->simultaneous = true; b->delegate = delegate; }
        if (policy == 1) a->prevents = false;
        if (policy == 2) b->preventable = false;
        // A later possible recognizer must not cancel an already recognized
        // peer if that peer cannot be prevented by it either.
        if (policy == 1) a->preventable = false;
        if (policy == 2) b->prevents = false;
        f.child->addGestureRecognizer(a); f.child->addGestureRecognizer(b); f.begin();
        expect(a->state() == UIGestureRecognizerState::began && b->state() == UIGestureRecognizerState::began,
               "simultaneous and explicit prevention overrides remain effective"); f.send(UITouchPhase::ended);
    }
    Fixture f; f.scroll->setScrollEnabled(false); auto a = new_shared<EndTap>(); auto b = new_shared<EndTap>(); auto d = std::make_shared<Delegate>(); d->simultaneous = true;
    a->delegate = d; b->delegate = d; a->requireGestureRecognizerToFail(b); f.child->addGestureRecognizer(a); f.child->addGestureRecognizer(b);
    int first = 0, second = 0; observe(a, first); observe(b, second); f.begin(); f.send(UITouchPhase::ended);
    expect(first == 0 && second == 1, "failure requirement takes precedence over simultaneous recognition permission");
}
void presses() {
    Fixture f; auto tap = new_shared<PressTap>(); auto d = std::make_shared<Delegate>(); tap->delegate = d; f.child->addGestureRecognizer(tap); int taps = 0; observe(tap, taps);
    const auto send = [&](bool allowed) {
        d->pressAllowed = allowed; auto press = new_shared<UIPress>();
        UIGestureRecognizerTestHarness::press(f.window, f.child, press, UIPressPhase::began);
        UIGestureRecognizerTestHarness::press(f.window, f.child, press, UIPressPhase::ended);
    };
    send(false); expect(taps == 0 && tap->receives == 0 && d->presses == 1, "press admission is applied through actual UIWindow dispatch");
    send(true); expect(taps == 1 && tap->receives == 1 && d->presses == 2, "accepted focused press reaches recognizer and completes");
}
void cancellationAndReentrancy() {
    Fixture f; auto eager = new_shared<Eager>(); eager->requireGestureRecognizerToFail(f.scroll->panGestureRecognizer());
    int begins = 0; eager->onStateChanged = [&](const auto& value) { if (value->state() == UIGestureRecognizerState::began) ++begins; };
    f.child->addGestureRecognizer(eager); f.begin(); eager->setEnabled(false); f.send(UITouchPhase::ended);
    expect(begins == 0 && eager->state() == UIGestureRecognizerState::possible, "disabling cancels pending request without later recognition");
    eager->setEnabled(true); f.begin(); f.send(UITouchPhase::moved, {80, 70}); f.send(UITouchPhase::ended, {80, 70});
    expect(begins == 1, "re-enabled recognizer accepts next contact with fresh dependency state");

    Fixture reentrant; auto recognizer = new_shared<Eager>(); auto delegate = std::make_shared<Delegate>();
    recognizer->delegate = delegate; reentrant.child->addGestureRecognizer(recognizer);
    recognizer->onStateChanged = [&](const auto& value) {
        if (value->state() == UIGestureRecognizerState::began) reentrant.child->removeGestureRecognizer(value);
    };
    reentrant.begin(); reentrant.send(UITouchPhase::ended);
    expect(recognizer->view().expired() && recognizer->state() == UIGestureRecognizerState::possible,
           "recognition callback may remove recognizer without stale state or active input");

    Fixture filtered; auto tap = new_shared<EndTap>(); auto filter = std::make_shared<Delegate>(); filter->touchAllowed = false;
    tap->delegate = filter; filtered.child->addGestureRecognizer(tap); int taps = 0; observe(tap, taps);
    filtered.begin(); filtered.send(UITouchPhase::moved, {50, 100}); filtered.send(UITouchPhase::ended, {50, 100});
    filter->touchAllowed = true; filtered.begin(); filtered.send(UITouchPhase::ended);
    expect(taps == 1, "competing scroll cannot permanently fail a recognizer that rejected the original contact");
}
void simultaneousTerminalContacts() {
    Fixture f; f.scroll->setScrollEnabled(false); auto recognizer = new_shared<Eager>(); f.child->addGestureRecognizer(recognizer);
    auto first = new_shared<UITouch>(100, NXPoint(50, 70), Timer());
    auto second = new_shared<UITouch>(101, NXPoint(70, 70), Timer());
    UIGestureRecognizerTestHarness::touch(f.window, first, UITouchPhase::began, {50, 70});
    UIGestureRecognizerTestHarness::touch(f.window, second, UITouchPhase::began, {70, 70});
    UIGestureRecognizerTestHarness::endTogether(f.window, {first, second});
    expect(recognizer->resets == 1 && recognizer->state() == UIGestureRecognizerState::possible,
           "contacts marked ended together reset only after every terminal delivery");
}
void reentrantAttachmentHasOneOwner() {
    for (const bool sameDestination : {false, true}) {
        Fixture f; auto recognizer = new_shared<Eager>(); f.child->addGestureRecognizer(recognizer);
        auto requested = new_shared<UIView>(); auto nested = sameDestination ? requested : new_shared<UIView>();
        requested->setFrame({0, 200, 300, 100}); f.window->addSubview(requested);
        if (!sameDestination) { nested->setFrame({0, 350, 300, 100}); f.window->addSubview(nested); }
        f.begin();
        recognizer->onStateChanged = [&](const auto& value) {
            if (value->state() == UIGestureRecognizerState::cancelled) nested->addGestureRecognizer(value);
        };
        requested->addGestureRecognizer(recognizer);
        const auto count = [&](const auto& view) { return std::count(view->gestureRecognizers()->begin(), view->gestureRecognizers()->end(), recognizer); };
        expect(recognizer->view().lock() == nested && count(f.child) == 0 && count(nested) == 1
                   && (sameDestination || count(requested) == 0),
               "reentrant cancellation attachment wins and never duplicates recognizer ownership");
        recognizer->onStateChanged = [](const auto&) {};
        f.send(UITouchPhase::ended);
        auto next = new_shared<UITouch>(200, NXPoint(50, sameDestination ? 230 : 380), Timer());
        UIGestureRecognizerTestHarness::touch(f.window, next, UITouchPhase::began, next->absoluteLocation());
        expect(recognizer->state() == UIGestureRecognizerState::began, "reentrantly attached recognizer can process a fresh contact");
        UIGestureRecognizerTestHarness::touch(f.window, next, UITouchPhase::ended, next->absoluteLocation());
    }
}
void throwingCancellationClosesDispatchScope() {
    Fixture f; auto recognizer = new_shared<Eager>(); f.child->addGestureRecognizer(recognizer); f.begin();
    recognizer->onStateChanged = [](const auto& value) {
        if (value->state() == UIGestureRecognizerState::cancelled) throw std::runtime_error("application cancellation callback");
    };
    bool propagated = false;
    try { f.child->setHidden(true); } catch (const std::runtime_error&) { propagated = true; }
    expect(propagated, "cancellation propagates application callback exception");
    recognizer->onStateChanged = [](const auto&) {};
    f.send(UITouchPhase::ended);
    Fixture later; auto tap = new_shared<EndTap>(); int taps = 0; observe(tap, taps);
    tap->requireGestureRecognizerToFail(later.scroll->panGestureRecognizer()); later.child->addGestureRecognizer(tap);
    later.begin(); later.send(UITouchPhase::ended);
    expect(taps == 1 && tap->state() == UIGestureRecognizerState::possible,
           "throwing cancellation leaves no dispatch scope blocking subsequent pending gestures");
}
}
int main() {
    childTapWaitsForPan(); continuousRequestsAndRemoval(); validationAndWeakOwnership(); dynamicRequirements(); filtering(); simultaneousAndPrevention(); presses();
    cancellationAndReentrancy(); simultaneousTerminalContacts();
    reentrantAttachmentHasOneOwner(); throwingCancellationClosesDispatchScope();
    std::cout << "Gesture failure regression failures: " << failures << '\n'; return failures ? 1 : 0;
}
