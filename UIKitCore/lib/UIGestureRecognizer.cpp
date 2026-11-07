#include <UIGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIWindow.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace NXKit {
namespace {
// Gesture dispatch is main-thread confined. Keep attempts weakly registered so
// a dependency cannot keep an application view or recognizer alive.
thread_local std::vector<std::weak_ptr<UIGestureRecognizer>> attempts;
thread_local unsigned dispatchDepth = 0;
thread_local bool resolving = false;
thread_local bool resolutionRequested = false;

bool terminal(UIGestureRecognizerState state) {
    return state == UIGestureRecognizerState::ended || state == UIGestureRecognizerState::failed
        || state == UIGestureRecognizerState::cancelled;
}
bool recognized(UIGestureRecognizerState state) {
    return state == UIGestureRecognizerState::began || state == UIGestureRecognizerState::changed
        || state == UIGestureRecognizerState::ended;
}
template<class T> bool contains(const std::vector<std::shared_ptr<T>>& values, const std::shared_ptr<T>& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}
template<class T> bool contains(const std::vector<std::weak_ptr<T>>& values, const std::shared_ptr<T>& value) {
    return std::any_of(values.begin(), values.end(), [&](const auto& candidate) { return candidate.lock() == value; });
}
}

UIGestureRecognizer::UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> action)
    : onStateChanged(std::move(action)) {}
UIGestureRecognizer::~UIGestureRecognizer() = default;

void UIGestureRecognizer::setDelaysTouchesBegan(bool value) {
    if (_delaysTouchesBegan == value) return;
    _delaysTouchesBegan = value;
    flushResponderTouchDelivery();
}

void UIGestureRecognizer::setDelaysTouchesEnded(bool value) {
    if (_delaysTouchesEnded == value) return;
    _delaysTouchesEnded = value;
    flushResponderTouchDelivery();
}

void UIGestureRecognizer::flushResponderTouchDelivery() {
    std::vector<std::shared_ptr<UIWindow>> windows;
    const auto snapshot = attempts;
    for (const auto& weak: snapshot) {
        const auto recognizer = weak.lock();
        if (!recognizer) continue;
        for (const auto& touch: recognizer->_allTouches)
            if (const auto window = touch->_window.lock(); window && !contains(windows, window))
                windows.push_back(window);
    }
    for (const auto& window: windows) window->flushResponderTouches();
}

std::size_t UIGestureRecognizer::numberOfTouches() const {
    return std::count_if(_allTouches.begin(), _allTouches.end(), [](const auto& touch) {
        return touch->phase() != UITouchPhase::ended && touch->phase() != UITouchPhase::cancelled;
    });
}

void UIGestureRecognizer::registerAttempt() {
    const auto self = shared_from_this();
    if (!contains(attempts, self)) attempts.push_back(self);
}

void UIGestureRecognizer::beginEventDispatch() { ++dispatchDepth; }
void UIGestureRecognizer::endEventDispatch() {
    if (dispatchDepth && --dispatchDepth == 0) resolvePendingRecognitions();
}

void UIGestureRecognizer::resolvePendingRecognitions() {
    if (dispatchDepth) return;
    if (resolving) { resolutionRequested = true; return; }
    resolving = true;
    struct ResolutionScope { ~ResolutionScope() { resolving = false; } } resolutionScope;
    do {
        resolutionRequested = false;
        bool changed;
        do {
            changed = false;
            const auto snapshot = attempts;
            for (const auto& weak : snapshot) {
                if (const auto recognizer = weak.lock()) changed |= recognizer->resolvePendingRecognition();
            }
        } while (changed);
        // Responder callbacks may change other recognizers' requirements or
        // states. Resolve that work before resetting any terminal outcomes.
        flushResponderTouchDelivery();
    } while (resolutionRequested);
    // Preserve every terminal outcome until all pending requests have seen it.
    // In particular, a child tap runs before its ancestor pan's touchesEnded.
    const auto snapshot = attempts;
    for (const auto& weak : snapshot) {
        if (const auto recognizer = weak.lock(); recognizer && recognizer->_needsAttemptCleanup
            && !recognizer->_pendingRecognition && !recognizer->hasLiveInputs()) recognizer->finishAttempt();
    }
    std::erase_if(attempts, [](const auto& weak) {
        const auto recognizer = weak.lock();
        return !recognizer || (recognizer->_allTouches.empty() && recognizer->_allPresses.empty()
            && !recognizer->_pendingRecognition && !recognizer->_needsAttemptCleanup);
    });
}

