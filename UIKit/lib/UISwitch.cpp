#include <UISwitch.h>
#include <LensView.h>
#include <UIImage.h>
#include <UITouch.h>
#include <UIPress.h>
#include <CADisplayLink.h>
#include <CATransaction.h>
#include <tools/IBTools.h>
#include "GlassInteraction.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>

namespace NXKit {
namespace {
// Sliding-provider metrics, independent of the containing row's dimensions.
const NXSize switchSize{63,28};
const NXSize intrinsicSwitchSize{61,28};
const NXSize restingThumbSize{37,24};
const NXSize liftedThumbSize{58,38.333333f};
void validateGeometry(NXRect rect) {
    if(!std::isfinite(rect.minX()) || !std::isfinite(rect.minY()) ||
       !std::isfinite(rect.width()) || !std::isfinite(rect.height()) ||
       rect.width()<0 || rect.height()<0)throw std::invalid_argument("Invalid switch geometry");
}
}
// UIKit's modern switch uses private tap/pan recognizers rather than the
// UIControl tracking path: isTracking/isHighlighted remain false, taps send
// valueChanged then touchUpInside, and pans send only valueChanged.
class UISwitchGestureRecognizer final:public UIGestureRecognizer {
public:
    explicit UISwitchGestureRecognizer(std::weak_ptr<UISwitch> owner):_owner(std::move(owner)){setCancelsTouchesInView(false);}
    bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&)const override {
        const auto owner=_owner.lock();return !owner || !owner->_acceptedContact;
    }
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event)override {
        if(_touch || touches.empty())return;
        const auto owner=_owner.lock();
        if(!owner || !owner->isEnabled() || !owner->isActionDispatchAllowed() || !owner->beginTracking(touches.front(),event)){
            setState(UIGestureRecognizerState::failed);return;
        }
        _touch=touches.front();setState(UIGestureRecognizerState::began);
    }
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event)override {
        if(!_touch || std::find(touches.begin(),touches.end(),_touch)==touches.end())return;
        if(const auto owner=_owner.lock();owner && owner->_acceptedContact && owner->isEnabled()){
            owner->continueTracking(_touch,event);setState(UIGestureRecognizerState::changed);
        }
    }
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event)override {
        if(!_touch || std::find(touches.begin(),touches.end(),_touch)==touches.end())return;
        auto touch=std::move(_touch);_touch.reset();
        if(const auto owner=_owner.lock();owner && owner->isEnabled())owner->endTracking(touch,event);
        setState(UIGestureRecognizerState::ended);
    }
    void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent> event)override {
        if(!_touch || std::find(touches.begin(),touches.end(),_touch)==touches.end())return;
        _touch.reset();if(const auto owner=_owner.lock())owner->cancelTracking(event);
        setState(UIGestureRecognizerState::cancelled);
    }
protected:
    void reset()override{const bool active=bool(_touch);_touch.reset();if(active)if(const auto owner=_owner.lock())owner->cancelTracking();}
private:
    std::weak_ptr<UISwitch> _owner;
    std::shared_ptr<UITouch> _touch;
};
struct UISwitch::Motion {
    Internal::GlassSpring position, elevation;
    bool pressed=false;
    double elapsed=0,positionElapsed=0;
    std::shared_ptr<CADisplayLink> link;
    std::chrono::steady_clock::time_point lastFrame;
};

