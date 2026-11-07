#include <UIControlGestureRecognizer.h>
#include <UITouch.h>
#include <DispatchQueue.h>
#include <algorithm>
#include <cmath>

namespace NXKit {
UIControlGestureRecognizer::UIControlGestureRecognizer(const std::weak_ptr<UIControl>& owner) : control(owner) {
    setCancelsTouchesInView(false);
}
bool UIControlGestureRecognizer::canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const {
    const auto owner=control.lock();
    return !owner || !owner->hasTrackingContact() || !owner->requiresExclusiveTouchTracking();
}
void UIControlGestureRecognizer::claimContact() {
    // A delayed tap may be replayed after the raw gesture attempt has ended.
    // Recognizing an empty attempt here would leak `began` into the next touch.
    if(_touchToTrack && !_inputEnded && state()==UIGestureRecognizerState::possible)
        setState(UIGestureRecognizerState::began);
}
void UIControlGestureRecognizer::rejectContact(const std::shared_ptr<UITouch>& touch) {
    if(_touchToTrack!=touch)return;
    setState(UIGestureRecognizerState::failed);
}
void UIControlGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if(_touchToTrack || touches.empty())return;
    const auto owner=control.lock();
    if(!owner || !owner->isEnabled() || !owner->isActionDispatchAllowed()) {setState(UIGestureRecognizerState::failed);return;}
    const auto generation=_contactGeneration;
    const auto expectedSuperview=owner->superview().lock();
    if(owner->hasTrackingContact() && owner->requiresExclusiveTouchTracking())owner->cancelTracking();
    if(_contactGeneration!=generation || _touchToTrack || !isEnabled()
        || !owner->isEnabled() || !owner->isActionDispatchAllowed()
        || owner->superview().lock()!=expectedSuperview || owner->window()!=touches.front()->window().lock())return;
    _touchToTrack=touches.front();_contactOrigin=_touchToTrack->locationIn(owner);
    _inputEnded=false;_completedTouch.reset();_peakTravel={};++_contactGeneration;
    _capturesDrag=owner->capturesUndelayedDrag(_touchToTrack);
    // Contact feedback is not UIControl tracking: UIKit's fluid slider starts
    // lifting before a scroll view delivers its delayed touchesBegan.
    owner->contactFeedbackBegan(_touchToTrack,event);
}
void UIControlGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) {
    if(!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end())return;
    const auto owner=control.lock();
    if(!owner || !owner->isEnabled() || !owner->isActionDispatchAllowed()) {setState(UIGestureRecognizerState::failed);return;}
    const auto travel=_touchToTrack->locationIn(owner)-_contactOrigin;
    _peakTravel={std::max(_peakTravel.x,std::abs(travel.x)),std::max(_peakTravel.y,std::abs(travel.y))};
    if(state()==UIGestureRecognizerState::possible && _capturesDrag) {
        if(std::max(std::abs(travel.x),std::abs(travel.y))>=10)claimContact();
    } else if(state()==UIGestureRecognizerState::began)setState(UIGestureRecognizerState::changed);
}
void UIControlGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) {
    if(!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end())return;
    _inputEnded=true;_completedTouch=_touchToTrack;
    if(const auto owner=control.lock()) {
        const auto travel=_touchToTrack->locationIn(owner)-_contactOrigin;
        _completedTravel={std::max(_peakTravel.x,std::abs(travel.x)),std::max(_peakTravel.y,std::abs(travel.y))};
    }
    if(state()==UIGestureRecognizerState::began || state()==UIGestureRecognizerState::changed)setState(UIGestureRecognizerState::ended);
    // If responder admission was vetoed, no tracking callback can retire early
    // feedback. Run once after this event's responder delivery, never per frame.
    const auto generation=_contactGeneration;
    const auto weak=weak_from_base<UIControlGestureRecognizer>();
    DispatchQueue::main()->async([weak,generation]{if(const auto self=weak.lock();self && self->_contactGeneration==generation)
        if(const auto owner=self->control.lock();owner && !owner->hasTrackingContact())owner->contactFeedbackCancelled();});
}
void UIControlGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event) {
    if(!_touchToTrack || std::find(touches.begin(),touches.end(),_touchToTrack)==touches.end())return;
    if(const auto owner=control.lock()){owner->cancelTracking(event);owner->contactFeedbackCancelled();}
    _inputEnded=true;setState(UIGestureRecognizerState::cancelled);
}
void UIControlGestureRecognizer::reset() {
    const bool interrupted=_touchToTrack && !_inputEnded;
    _touchToTrack.reset();_capturesDrag=false;
    if(interrupted)if(const auto owner=control.lock()){owner->cancelTracking();owner->contactFeedbackCancelled();}
}
void UIControlGestureRecognizer::pressesBegan(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesChanged(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesEnded(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
void UIControlGestureRecognizer::pressesCancelled(std::vector<std::shared_ptr<UIPress>>,std::shared_ptr<UIPressesEvent>) {}
}
