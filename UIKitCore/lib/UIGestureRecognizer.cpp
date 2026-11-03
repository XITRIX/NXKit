#include <UIGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>

#include <algorithm>

namespace NXKit {

UIGestureRecognizer::UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged):
onStateChanged(onStateChanged)
{ }

UIGestureRecognizer::~UIGestureRecognizer() = default;

std::size_t UIGestureRecognizer::numberOfTouches() const {
    return std::count_if(_allTouches.begin(),_allTouches.end(),[](const auto& touch) {
        return touch->phase()!=UITouchPhase::ended&&touch->phase()!=UITouchPhase::cancelled;
    });
}

void UIGestureRecognizer::setEnabled(bool enabled) {
    if (_isEnabled == enabled) return;

    _isEnabled = enabled;
    if (_isEnabled) return;

    if (_state == UIGestureRecognizerState::began
        || _state == UIGestureRecognizerState::changed) {
        setState(UIGestureRecognizerState::cancelled);
    } else if (_state == UIGestureRecognizerState::possible
               && !_allTouches.empty()) {
        setState(UIGestureRecognizerState::failed);
    }
    if (!_allTouches.empty()) {
        resetCurrentAttemptIfNeeded();
        _allTouches.clear();
        _state = UIGestureRecognizerState::possible;
    }
}

void UIGestureRecognizer::setState(UIGestureRecognizerState state) {
    if (_state == state) return;

    // Ask before recognition cancels competing gestures or content touches.
    if (_state == UIGestureRecognizerState::possible &&
        (state == UIGestureRecognizerState::began || state == UIGestureRecognizerState::ended)) {
        if (const auto target = delegate.lock()) {
            const auto generation = _resetGeneration;
            const auto allowed = target->gestureRecognizerShouldBegin(shared_from_this());
            if (!_isEnabled || _state != UIGestureRecognizerState::possible || _resetGeneration != generation) return;
            if (!allowed) state = UIGestureRecognizerState::failed;
        }
    }

    const auto previousState = _state;
    _state = state;
    if (state == UIGestureRecognizerState::ended
        || state == UIGestureRecognizerState::cancelled
        || state == UIGestureRecognizerState::failed) {
        _didResetCurrentAttempt = false;
    }

    const bool beganContinuousRecognition =
        state == UIGestureRecognizerState::began;
    const bool completedDiscreteRecognition =
        previousState == UIGestureRecognizerState::possible
        && state == UIGestureRecognizerState::ended;
    if (beganContinuousRecognition || completedDiscreteRecognition) {
        cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
        if (_cancelsTouchesInView) {
            for (const auto& touch : _allTouches) {
                touch->_hasBeenCancelledByAGestureRecognizer = true;
            }
        }
    }

    notifyStateChanged();
}

void UIGestureRecognizer::notifyStateChanged() {
    didChangeState();
    if (onStateChanged) onStateChanged(shared_from_this());
}

bool UIGestureRecognizer::canPrevent(
    const std::shared_ptr<UIGestureRecognizer>&
) const {
    return true;
}

bool UIGestureRecognizer::canBePreventedBy(
    const std::shared_ptr<UIGestureRecognizer>&
) const {
    return true;
}

void UIGestureRecognizer::reset() {}

void UIGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}

void UIGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}

bool UIGestureRecognizer::recognitionCondition() {
    return _state == UIGestureRecognizerState::possible
        || _state == UIGestureRecognizerState::began
        || _state == UIGestureRecognizerState::changed;
}

void UIGestureRecognizer::_touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    if (touches.empty()) return;

    bool firstTouch = _allTouches.empty();
    if (firstTouch) {
        _didResetCurrentAttempt = false;
    }

    for (auto touch : touches)
        addTouch(touch);

    if (!recognitionCondition()) return;

    const bool wasRecognizing = _state == UIGestureRecognizerState::began
        || _state == UIGestureRecognizerState::changed;
    touchesBegan(touches, event);

    // An already recognized continuous gesture also owns later fingers. They
    // must not start a child control's tap while the first finger is scrolling.
    if (wasRecognizing || _state == UIGestureRecognizerState::began
        || _state == UIGestureRecognizerState::changed) {
        cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
        if (_cancelsTouchesInView)
            for (const auto& touch : touches)
                touch->_hasBeenCancelledByAGestureRecognizer = true;
    }

    // Check touche's hasBeenCancelledByAGestureRecognizer in case
    // gesture has been deinited by action in touchBegan
    if (touches.back()->_hasBeenCancelledByAGestureRecognizer) return;

    if (firstTouch && _state == UIGestureRecognizerState::possible)
        notifyStateChanged();
}