bool UIGestureRecognizer::hasLiveInputs() const {
    // An event may mark several contacts ended before dispatching any of
    // them. Wait for terminal delivery, not merely each UITouch's phase.
    for (const auto& touch : _allTouches) if (!contains(_terminalTouches, touch)) return true;
    for (const auto& press : _allPresses) if (!contains(_terminalPresses, press)) return true;
    return false;
}

void UIGestureRecognizer::finishAttempt() {
    resetCurrentAttemptIfNeeded();
    _allTouches.clear();
    _allPresses.clear();
    _terminalTouches.clear();
    _terminalPresses.clear();
    _attemptRequirements.clear();
    _pendingRecognition.reset();
    _requirementsResolved = _askedShouldBegin = _needsAttemptCleanup = _attemptCancelled = false;
    _state = UIGestureRecognizerState::possible;
}

bool UIGestureRecognizer::requiresFailureTransitively(const UIGestureRecognizer* other) const {
    std::unordered_set<const UIGestureRecognizer*> visited;
    const auto visit = [&](const auto& recurse, const UIGestureRecognizer* recognizer) -> bool {
        if (!visited.insert(recognizer).second) return false;
        if (recognizer == other) return true;
        for (const auto& weak : recognizer->_requiredToFail)
            if (const auto required = weak.lock(); required && recurse(recurse, required.get())) return true;
        for (const auto& weak : recognizer->_attemptRequirements)
            if (const auto required = weak.lock(); required && recurse(recurse, required.get())) return true;
        return false;
    };
    return visit(visit, this);
}

void UIGestureRecognizer::requireGestureRecognizerToFail(const std::shared_ptr<UIGestureRecognizer>& other) {
    if (!other || other.get() == this || other->requiresFailureTransitively(this))
        throw std::invalid_argument("Gesture failure requirements must be non-null and acyclic");
    if (contains(_requiredToFail, other)) return;
    _requiredToFail.push_back(other);
    // Requirements added during an attempt apply to its next recognition
    // request as well as to subsequent attempts.
    _requirementsResolved = false;
    if (_pendingRecognition) prepareFailureRequirements();
    resolvePendingRecognitions();
}

bool UIGestureRecognizer::shouldRequireFailureOf(const std::shared_ptr<UIGestureRecognizer>&) const { return false; }
bool UIGestureRecognizer::shouldBeRequiredToFailBy(const std::shared_ptr<UIGestureRecognizer>&) const { return false; }

bool UIGestureRecognizer::sharesAttemptWith(const UIGestureRecognizer& other) const {
    // A dependency outside this input's hierarchy cannot recognize that input.
    // A not-yet-dispatched ancestor still participates, unless its admission
    // delegate rejected this particular contact.
    if (_allTouches.empty() && _allPresses.empty()) return true;
    for (const auto& touch : _allTouches) {
        if (contains(other._ignoredTouches, touch)) continue;
        if (contains(other._allTouches, touch)) return true;
        for (const auto& candidate : touch->_gestureRecognizers)
            if (candidate.lock().get() == &other) return true;
    }
    for (const auto& press : _allPresses) {
        if (contains(other._ignoredPresses, press)) continue;
        if (contains(other._allPresses, press)) return true;
        for (const auto& candidate : press->_gestureRecognizers)
            if (candidate.lock().get() == &other) return true;
    }
    return false;
}

void UIGestureRecognizer::prepareFailureRequirements() {
    if (_requirementsResolved) return;
    _requirementsResolved = true;
    _attemptRequirements = _requiredToFail;
    std::vector<std::shared_ptr<UIGestureRecognizer>> candidates;
    const auto collect = [&](const auto& inputs) {
        for (const auto& input : inputs)
            for (const auto& weak : input->_gestureRecognizers)
                if (const auto other = weak.lock(); other && other.get() != this && !contains(candidates, other))
                    candidates.push_back(other);
    };
    collect(_allTouches);
    collect(_allPresses);
    const auto self = shared_from_this();
    const auto generation = _resetGeneration;
    for (const auto& other : candidates) {
        if (contains(_attemptRequirements, other)) continue;
        bool required = shouldRequireFailureOf(other) || other->shouldBeRequiredToFailBy(self);
        if (const auto own = delegate.lock()) required |= own->gestureRecognizerShouldRequireFailureOf(self, other);
        if (const auto theirs = other->delegate.lock()) required |= theirs->gestureRecognizerShouldBeRequiredToFailBy(other, self);
        if (_resetGeneration != generation || !_isEnabled) return;
        if (required) {
            // A contradictory dynamic policy must fail this attempt rather
            // than leave two possible recognizers waiting forever.
            if (other->requiresFailureTransitively(this)) {
                _pendingRecognition.reset();
                commitState(UIGestureRecognizerState::failed);
                return;
            }
            _attemptRequirements.push_back(other);
        }
    }
}

