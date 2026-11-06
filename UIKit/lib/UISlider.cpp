#include <UISlider.h>
#include <LensView.h>
#include <UIImageView.h>
#include <UILabel.h>
#include <UITouch.h>
#include <UIPress.h>
#include <tools/IBTools.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <CADisplayLink.h>
#include <chrono>
#include "GlassInteraction.hpp"
#include "SliderThumbTransition.hpp"
#include <CATransaction.h>

namespace NXKit {
namespace {
void finite(NXFloat value) { if(!std::isfinite(value))throw std::invalid_argument("Slider values must be finite"); }
void geometry(NXRect rect) {
    finite(rect.minX());finite(rect.minY());finite(rect.width());finite(rect.height());
    if(rect.width()<0 || rect.height()<0)throw std::invalid_argument("Slider geometry must be nonnegative");
}
// UIKit's fluid track layout tapers the effective cap width over the outer
// two percent of each normalized endpoint. The fill endpoint is quadratic in
// that interval and meets the ordinary thumb-center mapping continuously.
NXFloat trackEndpoint(NXFloat progress,NXFloat width,NXFloat thumbWidth) {
    const double u=std::clamp(double(progress),0.,1.);
    const double w=std::max(0.,double(width));
    if(w==0)return 0;
    const double taper=std::clamp(std::min(u/.02,(1-u)/.02),0.,1.);
    const double cap=std::max(0.,double(thumbWidth))*taper;
    return NXFloat(std::clamp(cap*.5+u*(w-cap),0.,w));
}
using Images=std::map<UIControlState,std::shared_ptr<UIImage>>;
std::shared_ptr<UIImage> imageFor(const Images& images,UIControlState state) {
    if(auto it=images.find(state);it!=images.end())return it->second;
    if(auto it=images.find(UIControlState::normal);it!=images.end())return it->second;
    return nullptr;
}
void setImage(Images& images,std::shared_ptr<UIImage> image,UIControlState state) {
    if(image)images[state]=std::move(image);else images.erase(state);
}
}
struct UISlider::Tracking {
    std::shared_ptr<CADisplayLink> link;
    std::shared_ptr<CADisplayLink> thumbLink;
    std::chrono::steady_clock::time_point lastThumbFrame;
    Internal::SliderThumbTransition thumb;
    bool pendingUnlift=false;
    std::chrono::steady_clock::time_point lastFrame;
    Internal::GlassSpring spring;
    double elapsed=0;
    NXFloat target=0, velocity=0;
    NXPoint previousPoint;
    Timer previousTime;
    NXFloat previousVelocity=0;
    bool hasVelocity=false, advancing=false;
};
UISlider::~UISlider() = default;
UISlider::UISlider(NXRect frame,std::optional<UIAction> action):UIControl(frame),_interaction(std::make_unique<Tracking>()) {
    geometry(frame);
    // Unlike UIControl's primary initializer, UISlider maps this action to
    // valueChanged, as the native subclass documents.
    if(action)addAction(*action,UIControlEvent::valueChanged);
    _visuals=new_shared<UIView>();_visuals->setAutolayoutEnabled(false);addSubview(_visuals);
    _track=new_shared<UIView>();_track->setClipsToBounds(true);
    _maximumTrack=new_shared<UIView>();_minimumTrack=new_shared<UIView>();
    _minimumTrackImage=new_shared<UIImageView>();_maximumTrackImage=new_shared<UIImageView>();
    _minimumImage=new_shared<UIImageView>();_maximumImage=new_shared<UIImageView>();
    _thumbImage=new_shared<UIImageView>();_lens=new_shared<LensView>(LensView::Style::regular);
    _lens->setInteractive(true);_lens->setInteractionSource(LensView::InteractionSource::viewMotion);
    _lens->setWarpsContentBelow(false);
    // Native resting shadow belongs to the fill branch, not the lens root.
    // Its host opacity retires the shadow with the white surface; its live
    // capsule geometry follows the size spring throughout lowering.
    auto shadow=std::make_shared<UIShadowProperties>();shadow->setColor(UIColor::black);
    shadow->setOpacity(.14f);shadow->setRadius(5);shadow->setOffset({0,1});_lens->setRestingShadowProperties(shadow);
    _track->addSubview(_maximumTrack);_track->addSubview(_minimumTrack);
    _maximumTrack->addSubview(_maximumTrackImage);_minimumTrack->addSubview(_minimumTrackImage);
    _visuals->addSubview(_track);_visuals->addSubview(_minimumImage);_visuals->addSubview(_maximumImage);_visuals->addSubview(_lens);_visuals->addSubview(_thumbImage);
    // Keep the containing visual host interactive: LensView validates its
    // ancestor chain before advancing flex. Hit testing stays with the control.
    for(const auto& child:_visuals->subviews()) { child->setUserInteractionEnabled(false);child->setAutolayoutEnabled(false); }
    const auto weak=weak_from_base<UISlider>();
    for(const auto direction:{-1,1}) {
        UIResponderAction command;
        command.identifier=direction<0?"UISlider.decrement":"UISlider.increment";
        command.action=UIAction(command.identifier,[weak,direction]{if(auto self=weak.lock())self->adjustValue(direction);});
        command.action.setRepeatBehavior(UIMenuElementRepeatBehavior::repeatable);
        command.matches=[direction](const auto& press){return press && press->type()==(direction<0?UIPressType::leftArrow:UIPressType::rightArrow);};
        command.canPerform=[weak]{const auto self=weak.lock();return self && self->isEnabled() && self->_maximumValue>self->_minimumValue;};
        registerAction(std::move(command));
    }
    updateAppearance();
}
NXFloat UISlider::fraction(NXFloat value) const {
    const double span=double(_maximumValue)-_minimumValue;
    return span>0?NXFloat(std::clamp((double(value)-_minimumValue)/span,0.,1.)):0;
}
NXFloat UISlider::constrainedValue(NXFloat value,bool interactive)const {
    value=std::clamp(value,_minimumValue,_maximumValue);
    if(!interactive || !_configuration)return value;
    const auto low=std::clamp(_configuration->minimumEnabledValue(),_minimumValue,_maximumValue);
    const auto high=std::clamp(_configuration->maximumEnabledValue(),low,_maximumValue);
    value=std::clamp(value,low,high);
    if(_configuration->allowsTickValuesOnly()) {
        std::optional<NXFloat> nearest;
        for(const auto& tick:_configuration->ticks())if(tick->position()>=low && tick->position()<=high)
            if(!nearest || std::abs(double(tick->position())-value)<std::abs(double(*nearest)-value))nearest=tick->position();
        if(nearest)value=*nearest;
    }
    return value;
}
void UISlider::setValue(NXFloat value,bool animated) {
    finite(value);value=constrainedValue(value,false);
    if(_value==value) {if(!animated && !isTracking())stopValueAnimation();return;}
    const auto distance=std::abs(fraction(value)-fraction(_value));_value=value;
    if(animated && distance>0)UIView::animate(.25*distance,0,UIViewAnimationOptions(UIViewAnimationOptions::curveLinear|UIViewAnimationOptions::allowUserInteraction),[&]{layoutSubviews();});
    else {stopValueAnimation();layoutSubviews();}
    if(_interaction->link && !_interaction->advancing) {
        _interaction->link->invalidate();_interaction->link.reset();completeTracking();
    }
}
void UISlider::stopValueAnimation() {
    if(!_lens)return;
    _lens->layer()->removeAnimation("position");_thumbImage->layer()->removeAnimation("position");
    _minimumTrack->layer()->removeAnimation("position");_minimumTrack->layer()->removeAnimation("bounds");
}
void UISlider::setMinimumValue(NXFloat value) {
    finite(value);if(_minimumValue==value)return;cancelTracking();_minimumValue=value;_maximumValue=std::max(value,_maximumValue);
    _value=constrainedValue(_value,false);setNeedsLayout();layoutSubviews();
}
void UISlider::setMaximumValue(NXFloat value) {
    finite(value);if(_maximumValue==value)return;cancelTracking();_maximumValue=value;_minimumValue=std::min(value,_minimumValue);
    _value=constrainedValue(_value,false);setNeedsLayout();layoutSubviews();
}
void UISlider::setSliderStyle(UISliderStyle style) {
    if(style!=UISliderStyle::defaultStyle && style!=UISliderStyle::thumbless)throw std::invalid_argument("Invalid slider style");
    if(_style==style)return;cancelTracking();_style=style;updateAppearance();
}
void UISlider::setTrackConfiguration(const std::shared_ptr<UISliderTrackConfiguration>& configuration) {
    _configuration=configuration?configuration->copy():nullptr;rebuildTicks();updateAppearance();
}
std::shared_ptr<UIImage> UISlider::minimumValueImage()const{return _minimumImage->image();}
std::shared_ptr<UIImage> UISlider::maximumValueImage()const{return _maximumImage->image();}
void UISlider::setMinimumValueImage(const std::shared_ptr<UIImage>& image){_minimumImage->setImage(image);setNeedsLayout();layoutSubviews();}
void UISlider::setMaximumValueImage(const std::shared_ptr<UIImage>& image){_maximumImage->setImage(image);setNeedsLayout();layoutSubviews();}
void UISlider::setMinimumTrackTintColor(std::optional<UIColor> color){_minimumTrackTint=std::move(color);updateAppearance();}
void UISlider::setMaximumTrackTintColor(std::optional<UIColor> color){_maximumTrackTint=std::move(color);updateAppearance();}
void UISlider::setThumbTintColor(std::optional<UIColor> color){_thumbTint=std::move(color);updateAppearance();}
void UISlider::setThumbImage(const std::shared_ptr<UIImage>& image,UIControlState state){setImage(_thumbImages,image,state);updateAppearance();}
void UISlider::setMinimumTrackImage(const std::shared_ptr<UIImage>& image,UIControlState state){setImage(_minimumTrackImages,image,state);updateAppearance();}
void UISlider::setMaximumTrackImage(const std::shared_ptr<UIImage>& image,UIControlState state){setImage(_maximumTrackImages,image,state);updateAppearance();}
std::shared_ptr<UIImage> UISlider::thumbImageForState(UIControlState state)const{return imageFor(_thumbImages,state);}
std::shared_ptr<UIImage> UISlider::minimumTrackImageForState(UIControlState state)const{return imageFor(_minimumTrackImages,state);}
std::shared_ptr<UIImage> UISlider::maximumTrackImageForState(UIControlState state)const{return imageFor(_maximumTrackImages,state);}
std::shared_ptr<UIImage> UISlider::currentThumbImage()const{return thumbImageForState(state());}
std::shared_ptr<UIImage> UISlider::currentMinimumTrackImage()const{return minimumTrackImageForState(state());}
std::shared_ptr<UIImage> UISlider::currentMaximumTrackImage()const{return maximumTrackImageForState(state());}
NXRect UISlider::minimumValueImageRectForBounds(NXRect bounds)const {
    geometry(bounds);
    const auto image=minimumValueImage();if(!image)return {};
    const auto size=image->size();return {bounds.minX(),bounds.midY()-size.height*.5f,size.width,size.height};
}
NXRect UISlider::maximumValueImageRectForBounds(NXRect bounds)const {
    geometry(bounds);
    const auto image=maximumValueImage();if(!image)return {};
    const auto size=image->size();return {bounds.maxX()-size.width,bounds.midY()-size.height*.5f,size.width,size.height};
}
NXRect UISlider::trackRectForBounds(NXRect bounds)const {
    geometry(bounds);
    const auto left=minimumValueImage()?minimumValueImage()->size().width+11:0;
    const auto right=maximumValueImage()?maximumValueImage()->size().width+11:0;
    const auto height=_style==UISliderStyle::thumbless?7.f:6.f;
    return {bounds.minX()+left,bounds.midY()-height*.5f,std::max<NXFloat>(0,bounds.width()-left-right),height};
}
NXRect UISlider::thumbRectForBounds(NXRect bounds,NXRect track,NXFloat value)const {
    geometry(bounds);geometry(track);finite(value);const auto image=currentThumbImage();const auto size=image?image->size():NXSize{37,24};
    return {std::round(track.minX()+fraction(value)*std::max<NXFloat>(0,track.width()-size.width)),std::round(bounds.midY()-size.height*.5f),size.width,size.height};
}
NXSize UISlider::sizeThatFits(NXSize size){return {std::max<NXFloat>(0,size.width),34};}
void UISlider::setFrame(NXRect frame){geometry(frame);UIControl::setFrame(frame);}
void UISlider::setBounds(NXRect bounds){geometry(bounds);UIControl::setBounds(bounds);}
bool UISlider::point(NXPoint p,UIEvent*) {
    // A 34pt native intrinsic height remains usable inside a 44pt touch row.
    return bounds().insetBy({std::max(0.f,(44-bounds().height())*.5f),0,std::max(0.f,(44-bounds().height())*.5f),0}).contains(p);
}
std::shared_ptr<UIView> UISlider::hitTest(NXPoint p,UIEvent* event) {
    return UIControl::hitTest(p,event)?shared_from_this():nullptr;
}
void UISlider::updateAppearance() {
    if(!_lens)return;
    _minimumTrack->setBackgroundColor(_minimumTrackTint.value_or(tintColor()));
    _maximumTrack->setBackgroundColor(_maximumTrackTint.value_or(UIColor::label.withAlphaComponent(.1f)));
    _minimumTrackImage->setImage(currentMinimumTrackImage());_maximumTrackImage->setImage(currentMaximumTrackImage());
    _minimumTrackImage->setHidden(!_minimumTrackImage->image());_maximumTrackImage->setHidden(!_maximumTrackImage->image());
    if(_minimumTrackImage->image())_minimumTrack->setBackgroundColor(UIColor::clear);
    if(_maximumTrackImage->image())_maximumTrack->setBackgroundColor(UIColor::clear);
    _thumbImage->setImage(currentThumbImage());
    _thumbImage->setHidden(_style==UISliderStyle::thumbless || !_thumbImage->image());
    _lens->setHidden(_style==UISliderStyle::thumbless || bool(currentThumbImage()));
    _lens->setRestingBackgroundColor(_thumbTint.value_or(UIColor::white));
    const auto alpha=isEnabled()?1.f:.5f;
    _visuals->setAlpha(alpha);
    _minimumImage->setTintColor(UIColor::secondaryLabel);_maximumImage->setTintColor(UIColor::secondaryLabel);
    layoutSubviews();
}
void UISlider::updateThumb() {
    if(!_lens)return;
    auto rect=thumbRectForBounds(bounds(),trackRectForBounds(bounds()),_value);
    _thumbImage->setFrame(rect);
    const auto materialReferenceSize=rect.size*1.55f;
    const bool hasLens=_style==UISliderStyle::defaultStyle && !currentThumbImage();
    const auto progress=hasLens?NXFloat(_interaction->thumb.size.value):0.f;
    if(progress!=0) {
        const auto size=rect.size*(1+.55f*progress);
        const auto track=trackRectForBounds(bounds());
        const auto center=track.minX()+rect.width()*.5f+fraction(_value)*std::max(0.f,track.width()-rect.width());
        const auto x=rect.midX()+(center-rect.midX())*progress;
        const auto y=rect.midY()+(bounds().midY()-rect.midY())*progress;
        rect={x-size.width*.5f,y-size.height*.5f,size.width,size.height};
    }
    const auto glass=hasLens?std::clamp(NXFloat(_interaction->thumb.glass.value),0.f,1.f):0.f;
    _lens->setFrame(rect);
    _lens->setControlLift(_interaction->thumb.lifted&&hasLens,hasLens?NXFloat(_interaction->thumb.glass.value):0,1-glass,
        NXFloat(_interaction->thumb.optics.value),materialReferenceSize);
}
void UISlider::layoutSubviews() {
    UIView::layoutSubviews();if(!_lens)return;
    _visuals->setFrame(bounds());_visuals->setBounds(bounds());
    const auto track=trackRectForBounds(bounds());_track->setFrame(track);_track->layer()->setCornerRadius(track.height()*.5f);
    _maximumTrack->setFrame({0,0,track.width(),track.height()});
    const auto thumb=thumbRectForBounds(bounds(),track,_value);
    const auto amount=fraction(_value);
    const auto capWidth=_style==UISliderStyle::thumbless?0.f:thumb.width();
    const auto fill=trackEndpoint(amount,track.width(),capWidth);
    // Public trackConfiguration.neutralValue does not anchor the filled bar
    // on the observed native Glass provider. Configured sliders still fill
    // from the minimum; the private fluid neutralPosition is a separate input.
    _minimumTrack->setFrame({0,0,fill,track.height()});
    _minimumTrack->layer()->setCornerRadius(track.height()*.5f);
    _minimumTrackImage->setFrame(_minimumTrack->bounds());_maximumTrackImage->setFrame(_maximumTrack->bounds());
    _minimumImage->setFrame(minimumValueImageRectForBounds(bounds()));_maximumImage->setFrame(maximumValueImageRectForBounds(bounds()));
    updateThumb();
    if(isTracking() && _lastLayoutSize!=bounds().size) {
        _trackingBaseValue=_value;_dragOrigin=_lastTouch;_dragging=true;
    }
    _lastLayoutSize=bounds().size;
    if(_configuration) {
        const auto ticks=_configuration->ticks();
        for(std::size_t i=0;i<_ticks.size() && i<ticks.size();++i) {
            if(!_ticks[i])continue;
            const auto& tick=ticks[i];const auto rect=thumbRectForBounds(bounds(),track,tick->position());
            const auto size=(tick->image()||tick->title())?_ticks[i]->sizeThatFits({60,24}):NXSize(2,6);
            _ticks[i]->setFrame({rect.midX()-size.width*.5f,track.maxY()+8,size.width,size.height});
        }
    }
}
void UISlider::setPressed(bool pressed,bool animated) {
    if(_pressed==pressed)return;_pressed=pressed;
    auto& motion=*_interaction;
    const bool lifted=pressed&&_style==UISliderStyle::defaultStyle&&!currentThumbImage();
    if(!animated) {motion.pendingUnlift=false;motion.thumb.finish(lifted);}
    else if(!lifted && motion.thumb.lifted && motion.thumb.elapsed<.22)motion.pendingUnlift=true;
    else {motion.pendingUnlift=false;motion.thumb.retarget(lifted);}
    updateThumb();
    if((motion.thumb.atRest()&&!motion.pendingUnlift) || motion.thumbLink)return;
    motion.lastThumbFrame=std::chrono::steady_clock::now();
    const auto weak=weak_from_base<UISlider>();
    motion.thumbLink=std::make_shared<CADisplayLink>([weak] {
        if(auto self=weak.lock()) {
            const auto now=std::chrono::steady_clock::now();
            const auto seconds=std::chrono::duration<double>(now-self->_interaction->lastThumbFrame).count();
            self->_interaction->lastThumbFrame=now;self->advanceThumb(seconds);
        }
    });
    CALayer::requestFramerate(120);
}
void UISlider::advanceThumb(double seconds) {
    if(!std::isfinite(seconds)||seconds<=0)return;
    auto& motion=*_interaction;
    if(motion.pendingUnlift) {
        const auto held=std::min(seconds,std::max(0.,.22-motion.thumb.elapsed));
        motion.thumb.advance(held);seconds-=held;
        if(motion.thumb.elapsed>=.22) {motion.pendingUnlift=false;motion.thumb.retarget(false);}
    }
    if(seconds>0)motion.thumb.advance(seconds);
    CATransaction::begin();CATransaction::setDisableActions(true);
    updateThumb();
    CATransaction::commit();
    if(motion.thumb.atRest()&&!motion.pendingUnlift) {
        if(motion.thumbLink)motion.thumbLink->invalidate();
        motion.thumbLink.reset();
    } else CALayer::requestFramerate(120);
}
bool UISlider::beginTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&) {
    if(!touch || !isEnabled() || _maximumValue<=_minimumValue)return false;
    _lastTouch=touch->locationIn(shared_from_this());
    const auto track=trackRectForBounds(bounds());
    const auto visibleLayer=(currentThumbImage()?_thumbImage->layer():_lens->layer())->presentationOrSelf();
    const auto visibleThumb=visibleLayer->getFrame();
    if(_style==UISliderStyle::defaultStyle && !visibleThumb.insetBy({10,10,10,10}).contains(_lastTouch))return false;
    if(_style==UISliderStyle::defaultStyle && visibleLayer!=(currentThumbImage()?_thumbImage->layer():_lens->layer())) {
        const auto rest=thumbRectForBounds(bounds(),track,_value);
        const auto travel=std::max(0.f,track.width()-rest.width());
        if(travel>0)setValue(NXFloat(double(_minimumValue)+(double(_maximumValue)-_minimumValue)*std::clamp((visibleThumb.midX()-track.minX()-rest.width()*.5f)/travel,0.f,1.f)));
    }
    _trackingBaseValue=_value;
    _dragOrigin=_lastTouch;_dragging=false;
    _interaction->previousPoint=_lastTouch;_interaction->previousTime=touch->timestamp();
    _interaction->hasVelocity=false;_interaction->velocity=_interaction->previousVelocity=0;
    _lens->beginInteraction(_lastTouch);setPressed(true,true);
    if(_style==UISliderStyle::thumbless)updateFromTouch(touch);
    return true;
}
void UISlider::updateFromTouch(const std::shared_ptr<UITouch>& touch) {
    if(!touch)return;
    const auto p=touch->locationIn(shared_from_this());_lastTouch=p;
    const auto track=trackRectForBounds(bounds());
    auto& motion=*_interaction;
    const auto delta=p.x-motion.previousPoint.x;
    if(delta!=0 || touch->phase()==UITouchPhase::moved) {
        const auto dt=(touch->timestamp()-motion.previousTime)/1000.;
        if(dt>0) {
            const auto instantaneous=std::clamp(NXFloat(delta/dt),-10000.f,10000.f);
            motion.velocity=motion.hasVelocity?motion.previousVelocity*.8f+instantaneous*.2f:instantaneous;
            motion.previousVelocity=instantaneous;motion.hasVelocity=true;
        }
        motion.previousPoint=p;motion.previousTime=touch->timestamp();
    }
    NXFloat candidate=_value;
    if(_style==UISliderStyle::thumbless) {
        const auto ratio=track.width()>0?std::clamp((p.x-track.minX())/track.width(),0.f,1.f):0;
        candidate=NXFloat(double(_minimumValue)+(double(_maximumValue)-_minimumValue)*ratio);
    } else {
        if(!_dragging) {
            const auto distance=p.x-_dragOrigin.x;
            if(std::abs(distance)<=10)return;
            _dragOrigin.x+=std::copysign(10.f,distance);_dragging=true;
        }
        if(track.width()>0)candidate=NXFloat(double(_trackingBaseValue)+(double(_maximumValue)-_minimumValue)*(p.x-_dragOrigin.x)/track.width());
    }
    candidate=constrainedValue(candidate,true);
    const bool changed=candidate!=_value;setValue(candidate);_lens->updateInteraction(p);
    if(changed && _continuous)sendActions(UIControlEvent::valueChanged);
}
bool UISlider::continueTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&){updateFromTouch(touch);return true;}
bool UISlider::deferEndTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&) {
    updateFromTouch(touch);
    if(!isTracking())return false;
    auto& motion=*_interaction;
    // Native value projection and lens lowering overlap after finger-up.
    // Keep tracking alive for terminal delivery, independently of elevation;
    // setPressed preserves a short contact's minimum lift interval.
    _lens->endInteraction({motion.velocity,0});setPressed(false,true);setHighlighted(false);
    if(!isTracking())return false;
    // Native public pan-velocity probes project with decelerationRate .99:
    // distance = velocity * .99 / (1000 * (1-.99)). Preserve that last
    // moved-sample velocity through a stationary hold, as UIKit does.
    const auto projected=std::abs(motion.velocity)>100.f?.099f*motion.velocity:0.f;
    const auto width=trackRectForBounds(bounds()).width();
    motion.target=constrainedValue(width>0?NXFloat(double(_value)+(double(_maximumValue)-_minimumValue)*projected/width):_value,true);
    if(motion.target==_value)return false;
    motion.spring={_value,0};motion.elapsed=0;motion.lastFrame=std::chrono::steady_clock::now();
    const auto weak=weak_from_base<UISlider>();
    motion.link=std::make_shared<CADisplayLink>([weak] {
        if(auto self=weak.lock()) {
            const auto now=std::chrono::steady_clock::now();
            const auto dt=std::chrono::duration<double>(now-self->_interaction->lastFrame).count();
            self->_interaction->lastFrame=now;self->advanceRelease(dt);
        }
    });
    CALayer::requestFramerate(120);return true;
}
void UISlider::advanceRelease(double seconds) {
    auto& motion=*_interaction;
    if(!motion.link || !isTracking())return;
    if(!std::isfinite(seconds) || seconds<=0)return;
    motion.elapsed+=seconds;motion.spring.advance(motion.target,seconds,.4,0);
    const bool finished=motion.elapsed>=.4;
    const auto next=finished?motion.target:constrainedValue(NXFloat(motion.spring.value),true);
    const bool changed=next!=_value;motion.advancing=true;setValue(next);motion.advancing=false;
    if(changed && _continuous)sendActions(UIControlEvent::valueChanged);
    if(!motion.link || !isTracking())return;
    if(finished) {motion.link->invalidate();motion.link.reset();completeTracking();}
    else CALayer::requestFramerate(120);
}
void UISlider::endTracking(const std::shared_ptr<UITouch>&,const std::shared_ptr<UIEvent>&) {
    if(_interaction->link){_interaction->link->invalidate();_interaction->link.reset();}
    _lens->endInteraction();setPressed(false,true);
}
void UISlider::trackingDidEnd() {
    // UIKit sends a final valueChanged after touch-up, including a stationary
    // contact. Continuous tracking may already have sent intermediate values.
    sendActions(UIControlEvent::valueChanged);
}
void UISlider::cancelTracking(const std::shared_ptr<UIEvent>& event) {
    if(_interaction && _interaction->link){_interaction->link->invalidate();_interaction->link.reset();}
    if(_lens){
        _lens->endInteraction({},true);
        _interaction->pendingUnlift=false;
        // Cancellation is terminal, including during a short tap's minimum
        // lift interval. It must not leave a disabled control visually held.
        _interaction->thumb.elapsed=std::max(.22,_interaction->thumb.elapsed);
        _interaction->thumb.retarget(false);
        setPressed(false,true);updateThumb();
    }
    UIControl::cancelTracking(event);
}
void UISlider::adjustValue(NXFloat direction) {
    auto next=NXFloat(double(_value)+direction*(double(_maximumValue)-_minimumValue)*.01);
    if(_configuration && _configuration->allowsTickValuesOnly()) {
        std::optional<NXFloat> adjacent;
        for(const auto& tick:_configuration->ticks()) {
            const auto p=tick->position();if((direction>0 && p>_value) || (direction<0 && p<_value))
                if(!adjacent || (direction>0?p<*adjacent:p>*adjacent))adjacent=p;
        }
        if(adjacent)next=*adjacent;
    }
    next=constrainedValue(next,true);if(next==_value)return;
    setValue(next,true);sendActions(UIControlEvent::valueChanged);
}
void UISlider::rebuildTicks() {
    for(const auto& tick:_ticks)if(tick)tick->removeFromSuperview();_ticks.clear();if(!_configuration)return;
    for(const auto& tick:_configuration->ticks()) {
        std::shared_ptr<UIView> view;
        if(tick->image())view=new_shared<UIImageView>(tick->image());
        else if(tick->title()) {auto label=new_shared<UILabel>();label->setText(*tick->title());label->setFontSize(12);label->setTextColor(UIColor::secondaryLabel);view=label;}
        else {_ticks.push_back(nullptr);continue;} // Native unlabeled ticks add no resting decoration.
        view->setUserInteractionEnabled(false);view->setAutolayoutEnabled(false);_visuals->insertSubviewBelow(view,_lens);_ticks.push_back(std::move(view));
    }
}
void UISlider::accessibilityIncrement(){if(isEnabled()&&isActionDispatchAllowed())adjustValue(1);}
void UISlider::accessibilityDecrement(){if(isEnabled()&&isActionDispatchAllowed())adjustValue(-1);}
void UISlider::controlStateDidChange(){updateAppearance();}
void UISlider::tintColorDidChange(){UIControl::tintColorDidChange();updateAppearance();}
void UISlider::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous){UIControl::traitCollectionDidChange(std::move(previous));updateAppearance();}
bool UISlider::applyXMLAttribute(const std::string& name,const std::string& value) {
    if(name=="value" || name=="minimumValue" || name=="maximumValue") {
        const auto parsed=valueToFloat(value);if(!parsed || !std::isfinite(*parsed))return false;
        if(name=="value")setValue(*parsed);if(name=="minimumValue")setMinimumValue(*parsed);if(name=="maximumValue")setMaximumValue(*parsed);return true;
    }
    if(name=="continuous"){if(auto v=valueToBool(value)){setContinuous(*v);return true;}return false;}
    if(name=="sliderStyle"){if(value=="default"){setSliderStyle(UISliderStyle::defaultStyle);return true;}if(value=="thumbless"){setSliderStyle(UISliderStyle::thumbless);return true;}return false;}
    if(name=="minimumTrackTintColor" || name=="maximumTrackTintColor" || name=="thumbTintColor") {
        const auto color=value=="nil"?std::optional<UIColor>{}:valueToColor(value);if(!color && value!="nil")return false;
        if(name=="minimumTrackTintColor")setMinimumTrackTintColor(color);if(name=="maximumTrackTintColor")setMaximumTrackTintColor(color);if(name=="thumbTintColor")setThumbTintColor(color);return true;
    }
    return UIControl::applyXMLAttribute(name,value);
}
}
