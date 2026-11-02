#include <UIPanGestureRecognizer.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace NXKit {
void UIPanGestureRecognizer::setMinimumNumberOfTouches(std::size_t count) {
    if(!count||count>_maximumTouches)throw std::invalid_argument("Pan minimum touches must be positive and no greater than its maximum");
    _minimumTouches=count;
}
void UIPanGestureRecognizer::setMaximumNumberOfTouches(std::size_t count) {
    if(!count||count<_minimumTouches)throw std::invalid_argument("Pan maximum touches must be no smaller than its minimum");
    _maximumTouches=count;
    if(_touches.size()>count)setState(state()==UIGestureRecognizerState::possible?UIGestureRecognizerState::failed:UIGestureRecognizerState::cancelled);
}
std::size_t UIPanGestureRecognizer::numberOfTouches() const {
    return std::min(UIGestureRecognizer::numberOfTouches(),_maximumTouches);
}
NXPoint UIPanGestureRecognizer::centroid() const {
    NXPoint result;
    const auto count=std::min(_touches.size(),_maximumTouches);
    for(std::size_t i=0;i<count;++i)result+=_touches[i]->absoluteLocation();
    return count==0?result:result/float(count);
}
Timer UIPanGestureRecognizer::timestamp(const std::vector<std::shared_ptr<UITouch>>& changed) const {
    auto result=changed.front()->timestamp();
    for(const auto& touch:changed)if(touch->timestamp()-result>0)result=touch->timestamp();
    return result;
}
NXPoint UIPanGestureRecognizer::convertVector(NXPoint value,const std::shared_ptr<UIView>& view,bool toWindow) const {
    const auto window=_window.lock();
    if(!window||!view||view==window)return value;
    // Only the linear part affects a vector. UIView's point-conversion fallback
    // currently handles origins only, so accumulate the affine bases explicitly.
    NXPoint x(1,0),y(0,1);
    for(auto current=view;current&&current!=window;current=current->superview().lock()) {
        const auto t=current->transform();
        const auto apply=[&](NXPoint p){return NXPoint(t.m11*p.x+t.m21*p.y,t.m12*p.x+t.m22*p.y);};
        x=apply(x);y=apply(y);
    }
    if(toWindow)return x*value.x+y*value.y;
    const auto determinant=double(x.x)*y.y-double(y.x)*x.y;
    if(!std::isfinite(determinant)||determinant==0)return {};
    return {NXFloat((value.x*y.y-y.x*value.y)/determinant),NXFloat((x.x*value.y-value.x*x.y)/determinant)};
}
NXPoint UIPanGestureRecognizer::translationInView(const std::shared_ptr<UIView>& view) {return convertVector(_translation,view);}
void UIPanGestureRecognizer::setTranslation(NXPoint value,const std::shared_ptr<UIView>& view) {
    if(!std::isfinite(value.x)||!std::isfinite(value.y))throw std::invalid_argument("Pan translation must be finite");
    _translation=convertVector(value,view,true);
}
NXPoint UIPanGestureRecognizer::velocityIn(const std::shared_ptr<UIView>& view) {return convertVector(_velocity,view);}
void UIPanGestureRecognizer::rebase(const std::vector<std::shared_ptr<UITouch>>& changed) {
    _centroid=centroid();_sampleTime=timestamp(changed);_hasSample=true;
}
void UIPanGestureRecognizer::touchesBegan(std::vector<std::shared_ptr<UITouch>> changed,std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesBegan(changed,event);if(changed.empty())return;
    const bool first=_touches.empty();
    if(first){_translation={};_recognitionTravel={};_velocity={};_previousVelocity={};_hasVelocity=false;_window=changed.front()->window();}
    for(const auto& touch:changed)if(std::find(_touches.begin(),_touches.end(),touch)==_touches.end())_touches.push_back(touch);
    // Membership changes move the centroid but never move the gesture itself.
    rebase(changed);
    if(_touches.size()>_maximumTouches&&state()==UIGestureRecognizerState::possible)setState(UIGestureRecognizerState::failed);
}
void UIPanGestureRecognizer::touchesMoved(std::vector<std::shared_ptr<UITouch>> changed,std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesMoved(changed,event);if(changed.empty()||_touches.empty())return;
    if(std::none_of(changed.begin(),changed.end(),[&](const auto& t){return std::find(_touches.begin(),_touches.end(),t)!=_touches.end();}))return;
    const auto point=centroid();const auto time=timestamp(changed);const auto delta=point-_centroid;
    const auto seconds=(time-_sampleTime)/1000.0;
    if(_hasSample&&seconds<=0&&delta==NXPoint())return;
    _translation+=delta;_recognitionTravel+=delta;
    if(_hasSample&&seconds>0) {
        const auto instantaneous=delta/float(seconds);
        // Native traces use a two-sample FIR, not an accumulating low-pass.
        _velocity=_hasVelocity?_previousVelocity*.8f+instantaneous*.2f:instantaneous;
        _previousVelocity=instantaneous;_hasVelocity=true;
    }
    _centroid=point;_sampleTime=time;_hasSample=true;
    if(state()==UIGestureRecognizerState::possible) {
        if(_touches.size()<_minimumTouches)return;
        if(std::abs(_recognitionTravel.x)<10&&std::abs(_recognitionTravel.y)<10)return;
        if (_shouldBegin && !_shouldBegin(convertVector(_recognitionTravel,view().lock()))) {
            setState(UIGestureRecognizerState::failed);
            return;
        }
        // Native one-touch activation consumes the recognition distance;
        // waiting for additional required fingers retains the accumulated pan.
        if(_minimumTouches==1)_translation={};
        setState(UIGestureRecognizerState::began);
    } else if(state()==UIGestureRecognizerState::began)setState(UIGestureRecognizerState::changed);
    else if(state()==UIGestureRecognizerState::changed)onStateChanged(shared_from_this());
}
void UIPanGestureRecognizer::touchesEnded(std::vector<std::shared_ptr<UITouch>> changed,std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesEnded(changed,event);if(changed.empty())return;
    std::erase_if(_touches,[&](const auto& t){return std::find(changed.begin(),changed.end(),t)!=changed.end();});
    if(!_touches.empty()){rebase(changed);return;}
    if(state()==UIGestureRecognizerState::began||state()==UIGestureRecognizerState::changed)setState(UIGestureRecognizerState::ended);
    else if(state()==UIGestureRecognizerState::possible)setState(UIGestureRecognizerState::failed);
}
void UIPanGestureRecognizer::touchesCancelled(std::vector<std::shared_ptr<UITouch>> changed,std::shared_ptr<UIEvent> event) {
    UIGestureRecognizer::touchesCancelled(changed,event);
    if(std::none_of(changed.begin(),changed.end(),[&](const auto& t){return std::find(_touches.begin(),_touches.end(),t)!=_touches.end();}))return;
    if(state()==UIGestureRecognizerState::began||state()==UIGestureRecognizerState::changed)setState(UIGestureRecognizerState::cancelled);
    else if(state()==UIGestureRecognizerState::possible)setState(UIGestureRecognizerState::failed);
}
void UIPanGestureRecognizer::reset() {
    _touches.clear();_hasSample=false;_hasVelocity=false;_velocity={};_recognitionTravel={};
    // UIKit retains its final translation until the next touch sequence.
}
}