bool UIGestureRecognizer::resolvePendingRecognition() {
    if (!_pendingRecognition || _state != UIGestureRecognizerState::possible) return false;
    bool waiting = false;
    for (const auto& weak : _attemptRequirements) {
        const auto other = weak.lock();
        if (!other || !other->_isEnabled || other->_attemptCancelled || !sharesAttemptWith(*other)) continue;
        if (recognized(other->_state)) {
            _pendingRecognition.reset();
            commitState(UIGestureRecognizerState::failed);
            return true;
        }
        waiting |= other->_state == UIGestureRecognizerState::possible;
    }
    if (waiting) return false;
    const auto target = *_pendingRecognition;
    _pendingRecognition.reset();
    commitState(target);
    return true;
}

void UIGestureRecognizer::setEnabled(bool enabled) {
    if (_isEnabled == enabled) return;
    _isEnabled = enabled;
    if (enabled) return;
    cancelForUnavailableView();
}

void UIGestureRecognizer::cancelForUnavailableView() {
    if (_allTouches.empty() && _allPresses.empty() && !_pendingRecognition) return;
    _attemptCancelled = true;
    if (_state == UIGestureRecognizerState::began || _state == UIGestureRecognizerState::changed)
        setState(UIGestureRecognizerState::cancelled);
    else if (_state == UIGestureRecognizerState::possible) setState(UIGestureRecognizerState::failed);
    resetCurrentAttemptIfNeeded();
    // Contacts already assigned to this attempt must not restart it when an
    // application re-enables or reparents the view during the same sequence.
    for (const auto& touch : _allTouches) if (!contains(_ignoredTouches, touch)) _ignoredTouches.push_back(touch);
    for (const auto& press : _allPresses) if (!contains(_ignoredPresses, press)) _ignoredPresses.push_back(press);
    _allTouches.clear();
    _allPresses.clear();
    _needsAttemptCleanup = true;
    resolvePendingRecognitions();
}

void UIGestureRecognizer::setState(UIGestureRecognizerState state) {
    if (_state == state && !_pendingRecognition) return;
    if (_state == UIGestureRecognizerState::possible && recognized(state)) {
        if (!_askedShouldBegin) {
            _askedShouldBegin = true;
            if (const auto target = delegate.lock()) {
                const auto generation = _resetGeneration;
                const auto allowed = target->gestureRecognizerShouldBegin(shared_from_this());
                if (!_isEnabled || _state != UIGestureRecognizerState::possible || _resetGeneration != generation) return;
                if (!allowed) { setState(UIGestureRecognizerState::failed); return; }
            }
        }
        registerAttempt();
        prepareFailureRequirements();
        if (_state != UIGestureRecognizerState::possible || !_isEnabled) return;
        _pendingRecognition = state;
        resolvePendingRecognition();
        resolvePendingRecognitions();
        return;
    }
    _pendingRecognition.reset();
    commitState(state);
    resolvePendingRecognitions();
}

void UIGestureRecognizer::commitState(UIGestureRecognizerState state) {
    if (_state == state) return;
    const auto previousState = _state;
    _state = state;
    if (terminal(state)) _didResetCurrentAttempt = false;
    if (state == UIGestureRecognizerState::began
        || (previousState == UIGestureRecognizerState::possible && state == UIGestureRecognizerState::ended)) {
        const auto generation = _resetGeneration;
        cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
        if (_resetGeneration != generation || _state != state) return;
        if (_cancelsTouchesInView) {
            for (const auto& touch : _allTouches) touch->_hasBeenCancelledByAGestureRecognizer = true;
            for (const auto& press : _allPresses) press->_hasBeenCancelledByAGestureRecognizer = true;
        }
    }
    // Apple's action callbacks observe an already-cancelled responder stream.
    // During input arbitration UIWindow only flushes cancellations here.
    flushResponderTouchDelivery();
    notifyStateChanged();
}

void UIGestureRecognizer::notifyStateChanged() {
    didChangeState();
    if (onStateChanged) onStateChanged(shared_from_this());
}

