#include <UIGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>

#include <algorithm>

namespace NXKit {

UIGestureRecognizer::UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged):
onStateChanged(onStateChanged)
{ }

UIGestureRecognizer::~UIGestureRecognizer() = default;

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

    onStateChanged(shared_from_this());
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

    touchesBegan(touches, event);

    // Check touche's hasBeenCancelledByAGestureRecognizer in case
    // gesture has been deinited by action in touchBegan
    if (touches.back()->_hasBeenCancelledByAGestureRecognizer) return;

    if (firstTouch && _state == UIGestureRecognizerState::possible)
        onStateChanged(shared_from_this());
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
    _allTouches.erase(std::remove(_allTouches.begin(), _allTouches.end(), touch));
}

void UIGestureRecognizer::resetCurrentAttemptIfNeeded() {
    if (_didResetCurrentAttempt) return;
    reset();
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