UISwitch::UISwitch(NXRect frame,std::optional<UIAction> action)
    :UIControl({frame.origin,switchSize}),_motion(std::make_unique<Motion>()) {
    validateGeometry(frame);
    // At construction the only installed recognizer is UIControl's private
    // tracker; replace it before any input can observe the view.
    gestureRecognizers()->clear();
    auto gesture=new_shared<UISwitchGestureRecognizer>(weak_from_base<UISwitch>());
    _switchGesture=gesture;addGestureRecognizer(gesture);
    if(action)addAction(*action,UIControlEvent::valueChanged);
    _visuals=new_shared<UIView>();_visuals->setAutolayoutEnabled(false);addSubview(_visuals);
    _track=new_shared<UIView>();_track->setAutolayoutEnabled(false);_track->setUserInteractionEnabled(false);_visuals->addSubview(_track);
    _lens=new_shared<LensView>(LensView::Style::small);
    _lens->setAutolayoutEnabled(false);_lens->setUserInteractionEnabled(false);
    _lens->setInteractive(true);_lens->setInteractionSource(LensView::InteractionSource::viewMotion);
    _lens->setWarpsContentBelow(true);
    // The switch provider leaves resting shadow properties nil; lifted optics
    // and their retirement are owned by LensView's shared material path.
    _visuals->addSubview(_lens);
    const auto weak=weak_from_base<UISwitch>();
    UIResponderAction command;command.identifier="UISwitch.toggle";command.inputIdentifier=UIResponderActionInputSelect;
    command.matches=[](const auto& p){return p && p->type()==UIPressType::select && !p->isRepeat();};
    command.canPerform=[weak]{const auto self=weak.lock();return self && self->isEnabled() && self->isActionDispatchAllowed();};
    command.action=UIAction("Toggle",[weak]{if(auto self=weak.lock()){self->activate();self->sendActions(UIControlEvent::primaryActionTriggered);}});
    registerAction(std::move(command));layoutSubviews();updateAppearance();
}
UISwitch::~UISwitch()=default;