bool UIGestureRecognizer::canPrevent(const std::shared_ptr<UIGestureRecognizer>&) const { return true; }
bool UIGestureRecognizer::canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const { return true; }
void UIGestureRecognizer::reset() {}
void UIGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) {}
void UIGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) {}
void UIGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) {}
void UIGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>>, std::shared_ptr<UIEvent>) {}
void UIGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) {}
void UIGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) {}
void UIGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) {}
void UIGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) {}

bool UIGestureRecognizer::recognitionCondition() {
    return _isEnabled && !_attemptCancelled && !terminal(_state);
}

void UIGestureRecognizer::_touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    if (!_isEnabled || touches.empty()) return;
    const auto self = shared_from_this();
    const auto generation = _resetGeneration;
    bool acceptEvent = true;
    if (const auto target = delegate.lock()) acceptEvent = target->gestureRecognizerShouldReceiveEvent(self, event);
    if (!_isEnabled || generation != _resetGeneration) return;
    std::vector<std::shared_ptr<UITouch>> accepted;
    for (const auto& touch : touches) {
        bool accept = acceptEvent && !contains(_ignoredTouches, touch);
        if (accept) if (const auto target = delegate.lock()) accept = target->gestureRecognizerShouldReceiveTouch(self, touch);
        if (!_isEnabled || generation != _resetGeneration) return;
        if (accept) accepted.push_back(touch);
        else if (!contains(_ignoredTouches, touch)) _ignoredTouches.push_back(touch);
    }
    if (accepted.empty()) return;
    const bool firstTouch = _allTouches.empty() && _allPresses.empty();
    if (firstTouch) { _didResetCurrentAttempt = false; _attemptCancelled = false; }
    for (const auto& touch : accepted) addTouch(touch);
    registerAttempt();
    if (!recognitionCondition()) return;
    const bool wasRecognizing = recognized(_state);
    touchesBegan(accepted, event);
    if (wasRecognizing || recognized(_state)) {
        cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
        if (_cancelsTouchesInView) for (const auto& touch : accepted) touch->_hasBeenCancelledByAGestureRecognizer = true;
    }
    if (accepted.back()->_hasBeenCancelledByAGestureRecognizer) return;
    if (firstTouch && _state == UIGestureRecognizerState::possible) notifyStateChanged();
}

void UIGestureRecognizer::_touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    std::erase_if(touches, [&](const auto& touch) { return !contains(_allTouches, touch); });
    if (recognitionCondition() && !touches.empty()) touchesMoved(touches, event);
}

void UIGestureRecognizer::_touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    std::erase_if(_ignoredTouches, [&](const auto& weak) { const auto touch = weak.lock(); return !touch; });
    std::erase_if(touches, [&](const auto& touch) { return !contains(_allTouches, touch); });
    if (touches.empty()) return;
    if (recognitionCondition()) touchesEnded(touches, event);
    for (const auto& touch : touches) if (!contains(_terminalTouches, touch)) _terminalTouches.push_back(touch);
    if (!hasLiveInputs()) {
        if (_state == UIGestureRecognizerState::possible && !_pendingRecognition) setState(UIGestureRecognizerState::failed);
        _needsAttemptCleanup = true;
    }
    resolvePendingRecognitions();
}

void UIGestureRecognizer::_touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    std::erase_if(_ignoredTouches, [&](const auto& weak) { const auto touch = weak.lock(); return !touch; });
    std::erase_if(touches, [&](const auto& touch) { return !contains(_allTouches, touch); });
    if (touches.empty()) return;
    if (recognitionCondition()) touchesCancelled(touches, event);
    for (const auto& touch : touches) if (!contains(_terminalTouches, touch)) _terminalTouches.push_back(touch);
    if (!hasLiveInputs()) {
        _pendingRecognition.reset();
        if (!terminal(_state)) setState(recognized(_state) ? UIGestureRecognizerState::cancelled : UIGestureRecognizerState::failed);
        _needsAttemptCleanup = true;
    }
    resolvePendingRecognitions();
}

