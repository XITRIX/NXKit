#include <UIGestureRecognizer.h>
#include <UITouch.h>
#include <UIPress.h>

namespace NXKit {

UIGestureRecognizer::UIGestureRecognizer(std::function<void(std::shared_ptr<UIGestureRecognizer>)> onStateChanged):
onStateChanged(onStateChanged)
{ }

UIGestureRecognizer::~UIGestureRecognizer() {
    for (auto touch: _allTouches) {
        touch->_hasBeenCancelledByAGestureRecognizer = true;
    }
}

void UIGestureRecognizer::setEnabled(bool enabled) {
    _isEnabled = enabled;
    if (!_isEnabled) { setState(UIGestureRecognizerState::cancelled); }
}

void UIGestureRecognizer::setState(UIGestureRecognizerState state) {
    if (this->_state == state) return;
    this->_state = state;
    onStateChanged(shared_from_this());
    switch (state) {
        case UIGestureRecognizerState::began:
            cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously();
            break;
        case UIGestureRecognizerState::recognized:
        case UIGestureRecognizerState::ended:
            this->_state = UIGestureRecognizerState::possible;
            break;
        default: break;
    }
}

void UIGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}
void UIGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {}

void UIGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}
void UIGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>> presses, std::shared_ptr<UIPressesEvent> event) {}

bool UIGestureRecognizer::recognitionCondition() {
    return _state != UIGestureRecognizerState::failed &&
    _state != UIGestureRecognizerState::cancelled;
}

void UIGestureRecognizer::_touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    bool firstTouch = _allTouches.size() == 0;

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

    for (auto touch : touches)
        removeTouch(touch);

    if (condition)
        touchesEnded(touches, event);

    if (_allTouches.size() == 0)
        setState(UIGestureRecognizerState::possible);
}

void UIGestureRecognizer::_touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent> event) {
    for (auto touch : touches)
        removeTouch(touch);

    //    if (!recognitionCondition()) return;

    touchesCancelled(touches, event);
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
    if (_allTouches.size() <= 0 && _state == UIGestureRecognizerState::failed)
        setState(UIGestureRecognizerState::possible);
}

void UIGestureRecognizer::cancelOtherGestureRecognizersThatShouldNotRecognizeSimultaneously() {
    for (auto touch : _allTouches) {
        for (auto rec : touch->_gestureRecognizers) {
            if (rec.expired()) continue;
            auto lrec = rec.lock();
            if (lrec.get() != this && (lrec->delegate.expired() || !lrec->delegate.lock()->gestureRecognizerShouldRecognizeSimultaneouslyWith(lrec, shared_from_this()))) {
                lrec->setState(UIGestureRecognizerState::cancelled);
                lrec->touchesCancelled({touch}, nullptr);
            }
        }
    }
}

}