void UIGestureRecognizer::_touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    if (!recognitionCondition()) return;
    touchesMoved(touches, event);
}

void UIGestureRecognizer::_touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    bool condition = recognitionCondition();

    if (condition)
        touchesEnded(touches, event);

    for (auto touch : touches)
        removeTouch(touch);

    if (_allTouches.empty()) {
        resetCurrentAttemptIfNeeded();
        _state = UIGestureRecognizerState::possible;
    }
}

void UIGestureRecognizer::_touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    bool condition = recognitionCondition();

    if (condition)
        touchesCancelled(touches, event);

    for (auto touch : touches)
        removeTouch(touch);

    if (_allTouches.empty()) {
        resetCurrentAttemptIfNeeded();
        _state = UIGestureRecognizerState::possible;
    }
}

void UIGestureRecognizer::_pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (!recognitionCondition()) return;
    pressesBegan(presses, event);
}

void UIGestureRecognizer::_pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (!recognitionCondition()) return;
    pressesChanged(presses, event);
}

void UIGestureRecognizer::_pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (!recognitionCondition()) return;
    pressesEnded(presses, event);
}

void UIGestureRecognizer::_pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {
    if (!recognitionCondition()) return;
    pressesCancelled(presses, event);
}

void UIGestureRecognizer::addTouch(std::shared_ptr<UITouch> touch) {
    _allTouches.push_back(touch);
}

void UIGestureRecognizer::removeTouch(std::shared_ptr<UITouch> touch) {
    // A callback can disable/reset this recognizer before terminal delivery
    // removes the touch. Erasing an absent touch must be harmless.
    std::erase(_allTouches, touch);
}

void UIGestureRecognizer::resetCurrentAttemptIfNeeded() {
    if (_didResetCurrentAttempt) return;
    reset();
    ++_resetGeneration;
    _didResetCurrentAttempt = true;
}

void UIGestureRecognizer::cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously() {
    std::vector<std::shared_ptr<UIGestureRecognizer>> competingRecognizers;
    for (auto touch : _allTouches) {
        for (auto rec : touch->_gestureRecognizers) {
            if (rec.expired()) continue;
            auto competingRecognizer = rec.lock();
            if (competingRecognizer.get() == this) continue;
            if (std::find(
                    competingRecognizers.begin(),
                    competingRecognizers.end(),
                    competingRecognizer
                ) == competingRecognizers.end()) {
                competingRecognizers.push_back(std::move(competingRecognizer));
            }
        }
    }

    const auto self = shared_from_this();
    for (const auto& competingRecognizer : competingRecognizers) {
        bool allowsSimultaneousRecognition = false;
        if (const auto ownDelegate = delegate.lock()) {
            allowsSimultaneousRecognition =
                ownDelegate->gestureRecognizerShouldRecognizeSimultaneouslyWith(
                    self,
                    competingRecognizer
                );
        }
        if (const auto competingDelegate = competingRecognizer->delegate.lock()) {
            allowsSimultaneousRecognition = allowsSimultaneousRecognition
                || competingDelegate->gestureRecognizerShouldRecognizeSimultaneouslyWith(
                    competingRecognizer,
                    self
                );
        }
        if (allowsSimultaneousRecognition
            || !canPrevent(competingRecognizer)
            || !competingRecognizer->canBePreventedBy(self)) {
            continue;
        }

        const auto competingState = competingRecognizer->state();
        if (competingState == UIGestureRecognizerState::possible) {
            competingRecognizer->setState(UIGestureRecognizerState::failed);
        } else if (competingState == UIGestureRecognizerState::began
                   || competingState == UIGestureRecognizerState::changed) {
            competingRecognizer->setState(UIGestureRecognizerState::cancelled);
        } else {
            continue;
        }

        competingRecognizer->resetCurrentAttemptIfNeeded();
    }
}

}