void UISwitch::setOn(bool on,bool animated) {
    const bool changed=_on!=on;_on=on;
    if(!changed && animated)return;
    if(!animated) {
        _position=on?1:0;_motion->position={_position,0};
        if(!_motion->pressed){
            _motion->elevation={0,0};_lens->setLifted(false,false);
            if(_motion->link)_motion->link->invalidate();_motion->link.reset();
        }
        updateGeometry();return;
    }
    _motion->elapsed=0;_motion->positionElapsed=0;
    wakeAnimation();
}
void UISwitch::setOnTintColor(std::optional<UIColor> color){_onTintColor=std::move(color);updateAppearance();}
void UISwitch::setThumbTintColor(std::optional<UIColor> color){_thumbTintColor=std::move(color);updateAppearance();}
// These appearance properties are retained by modern UIKit's sliding provider;
// they do not replace the modern track or thumb artwork.
void UISwitch::setOnImage(std::shared_ptr<UIImage> image){_onImage=std::move(image);}
void UISwitch::setOffImage(std::shared_ptr<UIImage> image){_offImage=std::move(image);}
void UISwitch::setPreferredStyle(UISwitchStyle value) {
    if(value<UISwitchStyle::automatic || value>UISwitchStyle::sliding)throw std::invalid_argument("Invalid switch style");
    _preferredStyle=value;
}
NXSize UISwitch::sizeThatFits(NXSize){return intrinsicSwitchSize;}
void UISwitch::sizeToFit(){setFrame(frame());}
void UISwitch::setFrame(NXRect value){validateGeometry(value);UIControl::setFrame({value.origin,switchSize});}
void UISwitch::setBounds(NXRect value){validateGeometry(value);UIControl::setBounds(value);}
bool UISwitch::point(NXPoint p,UIEvent*) {
    const auto b=bounds();
    return b.insetBy({std::max(0.f,(44-b.height())*.5f),0,std::max(0.f,(44-b.height())*.5f),0}).contains(p);
}
std::shared_ptr<UIView> UISwitch::hitTest(NXPoint p,UIEvent* event){return UIControl::hitTest(p,event)?shared_from_this():nullptr;}
void UISwitch::updateAppearance() {
    if(!_lens)return;
    _lens->setRestingBackgroundColor(_thumbTintColor.value_or(UIColor::white));
    _visuals->setAlpha(isEnabled()?1.f:.5f);updateGeometry();
}
void UISwitch::updateGeometry() {
    if(!_lens)return;
    const auto b=_visuals->bounds();
    const auto track=NXRect{b.midX()-switchSize.width*.5f,b.midY()-switchSize.height*.5f,switchSize.width,switchSize.height};
    _track->setFrame(track);_track->layer()->setCornerRadius(track.height()*.5f);
    const auto traits=traitCollection();
    const auto off=UIColor::tertiaryLabel.resolvedColorWithTraitCollection(traits);
    const auto on=_onTintColor.value_or(UIColor::systemGreen).resolvedColorWithTraitCollection(traits);
    _track->setBackgroundColor(off.interpolationTo(on,_position));
    const auto elevation=NXFloat(_motion->elevation.value);
    const NXSize size{restingThumbSize.width+(liftedThumbSize.width-restingThumbSize.width)*elevation,
        restingThumbSize.height+(liftedThumbSize.height-restingThumbSize.height)*elevation};
    const auto travel=track.width()-restingThumbSize.width-4;
    const auto x=track.minX()+2+restingThumbSize.width*.5f+travel*_position;
    _lens->setFrame({x-size.width*.5f,track.midY()-size.height*.5f,size.width,size.height});
}
void UISwitch::layoutSubviews() {
    UIView::layoutSubviews();if(!_visuals)return;
    _visuals->setFrame(bounds());_visuals->setBounds(bounds());updateGeometry();
}
void UISwitch::setPressed(bool pressed,bool animated) {
    if(_motion->pressed==pressed)return;
    _motion->pressed=pressed;_motion->elapsed=0;
    _lens->setLifted(pressed,animated);
    if(!animated){_motion->elevation={pressed?1.:0.,0};updateGeometry();}
    else {
        const auto omega=2*std::numbers::pi/(pressed?.27:.5);
        _motion->elevation.velocity+=(double(pressed)-_motion->elevation.value)*omega*omega*(pressed?.032:.016);
        wakeAnimation();
    }
}
void UISwitch::wakeAnimation() {
    if(_motion->link)return;
    _motion->lastFrame=std::chrono::steady_clock::now();
    const auto weak=weak_from_base<UISwitch>();
    _motion->link=std::make_shared<CADisplayLink>([weak]{if(const auto self=weak.lock()) {
        const auto now=std::chrono::steady_clock::now();
        const auto dt=std::chrono::duration<double>(now-self->_motion->lastFrame).count();
        self->_motion->lastFrame=now;self->advance(dt);
    }});CALayer::requestFramerate(120);
}
void UISwitch::advance(double seconds) {
    if(!std::isfinite(seconds) || seconds<=0)return;
    auto& m=*_motion;m.elapsed+=seconds;
    const double elevation=m.pressed?1:0;
    m.elevation.advance(elevation,seconds,m.pressed?.27:.5,m.pressed?.375:.3);
    if(m.elapsed>=(m.pressed?.25:.46))m.elevation={elevation,0};
    // Native public setOn:animated: uses a separate critically damped .3s
    // position response; it does not share the well-image spring or lift.
    if(!_acceptedContact){
        m.positionElapsed+=seconds;m.position.advance(_on?1:0,seconds,.3,0);
        // Native samples retire the final position tail between .3283 and
        // .3450s. One third of a second matches that bounded completion;
        // unlike the response itself, the exact native cutoff is unexposed.
        if(m.positionElapsed>=1./3)m.position={_on?1.:0.,0};
        _position=std::clamp(NXFloat(m.position.value),0.f,1.f);
    }
    if(m.elapsed>=.6){m.elevation={elevation,0};if(!_acceptedContact){m.position={_on?1.:0.,0};_position=_on?1:0;}}
    CATransaction::begin();CATransaction::setDisableActions(true);updateGeometry();CATransaction::commit();
    if(m.elevation.atRest(elevation) && (_acceptedContact || m.position.atRest(_on?1:0))){
        if(m.link)m.link->invalidate();m.link.reset();
    }else CALayer::requestFramerate(120);
}
bool UISwitch::beginTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&) {
    if(!touch || !isEnabled())return false;
    _dragOrigin=touch->locationIn(shared_from_this());_dragged=false;_acceptedContact=true;
    const auto presented=_lens->layer()->presentationOrSelf()->getFrame();
    const auto travel=switchSize.width-restingThumbSize.width-4;
    const auto base=_visuals->bounds().midX()-switchSize.width*.5f+2+restingThumbSize.width*.5f;
    _trackingStartPosition=std::clamp((presented.midX()-base)/travel,0.f,1.f);
    _position=_trackingStartPosition;_motion->position={_position,0};
    _lens->beginInteraction(_dragOrigin);setPressed(true,true);return true;
}
bool UISwitch::continueTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&) {
    if(!touch)return false;
    const auto p=touch->locationIn(shared_from_this());
    const auto delta=p.x-_dragOrigin.x;
    if(std::abs(delta)>3)_dragged=true;
    if(_dragged) {
        const auto travel=switchSize.width-restingThumbSize.width-4;
        _position=std::clamp(_trackingStartPosition+delta/travel,0.f,1.f);
        _motion->position={_position,0};updateGeometry();
    }
    _lens->updateInteraction(p);return true;
}
void UISwitch::endTracking(const std::shared_ptr<UITouch>& touch,const std::shared_ptr<UIEvent>&) {
    if(!_acceptedContact)return;
    const bool inside=touch && point(touch->locationIn(shared_from_this()),nullptr);
    const bool wasTap=!_dragged && inside;
    const bool target=_dragged?_position>=.5f:(inside?!_on:_on);
    _acceptedContact=false;_lens->endInteraction();setPressed(false,true);
    const bool changed=target!=_on;_on=target;_motion->position={_position,0};_motion->positionElapsed=0;wakeAnimation();
    if(changed)sendActions(UIControlEvent::valueChanged);
    if(wasTap && isEnabled() && isActionDispatchAllowed())sendActions(UIControlEvent::touchUpInside);
}
void UISwitch::cancelTracking(const std::shared_ptr<UIEvent>& event) {
    const bool accepted=_acceptedContact;
    if(_lens && accepted) {
        _acceptedContact=false;_lens->endInteraction({},true);setPressed(false,true);
        _motion->position={_position,0};_motion->positionElapsed=0;wakeAnimation();
    }
    UIControl::cancelTracking(event);
    if(accepted) {
        if(const auto gesture=_switchGesture.lock()) {
            const bool enabled=gesture->isEnabled();gesture->setEnabled(false);gesture->setEnabled(enabled);
        }
        sendActions(UIControlEvent::touchCancel);
    }
}
void UISwitch::activate(){if(!isEnabled() || !isActionDispatchAllowed())return;setOn(!_on,true);sendActions(UIControlEvent::valueChanged);}
bool UISwitch::accessibilityActivate(){if(!isEnabled() || !isActionDispatchAllowed())return false;activate();return true;}
void UISwitch::controlStateDidChange(){updateAppearance();}
void UISwitch::tintColorDidChange(){UIControl::tintColorDidChange();updateAppearance();}
void UISwitch::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous){UIControl::traitCollectionDidChange(std::move(previous));updateAppearance();}
bool UISwitch::applyXMLAttribute(const std::string& name,const std::string& value) {
    if(name=="on"){const auto parsed=valueToBool(value);if(!parsed)return false;setOn(*parsed);return true;}
    if(name=="onTintColor" || name=="thumbTintColor") {
        const auto color=value=="nil"?std::optional<UIColor>{}:valueToColor(value);if(!color && value!="nil")return false;
        if(name=="onTintColor")setOnTintColor(color);else setThumbTintColor(color);return true;
    }
    if(name=="onImage" || name=="offImage") {
        const auto image=value=="nil"?std::optional<std::shared_ptr<UIImage>>{nullptr}:valueResToImage(value);
        if(!image)return false;
        if(name=="onImage")setOnImage(*image);else setOffImage(*image);return true;
    }
    if(name=="preferredStyle") {
        if(value=="automatic")setPreferredStyle(UISwitchStyle::automatic);
        else if(value=="sliding")setPreferredStyle(UISwitchStyle::sliding);
        else if(value=="checkbox")setPreferredStyle(UISwitchStyle::checkbox);
        else return false;return true;
    }
    return UIControl::applyXMLAttribute(name,value);
}
}