void UIGestureRecognizer::_pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (!_isEnabled || presses.empty()) return;
    const auto self = shared_from_this();
    const auto generation = _resetGeneration;
    bool acceptEvent = true;
    if (const auto target = delegate.lock()) acceptEvent = target->gestureRecognizerShouldReceiveEvent(self, event);
    if (!_isEnabled || generation != _resetGeneration) return;
    std::vector<std::shared_ptr<UIPress>> accepted;
    for (const auto& press : presses) {
        bool accept = acceptEvent && !contains(_ignoredPresses, press);
        if (accept) if (const auto target = delegate.lock()) accept = target->gestureRecognizerShouldReceivePress(self, press);
        if (!_isEnabled || generation != _resetGeneration) return;
        if (accept) accepted.push_back(press);
        else if (!contains(_ignoredPresses, press)) _ignoredPresses.push_back(press);
    }
    if (accepted.empty()) return;
    if (_allTouches.empty() && _allPresses.empty()) { _didResetCurrentAttempt = false; _attemptCancelled = false; }
    for (const auto& press : accepted) if (!contains(_allPresses, press)) _allPresses.push_back(press);
    registerAttempt();
    if (recognitionCondition()) pressesBegan(accepted, event);
}
void UIGestureRecognizer::_pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    std::erase_if(presses, [&](const auto& press) { return !contains(_allPresses, press); });
    if (recognitionCondition() && !presses.empty()) pressesChanged(presses, event);
}
void UIGestureRecognizer::_pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    std::erase_if(_ignoredPresses, [&](const auto& weak) { const auto press = weak.lock(); return !press; });
    std::erase_if(presses, [&](const auto& press) { return !contains(_allPresses, press); });
    if (presses.empty()) return;
    if (recognitionCondition()) pressesEnded(presses, event);
    for (const auto& press : presses) if (!contains(_terminalPresses, press)) _terminalPresses.push_back(press);
    if (!hasLiveInputs()) {
        if (_state == UIGestureRecognizerState::possible && !_pendingRecognition) setState(UIGestureRecognizerState::failed);
        _needsAttemptCleanup = true;
    }
    resolvePendingRecognitions();
}
void UIGestureRecognizer::_pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    std::erase_if(_ignoredPresses, [&](const auto& weak) { const auto press = weak.lock(); return !press; });
    std::erase_if(presses, [&](const auto& press) { return !contains(_allPresses, press); });
    if (presses.empty()) return;
    if (recognitionCondition()) pressesCancelled(presses, event);
    for (const auto& press : presses) if (!contains(_terminalPresses, press)) _terminalPresses.push_back(press);
    if (!hasLiveInputs()) {
        _pendingRecognition.reset();
        if (!terminal(_state)) setState(recognized(_state) ? UIGestureRecognizerState::cancelled : UIGestureRecognizerState::failed);
        _needsAttemptCleanup = true;
    }
    resolvePendingRecognitions();
}

void UIGestureRecognizer::addTouch(std::shared_ptr<UITouch> touch) {
    if (!contains(_allTouches, touch)) _allTouches.push_back(std::move(touch));
}
void UIGestureRecognizer::removeTouch(std::shared_ptr<UITouch> touch) { std::erase(_allTouches, touch); }
void UIGestureRecognizer::resetCurrentAttemptIfNeeded() {
    if (_didResetCurrentAttempt) return;
    _didResetCurrentAttempt = true;
    ++_resetGeneration;
    reset();
}

void UIGestureRecognizer::cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously() {
    std::vector<std::shared_ptr<UIGestureRecognizer>> competitors;
    const auto collect = [&](const auto& inputs) {
        for (const auto& input : inputs) for (const auto& weak : input->_gestureRecognizers)
            if (const auto other = weak.lock(); other && other.get() != this && !contains(competitors, other)) competitors.push_back(other);
    };
    collect(_allTouches);
    collect(_allPresses);
    const auto self = shared_from_this();
    for (const auto& other : competitors) {
        if (!other->_isEnabled || other->_attemptCancelled || !sharesAttemptWith(*other)) continue;
        bool simultaneous = false;
        if (const auto own = delegate.lock()) simultaneous = own->gestureRecognizerShouldRecognizeSimultaneouslyWith(self, other);
        if (const auto theirs = other->delegate.lock()) simultaneous |= theirs->gestureRecognizerShouldRecognizeSimultaneouslyWith(other, self);
        if (simultaneous || !canPrevent(other) || !other->canBePreventedBy(self)) continue;
        if (other->_state == UIGestureRecognizerState::possible) other->setState(UIGestureRecognizerState::failed);
        else if (other->_state == UIGestureRecognizerState::began || other->_state == UIGestureRecognizerState::changed)
            other->setState(UIGestureRecognizerState::cancelled);
        else continue;
        other->resetCurrentAttemptIfNeeded();
    }
}
}
