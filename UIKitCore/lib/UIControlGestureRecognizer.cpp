#include <UIControlGestureRecognizer.h>
#include <UITouch.h>
#include <algorithm>

namespace NXKit {
UIControlGestureRecognizer::UIControlGestureRecognizer(const std::weak_ptr<UIControl>& owner) : control(owner) {
    setCancelsTouchesInView(false);
}
bool UIControlGestureRecognizer::canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const {
    const auto owner=control.lock();
    return !owner || !owner->isTracking() || !owner->requiresExclusiveTouchTracking();
}
void UIControlGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if (_touchToTrack || touches.empty()) return;
    const auto owner=control.lock(); if(!owner)return;
    if (owner->isTracking() && owner->requiresExclusiveTouchTracking()) owner->cancelTracking();
    _touchToTrack=touches.front();
    if (!owner->beginTouch(_touchToTrack,event)) {
        _touchToTrack.reset();setState(UIGestureRecognizerState::failed);return;
    }
    if (owner->requiresExclusiveTouchTracking()) setState(UIGestureRecognizerState::began);
}
void UIControlGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if (!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end()) return;
    if(auto owner=control.lock()) {
        owner->moveTouch(_touchToTrack,event);
        if(!owner->isTracking()) { _touchToTrack.reset();setState(UIGestureRecognizerState::failed); }
        else if(state()==UIGestureRecognizerState::began) setState(UIGestureRecognizerState::changed);
    }
}
void UIControlGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if (!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end()) return;
    auto touch=std::move(_touchToTrack); _touchToTrack.reset();
    setState(UIGestureRecognizerState::ended);
    if(auto owner=control.lock())owner->endTouch(touch,event);
}
void UIControlGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if (!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end()) return;
    _touchToTrack.reset();
    if(auto owner=control.lock())owner->cancelTracking(event);
    setState(UIGestureRecognizerState::failed);
}
void UIControlGestureRecognizer::reset() {
    const bool active=bool(_touchToTrack); _touchToTrack.reset();
    if(active)if(auto owner=control.lock())owner->cancelTracking();
}
void UIControlGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
}
