#include "GlassInteraction.hpp"
#include "LensInteraction.hpp"
#include "LensMaterial.hpp"
#include "NativeLensProfile.hpp"
#include "LensForegroundComposition.hpp"
#include "CABackdropEffectLayer.hpp"
#include <typeinfo>
#include <BackdropEffectView.h>
#include <CADisplayLink.h>
#include <CATransaction.h>
#include <LensView.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <tools/IBTools.h>

using namespace NXKit;

namespace {
// Keep elastic geometry separate from the caller's model frame/application
// transform. Source-to-owner capture includes this affine, so a stationary
// full source row is not deformed a second time by the moving lens.
class LensMotionLayer final : public CALayer {
public:
  LensMotionLayer() = default;
  explicit LensMotionLayer(LensMotionLayer* source)
      : CALayer(source), _elastic(source->_elastic), canBoundResting(source->canBoundResting) {}
  std::shared_ptr<CALayer> copy() override {
    return new_shared<LensMotionLayer>(this);
  }
  std::function<bool()> canBoundResting;
  void renderSublayers(SkCanvas* canvas) override {
    Internal::LensForegroundComposition composition(canvas,bounds().size,
        Internal::boundedLensForegroundEnabled()&&canBoundResting&&canBoundResting());
    if(composition.isActive())if(auto* metrics=Internal::RenderPerformance::current)++metrics->boundedLensGroups;
    CALayer::renderSublayers(canvas);
  }
  NXAffineTransform renderingTransform() const override { return _elastic; }
  void setElasticTransform(NXAffineTransform value) {
    if(_elastic==value)return;
    _elastic = value;
    if (auto active = presentation())
      std::static_pointer_cast<LensMotionLayer>(active)->_elastic = value;
    setNeedsDisplay();
  }
private:
  NXAffineTransform _elastic = NXAffineTransform::identity;
};
class LensTouchObserver final : public UIGestureRecognizer {
public:
  std::function<void(UITouchPhase,const std::shared_ptr<UITouch>&)> observe;
  LensTouchObserver() { setCancelsTouchesInView(false); }
  bool canPrevent(const std::shared_ptr<UIGestureRecognizer>&) const override { return false; }
  bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const override { return false; }
  void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) override {
    for(const auto& touch:touches) observe(UITouchPhase::began,touch);
  }
  void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) override {
    for(const auto& touch:touches) observe(UITouchPhase::moved,touch);
  }
  void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) override {
    for(const auto& touch:touches) observe(UITouchPhase::ended,touch);
  }
  void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches,std::shared_ptr<UIEvent>) override {
    for(const auto& touch:touches) observe(UITouchPhase::cancelled,touch);
  }
};
NXPoint sampledVelocity(NXPoint current,NXPoint previous,double seconds) {
  if(!std::isfinite(seconds)||seconds<=0) return {};
  // Match the recognizer's minimum sampling interval; explicit velocity input
  // remains untouched for native trace replay and coalesced-event clients.
  seconds=std::max(seconds,1.0/240.0);
  const double maximum=std::numeric_limits<NXFloat>::max();
  const auto component=[&](double value) {
    return NXFloat(std::clamp(value/seconds,-maximum,maximum));
  };
  return {component(double(current.x)-previous.x),component(double(current.y)-previous.y)};
}
void validateInput(NXPoint value) {
  if(!std::isfinite(value.x)||!std::isfinite(value.y))
    throw std::invalid_argument("Lens interaction input must be finite");
}
struct MotionPose { NXPoint center; NXAffineTransform inverse; bool valid = true; };
MotionPose modelMotionPose(const std::shared_ptr<UIView>& view,const std::function<NXAffineTransform(const std::shared_ptr<CALayer>&)>& ancestorExtra) {
  MotionPose result;
  const auto first=view->layer()->presentationOrSelf();
  result.center={first->bounds().width()*.5f,first->bounds().height()*.5f};
  NXPoint basisX(1,0),basisY(0,1);
  for(auto current=view;current;current=current->superview().lock()) {
    const auto layer=current->layer()->presentationOrSelf();
    const auto transform=layer->transform();
    if(transform.m13!=0||transform.m23!=0||transform.m31!=0||transform.m32!=0||transform.m14!=0||transform.m24!=0||transform.m34!=0||transform.m44!=1) { result.valid=false; return result; }
    const auto a=layer->affineTransform();
    const auto linear=[&](NXPoint p) { return NXPoint(p.x*a.m11+p.y*a.m21,p.x*a.m12+p.y*a.m22); };
    if(current!=view) {
      const auto extra=ancestorExtra(layer);
      result.center=result.center.applying(extra);
      const auto apply=[&](NXPoint p) { return NXPoint(p.x*extra.m11+p.y*extra.m21,p.x*extra.m12+p.y*extra.m22); };
      basisX=apply(basisX); basisY=apply(basisY);
    }
    result.center-=NXPoint(layer->bounds().width()*layer->anchorPoint().x,layer->bounds().height()*layer->anchorPoint().y);
    result.center=result.center.applying(a)+layer->position();
    basisX=linear(basisX); basisY=linear(basisY);
    if(auto parent=current->superview().lock()) result.center-=parent->layer()->presentationOrSelf()->bounds().origin;
  }
  const double det=double(basisX.x)*basisY.y-double(basisX.y)*basisY.x;
  if(!Internal::LensInteraction::valid(result.center)||!std::isfinite(det)||std::abs(det)<1e-8) { result.valid=false; return result; }
  result.inverse={NXFloat(basisY.y/det),NXFloat(-basisX.y/det),NXFloat(-basisY.x/det),NXFloat(basisX.x/det),0,0};
  return result;
}
void validateGeometry(NXRect value) {
  if (!std::isfinite(value.origin.x) || !std::isfinite(value.origin.y) ||
      !std::isfinite(value.size.width) || !std::isfinite(value.size.height) ||
      value.size.width < 0 || value.size.height < 0)
    throw std::invalid_argument(
        "Lens geometry must be finite with nonnegative size");
}
} // namespace

struct LensView::Animation {
  Internal::GlassSpring progress;
  double materialElapsed=0;
  std::optional<NXFloat> controlledRestingOpacity;
  std::optional<NXFloat> controlledOpticalProgress;
  std::optional<NXFloat> controlledMaterialProgress;
  NXSize materialReferenceSize;
  bool controlledBackdropCreated=false, firstControlLift=false;
  std::vector<LiftCompletion> completions;
  bool interactive = false;
  InteractionSource source = InteractionSource::contact;
  Internal::LensInteraction motion;
  bool hasLifetime = false;
  double lastVelocityTime = 0;
  bool contact = false;
  bool direct = false;
  bool hadWindow = false;
  NXPoint origin;
  NXPoint location;
  NXPoint velocity;
  std::weak_ptr<UIView> parent;
  std::weak_ptr<UIWindow> window;
  std::weak_ptr<UITouch> touch;
  Timer lastTouchTimestamp;
  NXPoint lastTouchLocation;
  std::shared_ptr<LensTouchObserver> observer;
  std::shared_ptr<CADisplayLink> link;
  std::chrono::steady_clock::time_point lastFrame;
  std::chrono::steady_clock::time_point lastInput;
};

LensView::LensView(Style style)
    : UIView(NXRect(), new_shared<LensMotionLayer>()), _style(style), _restingHost(new_shared<UIView>()),
      _resting(new_shared<UIView>()), _content(new_shared<UIView>()),
      _warpBackdrop(new_shared<BackdropEffectView>(Internal::lensBackdropMaterial(style==Style::selection,0,{}))),
      _optics(new_shared<BackdropEffectView>(
          Internal::lensMaterial(style == Style::selection, 0))),
      _contact(new_shared<UIView>(NXRect(), new_shared<Internal::LensLayer>())),
      _animation(std::make_unique<Animation>()) {
  if (style != Style::regular && style != Style::selection)
    throw std::invalid_argument("Invalid lens style");
  std::static_pointer_cast<LensMotionLayer>(layer())->canBoundResting=[weak=weak_from_base<LensView>()] {
    const auto self=weak.lock();if(!self||self->_progress!=0)return false;
    const auto root=self->layer()->presentationOrSelf();
    const std::vector<std::shared_ptr<CALayer>> expected={self->_warpBackdrop->layer(),self->_restingHost->layer(),self->_optics->layer(),self->_content->layer(),self->_contact->layer()};
    if(root->_mask||root->_sublayers!=expected||!self->_optics->layer()->presentationOrSelf()->isHidden())return false;
    if(!self->_warpBackdrop->isHidden())return false;
    for(auto ancestor=root;ancestor;ancestor=ancestor->_superlayer.lock())if(!ancestor->_contentMasks.empty())return false;
    const auto host=self->_restingHost->layer()->presentationOrSelf(),contact=self->_contact->layer()->presentationOrSelf();
    if(!self->_resting->layer()->presentationOrSelf()->masksToBounds()
       ||host->_sublayers!=std::vector<std::shared_ptr<CALayer>>{self->_resting->layer()}
       ||(host->_shadowColor&&host->_shadowColor->a()>0&&host->_shadowOpacity>0)
       ||host->getFrame()!=root->bounds()||contact->getFrame()!=root->bounds()
       ||host->transform()!=NXTransform3DIdentity||contact->transform()!=NXTransform3DIdentity
       ||contact->_mask||!contact->_contentMasks.empty()||!contact->_sublayers.empty()||contact->_contents
       ||(contact->_shadowColor&&contact->_shadowColor->a()>0&&contact->_shadowOpacity>0))return false;
    const auto plain=[](auto&& recurse,const std::shared_ptr<CALayer>& model)->bool {
      const auto node=model->presentationOrSelf();const CALayer& concrete=*node;
      const auto pointEffect=std::dynamic_pointer_cast<CABackdropEffectLayer>(node);
      if((typeid(concrete)!=typeid(CALayer)&&(!pointEffect||!pointEffect->samplesOnlyLocalBackdrop()))
         ||node->_mask||!node->_contentMasks.empty()||node->transform()!=NXTransform3DIdentity)return false;
      for(const auto& child:node->_sublayers)if(!recurse(recurse,child))return false;
      return true;
    };
    if(!plain(plain,host))return false;
    // The source picture must also keep its sampling axes. Stop at the shared
    // ancestor; glass outside the source/owner paths does not affect that map.
    auto source=self->_liftedContent.lock();if(!source)source=self->_content;
    std::vector<std::shared_ptr<UIView>> owners;
    for(auto view=std::static_pointer_cast<UIView>(self);view;view=view->superview().lock())owners.push_back(view);
    for(auto view=source;view&&std::find(owners.begin(),owners.end(),view)==owners.end();view=view->superview().lock()) {
      const auto sourceLayer=view->layer()->presentationOrSelf();const CALayer& concrete=*sourceLayer;
      if(typeid(concrete)!=typeid(CALayer))return false;
      const auto t=sourceLayer->transform();
      if(t.m12!=0||t.m21!=0||t.m13!=0||t.m14!=0||t.m23!=0||t.m24!=0||t.m31!=0||t.m32!=0||t.m33!=1||t.m34!=0||t.m43!=0||t.m44!=1||t.m11<=0||t.m11!=t.m22)return false;
    }
    return true;
  };
  _restingHost->addSubview(_resting);
  // Native order: below-glass blur/displacement, caller's resting surface,
  // ClearGlass optics, then the selected-content portal and highlight.
  addSubview(_warpBackdrop);
  addSubview(_restingHost);
  addSubview(_optics);
  // The selected input has separate ownership from the ordinary backdrop.
  // Its optical source pass is emitted by the final lens layer, once.
  addSubview(_content);
  // Paint contact shading after the optical pass so the lens cannot refract
  // its own exterior contour back into the material as a second dark ring.
  addSubview(_contact);
  setSourceOwnership(_content->layer(),true);
  std::static_pointer_cast<Internal::LensLayer>(_contact->layer())->setSource(_content, shared_from_this());
  _restingHost->setUserInteractionEnabled(false);
  _warpBackdrop->setUserInteractionEnabled(false);
  _optics->setUserInteractionEnabled(false);
  _contact->setUserInteractionEnabled(false);
  _resting->setClipsToBounds(true);
  _content->setClipsToBounds(true);
  _optics->setClipsToBounds(false);
  updateDisplayScale();
  updateLayout();
  updateMaterial();
}
LensView::LensView(NXRect frame,Style style):LensView(style) {setFrame(frame);}
LensView::LensView(std::shared_ptr<UIView> restingBackground,Style style):LensView(style) {
  if(restingBackground) {
    _hasCustomRestingBackground=true;
    _resting->removeFromSuperview();_resting=std::move(restingBackground);
    _resting->setClipsToBounds(true);_restingHost->addSubview(_resting);updateLayout();
  }
}
NXAffineTransform LensView::interactionTransform() const {
  return std::static_pointer_cast<LensMotionLayer>(layer()->presentationOrSelf())
      ->renderingTransform();
}
LensView::~LensView() {
  setSourceOwnership(_content->layer(),false);
  setSourceOwnership(_registeredSource.lock(),false);
  if (_animation->link)
    _animation->link->invalidate();
}
bool LensView::isInteractive() const { return _animation->interactive; }
void LensView::setInteractive(bool interactive) {
  if(_animation->interactive==interactive) return;
  _animation->interactive=interactive;
  if(!interactive) { resetInteraction(); return; }
  if(!_animation->observer) {
    _animation->observer=new_shared<LensTouchObserver>();
    const auto weak=weak_from_base<LensView>();
    _animation->observer->observe=[weak](auto phase,const auto& touch) {
      if(const auto view=weak.lock()) view->observeTouch(phase,touch);
    };
    addGestureRecognizer(_animation->observer);
  }
  if(interactionSource()==InteractionSource::viewMotion&&_lifted) { prepareViewMotion(); wakeAnimation(); }
}
LensView::InteractionSource LensView::interactionSource() const { return _animation->source; }
void LensView::setInteractionSource(InteractionSource source) {
  if(source!=InteractionSource::contact&&source!=InteractionSource::viewMotion)
    throw std::invalid_argument("Invalid lens interaction source");
  if(_animation->source==source) return;
  resetInteraction(); _animation->source=source;
  if(isInteractive()&&source==InteractionSource::viewMotion&&_lifted) { prepareViewMotion(); wakeAnimation(); }
}
std::optional<std::pair<NXPoint,NXAffineTransform>> LensView::motionPose() {
  const auto pose=modelMotionPose(shared_from_this(),[](const auto& layer) { return layer->renderingTransform(); });
  if(!pose.valid) return std::nullopt;
  return std::pair(pose.center,pose.inverse);
}
void LensView::prepareViewMotion() {
  if(!isInteractive()||interactionSource()!=InteractionSource::viewMotion||superview().expired()) return;
  auto& state=*_animation;
  if(!state.hasLifetime) {
    state.parent=superview(); state.window=window(); state.hadWindow=!state.window.expired(); state.hasLifetime=true;
  }
  if(!state.motion.hasPosition) {
    const auto pose=motionPose();
    if(pose) state.motion.rebase(pose->first);
  }
}
void LensView::beginInteraction(NXPoint location) {
  validateInput(location);
  if(!isInteractive()||superview().expired()||bounds().width()<=0||bounds().height()<=0) return;
  auto& state=*_animation;
  state.contact=true; state.direct=false; state.touch.reset();
  state.origin=state.location=location; state.velocity={};
  state.lastInput=std::chrono::steady_clock::now();
  state.lastVelocityTime=state.motion.time; state.hasLifetime=true;
  state.parent=superview(); state.window=window(); state.hadWindow=!state.window.expired();
  if(state.source==InteractionSource::viewMotion) {
    const auto pose=motionPose(); if(pose) state.motion.rebase(pose->first);
  }
  wakeAnimation();
}
void LensView::updateInteraction(NXPoint location) {
  validateInput(location);
  if(!isInteractive()||!_animation->contact) return;
  const auto now=std::chrono::steady_clock::now();
  const double dt=std::chrono::duration<double>(now-_animation->lastInput).count();
  const auto velocity=dt>0 ? sampledVelocity(location,_animation->location,dt) : _animation->velocity;
  updateInteraction(location,velocity);
}
void LensView::updateInteraction(NXPoint location,NXPoint velocity) {
  validateInput(location); validateInput(velocity);
  if(!isInteractive()||!_animation->contact) return;
  _animation->location=location; _animation->velocity=velocity;
  _animation->lastVelocityTime=_animation->motion.time;
  _animation->lastInput=std::chrono::steady_clock::now();
  wakeAnimation();
}
void LensView::endInteraction(NXPoint velocity,bool cancelled) {
  validateInput(velocity);
  if(!_animation->contact) return;
  _animation->contact=false; _animation->direct=false; _animation->touch.reset();
  _animation->velocity=cancelled?NXPoint():velocity;
  _animation->lastVelocityTime=_animation->motion.time;
  wakeAnimation();
}
void LensView::applyInteractionTransform(NXAffineTransform transform) {
  std::static_pointer_cast<LensMotionLayer>(layer())->setElasticTransform(transform);
}
void LensView::resetInteraction() {
  auto& state=*_animation;
  state.contact=false; state.direct=false; state.touch.reset();
  state.parent.reset(); state.window.reset(); state.velocity={};
  state.hasLifetime=false; state.motion.reset();
  applyInteractionTransform(NXAffineTransform::identity);
  if(state.progress.atRest(_lifted ? 1 : 0) && state.link) {
    state.link->invalidate(); state.link.reset();
  }
}
void LensView::observeTouch(UITouchPhase phase,const std::shared_ptr<UITouch>& touch) {
  if(!isInteractive()||!touch) return;
  auto& state=*_animation;
  if(phase==UITouchPhase::began) {
    if(state.contact) return;
    const auto parent=superview().lock(); if(!parent) return;
    const auto local=touch->locationIn(shared_from_this())-bounds().origin;
    const auto size=bounds().size;
    if(size.width<=0||size.height<=0) return;
    const double radius=std::min(size.width,size.height)*.5;
    const double x=std::max(0.0,std::abs(double(local.x)-size.width*.5)-(size.width*.5-radius));
    const double y=std::max(0.0,std::abs(double(local.y)-size.height*.5)-(size.height*.5-radius));
    if(std::hypot(x,y)>radius) return;
    const auto location=touch->locationIn(parent);
    beginInteraction(location);
    if(!state.contact) return;
    state.direct=true; state.touch=touch;
    state.lastTouchLocation=location; state.lastTouchTimestamp=touch->timestamp();
  } else {
    if(!state.direct||state.touch.lock()!=touch) return;
    const auto parent=state.parent.lock(); if(!parent) { resetInteraction(); return; }
    const auto location=touch->locationIn(parent);
    const double dt=(touch->timestamp()-state.lastTouchTimestamp)/1000.0;
    const auto velocity=dt>0 ? sampledVelocity(location,state.lastTouchLocation,dt) : state.velocity;
    if(phase==UITouchPhase::moved) updateInteraction(location,velocity);
    else endInteraction(velocity,phase==UITouchPhase::cancelled);
    state.lastTouchLocation=location; state.lastTouchTimestamp=touch->timestamp();
  }
}
void LensView::setContentVibrancyEnabled(bool enabled) {
  if (_contentVibrancy == enabled) return;
  _contentVibrancy = enabled;
  updateMaterial();
}
void LensView::setLiftedContentMagnification(NXFloat magnification) {
  if (!std::isfinite(magnification) || magnification < 1)
    throw std::invalid_argument("Lifted content magnification must be finite and at least one");
  if (_liftedContentMagnification == magnification) return;
  _liftedContentMagnification = magnification;
  updateMaterial();
}
void LensView::setStyle(Style style) {
  if (style != Style::regular && style != Style::selection)
    throw std::invalid_argument("Invalid lens style");
  if (_style == style)
    return;
  _style = style;
  updateMaterial();
}
void LensView::setSourceOwnership(const std::shared_ptr<CALayer>& source, bool owned) {
  if (!source) return;
  auto owners = source->_lensSourceOwners;
  if (!owners && owned) owners = std::make_shared<std::vector<std::weak_ptr<CALayer>>>();
  if (!owners) return;
  std::erase_if(*owners,[&](const auto& owner) { return owner.expired() || owner.lock()==layer(); });
  if (owned) owners->push_back(layer());
  source->_lensSourceOwners = owners;
  if (auto active = source->presentation()) active->_lensSourceOwners = owners;
  source->setNeedsDisplay();
}
void LensView::setLiftedContentView(const std::shared_ptr<UIView> &source) {
  for (auto ancestor = shared_from_this(); ancestor; ancestor = ancestor->superview().lock())
    if (ancestor == source)
      throw std::invalid_argument("A lens content source cannot contain the lens");
  setSourceOwnership(_registeredSource.lock(),false);
  _registeredSource.reset();
  if (source && source != _content) {
    _registeredSource = source->layer();
    setSourceOwnership(source->layer(),_liftedContentMode==LiftedContentMode::hideSource);
  }
  _liftedContent = source;
  std::static_pointer_cast<Internal::LensLayer>(_contact->layer())->setSource(source ? source : _content, shared_from_this());
  layer()->setNeedsDisplay();
}
void LensView::setLiftedContentMode(LiftedContentMode mode) {
  if(mode!=LiftedContentMode::preserveSource&&mode!=LiftedContentMode::hideSource)
    throw std::invalid_argument("Invalid lifted content mode");
  if(_liftedContentMode==mode)return;
  _liftedContentMode=mode;
  setSourceOwnership(_registeredSource.lock(),mode==LiftedContentMode::hideSource);
  layer()->setNeedsDisplay();
}
std::optional<UIColor> LensView::restingBackgroundColor() const {
  return _restingBackgroundColor;
}
void LensView::setRestingBackgroundColor(std::optional<UIColor> color) {
  _restingBackgroundColor=color;
  if(_hasCustomRestingBackground)return;
  CATransaction::begin();
  CATransaction::setDisableActions(true);
  _resting->setBackgroundColor(color);
  _resting->layer()->presentationOrSelf()->setBackgroundColor(color);
  CATransaction::commit();
}
void LensView::setRestingShadowProperties(std::shared_ptr<UIShadowProperties> properties) {
  _restingShadow=std::move(properties);
  (_restingShadow?_restingShadow:std::make_shared<UIShadowProperties>())->applyToView(_restingHost);
  layer()->setNeedsDisplay();
}
void LensView::setRestingBorderColor(std::optional<UIColor> color) {
  _restingBorderColor=std::move(color);updateMaterial();
}
void LensView::setWarpsContentBelow(bool value) {
  if(_warpsContentBelow==value)return;
  _warpsContentBelow=value;updateMaterial();
}
void LensView::setLifted(bool lifted, bool animated) {
  _animation->controlledRestingOpacity.reset();
  _animation->controlledOpticalProgress.reset();
  _animation->controlledMaterialProgress.reset();
  const bool changed = _lifted != lifted;
  _lifted = lifted;
  if(lifted&&!_animation->controlledBackdropCreated) {
    _animation->controlledBackdropCreated=true;_animation->firstControlLift=true;
  }
  if(!lifted)_animation->firstControlLift=false;
  if(changed) {
    _animation->materialElapsed=0;
    if(!lifted)_animation->progress.velocity=0;
    else if(_style==Style::small) {
      const auto omega=2*std::numbers::pi/.27;
      _animation->progress.velocity+=(1-_animation->progress.value)*omega*omega*.032;
    }
  }
  // Native material distances retire from their lifted reference geometry,
  // even while the caller's presentation bounds shrink underneath them.
  if(lifted || _animation->materialReferenceSize.width<=0 || _animation->materialReferenceSize.height<=0)
    _animation->materialReferenceSize=bounds().size;
  if(isInteractive()&&interactionSource()==InteractionSource::viewMotion) prepareViewMotion();
  if (!animated) {
    _animation->progress = {lifted ? 1.0 : 0.0, 0};
    _progress = lifted ? 1 : 0;
    updateMaterial();
    if(isInteractive()&&interactionSource()==InteractionSource::viewMotion) prepareViewMotion();
    if(_animation->contact||!_animation->motion.atRest()||(isInteractive()&&interactionSource()==InteractionSource::viewMotion&&lifted)) wakeAnimation();
    else if(_animation->link) { _animation->link->invalidate(); _animation->link.reset(); }
    auto completions=std::move(_animation->completions);_animation->completions.clear();
    for(const auto& completion:completions)completion(true,false);
    return;
  }
  if (!changed) return;
  wakeAnimation();
}
void LensView::setLifted(bool lifted,bool animated,const std::function<void()>& alongsideAnimations,LiftCompletion completion) {
  if(_lifted==lifted) {if(alongsideAnimations)alongsideAnimations();return;}
  setLifted(lifted,animated);
  if(alongsideAnimations) {
    if(animated)UIView::animate(_style==Style::large?.25:(_lifted?.25:.46),0,
        UIViewAnimationOptions(UIViewAnimationOptions::allowUserInteraction|UIViewAnimationOptions::beginFromCurrentState),alongsideAnimations);
    else UIView::performWithoutAnimation(alongsideAnimations);
  }
  if(completion) {
    if(animated)_animation->completions.push_back(std::move(completion));
    else completion(true,false);
  }
}
void LensView::setControlLift(bool lifted, NXFloat progress, NXFloat restingOpacity, NXFloat opticalProgress, NXSize materialReferenceSize) {
  const auto materialProgress=progress;
  progress=std::clamp(progress,0.f,1.f);
  restingOpacity=std::clamp(restingOpacity,0.f,1.f);
  const bool changed=_lifted!=lifted || _progress!=progress
      || _animation->controlledRestingOpacity!=restingOpacity || _animation->controlledOpticalProgress!=opticalProgress
      || _animation->controlledMaterialProgress!=materialProgress || _animation->materialReferenceSize!=materialReferenceSize;
  if(lifted&&!_animation->controlledBackdropCreated) {
    _animation->controlledBackdropCreated=true;_animation->firstControlLift=true;
  }
  if(!lifted)_animation->firstControlLift=false;
  _lifted=lifted;
  _progress=progress;
  _animation->progress={progress,0};
  _animation->controlledRestingOpacity=restingOpacity;
  _animation->controlledOpticalProgress=opticalProgress;
  _animation->controlledMaterialProgress=materialProgress;
  _animation->materialReferenceSize=materialReferenceSize;
  if(changed)updateMaterial();
  if(isInteractive()&&interactionSource()==InteractionSource::viewMotion) {
    prepareViewMotion();
    if(lifted || !_animation->motion.atRest())wakeAnimation();
  }
}
void LensView::wakeAnimation() {
  if(_animation->link) return;
  _animation->lastFrame = std::chrono::steady_clock::now();
  const auto weak = weak_from_base<LensView>();
  _animation->link = std::make_shared<CADisplayLink>([weak] {
    if (const auto view = weak.lock()) {
      const auto now = std::chrono::steady_clock::now();
      const auto elapsed =
          std::chrono::duration<double>(now - view->_animation->lastFrame)
              .count();
      view->_animation->lastFrame = now;
      view->advance(elapsed);
    }
  });
  CALayer::requestFramerate(120);
}
void LensView::advance(double seconds) {
  if (!std::isfinite(seconds) || seconds <= 0)
    return;
  auto& state=*_animation;
  if(state.hasLifetime) {
    bool invalid=state.parent.lock()!=superview().lock()
        || (state.hadWindow && state.window.lock()!=window())
        || (state.direct && state.touch.expired())
        || bounds().width()<=0 || bounds().height()<=0;
    for(auto view=shared_from_this();view;view=view->superview().lock())
      invalid=invalid||view->isHidden()||view->alpha()<.01f
          ||(view.get()!=this&&!view->isUserInteractionEnabled());
    if(invalid) resetInteraction();
  }
  if(isInteractive() && state.hasLifetime) {
    const auto size=layer()->presentationOrSelf()->bounds().size;
    if(state.source==InteractionSource::viewMotion) {
      const auto pose=motionPose();
      if(pose) state.motion.advancePosition(pose->first,seconds,size,pose->second);
      else resetInteraction();
    } else {
      NXPoint velocity=state.motion.time+seconds-state.lastVelocityTime<=Internal::LensInteraction::contactHysteresis ? state.velocity : NXPoint();
      const auto a=layer()->presentationOrSelf()->affineTransform();
      const double det=double(a.m11)*a.m22-double(a.m12)*a.m21;
      if(std::isfinite(det)&&std::abs(det)>1e-8) velocity={NXFloat((velocity.x*a.m22-velocity.y*a.m21)/det),NXFloat((-velocity.x*a.m12+velocity.y*a.m11)/det)};
      else velocity={};
      state.motion.advanceVelocity(velocity,seconds,size);
    }
    applyInteractionTransform(state.motion.transform(size));
  }
  const auto previousProgress=_progress;
  const auto previousMaterialProgress=state.progress.value;
  if(!state.controlledRestingOpacity) {
    state.materialElapsed+=seconds;
    const bool large=_style==Style::selection;
    _animation->progress.advance(_lifted ? 1 : 0, seconds, _lifted?(large?.25:.27):.4, _lifted&&!large?.375:0);
    // Both captured native controls terminate this material return at .48s;
    // do not keep submitting imperceptible analytic-tail frames after that.
    if(!_lifted && state.materialElapsed>=.48)state.progress={0,0};
    _progress = std::clamp(NXFloat(_animation->progress.value), NXFloat(0), NXFloat(1));
  }
  if(_progress!=previousProgress || state.progress.value!=previousMaterialProgress) updateMaterial();
  if(!state.controlledRestingOpacity && state.progress.atRest(_lifted?1:0) && !state.completions.empty()) {
    auto completions=std::move(state.completions);state.completions.clear();
    for(const auto& completion:completions)completion(true,false);
  }
  if (!_animation->contact && _animation->motion.atRest() && (state.controlledRestingOpacity || _animation->progress.atRest(_lifted ? 1 : 0))
      && !(isInteractive()&&interactionSource()==InteractionSource::viewMotion&&_lifted&&_animation->hasLifetime)) {
    if (_animation->link)
      _animation->link->invalidate();
    _animation->link.reset();
  } else
    CALayer::requestFramerate(120);
}
void LensView::updateMaterial() {
  _materialSize = bounds().size;
  CATransaction::begin();
  CATransaction::setDisableActions(true);
  const auto restingOpacity=_animation->controlledRestingOpacity.value_or(1 - _progress);
  _restingHost->setAlpha(restingOpacity);
  _restingHost->layer()->presentationOrSelf()->setOpacity(restingOpacity);
  for(const auto& target:{layer(),_optics->layer()}) {
    const auto width=_restingBorderColor&&!_lifted?3.f:0.f;
    target->setBorderColor(_restingBorderColor);target->setBorderWidth(width);
    target->presentationOrSelf()->setBorderColor(_restingBorderColor);target->presentationOrSelf()->setBorderWidth(width);
  }
  _optics->setHidden(_progress == 0);
  _optics->layer()->presentationOrSelf()->setHidden(_progress == 0);
  _warpBackdrop->setHidden(_progress==0);
  _warpBackdrop->layer()->presentationOrSelf()->setHidden(_warpBackdrop->isHidden());
  if(!_warpBackdrop->isHidden()) {
    const auto optical=_animation->controlledOpticalProgress.value_or(_progress);
    std::optional<NXFloat> displacement;
    if(!_warpsContentBelow) {
      const auto initial=Internal::nativeLensProfile(_style==Style::selection,false,bounds().size).spec.unliftedDisplacement;
      displacement=_animation->firstControlLift?initial*(1-optical):0.f;
    }
    _warpBackdrop->setEffect(Internal::lensBackdropMaterial(_style==Style::selection,optical,bounds().size,displacement));
  }
  const auto traits = traitCollection();
  const bool dark =
      traits && traits->userInterfaceStyle() == UIUserInterfaceStyle::dark;
  _optics->setEffect(
      Internal::lensMaterial(_style == Style::selection, _animation->controlledMaterialProgress.value_or(NXFloat(_animation->progress.value)), dark, bounds().size, false,
          _animation->materialReferenceSize.width>0&&_animation->materialReferenceSize.height>0 ? std::optional<NXSize>(_animation->materialReferenceSize) : std::nullopt, _lifted));
  std::static_pointer_cast<Internal::LensLayer>(_contact->layer())
      ->setLiftState(_style == Style::selection, _progress, _liftedContentMagnification, _contentVibrancy, dark);
  CATransaction::commit();
  layer()->setNeedsDisplay();
}
void LensView::updateLayout() {
  const auto size = bounds().size;
  if(_lifted && !_animation->controlledMaterialProgress)_animation->materialReferenceSize=size;
  const auto radius =
      std::max(NXFloat(0), std::min(size.width, size.height) * .5f);
  const NXRect local(0, 0, size.width, size.height);
  layer()->setCornerRadius(radius);
  for (const auto &view :
       {_restingHost, _content, std::static_pointer_cast<UIView>(_warpBackdrop), std::static_pointer_cast<UIView>(_optics),
        _contact}) {
    view->setFrame(bounds());
    view->layer()->setCornerRadius(radius);
  }
  _resting->setFrame(local);
  _resting->layer()->setCornerRadius(radius);
  if (_materialSize != size) updateMaterial();
}
void LensView::setFrame(NXRect frame) {
  validateGeometry(frame);
  prepareViewMotion();
  UIView::setFrame(frame);
  updateLayout();
  if(isInteractive()&&interactionSource()==InteractionSource::viewMotion) wakeAnimation();
}
void LensView::setBounds(NXRect bounds) {
  validateGeometry(bounds);
  prepareViewMotion();
  UIView::setBounds(bounds);
  updateLayout();
  if(isInteractive()&&interactionSource()==InteractionSource::viewMotion) wakeAnimation();
}
void LensView::layoutSubviews() {
  UIView::layoutSubviews();
  updateLayout();
}
void LensView::traitCollectionDidChange(
    std::shared_ptr<UITraitCollection> previous) {
  updateDisplayScale();
  UIView::traitCollectionDidChange(std::move(previous));
  updateMaterial();
}
void LensView::updateDisplayScale() {
  const auto traits = traitCollection();
  const auto candidate = traits ? traits->displayScale() : 1;
  const auto scale = std::isfinite(candidate) && candidate > 0 ? candidate : 1;
  if (_resolvedDisplayScale == scale)
    return;
  _resolvedDisplayScale = scale;
  CATransaction::begin();
  CATransaction::setDisableActions(true);
  layer()->setContentsScale(scale);
  layer()->presentationOrSelf()->setContentsScale(scale);
  _contact->layer()->setContentsScale(scale);
  _contact->layer()->presentationOrSelf()->setContentsScale(scale);
  CATransaction::commit();
}
bool LensView::applyXMLAttribute(const std::string &name,
                                 const std::string &value) {
  if(name=="warpsContentBelow") {
    const auto parsed=valueToBool(value);if(!parsed)return false;
    setWarpsContentBelow(*parsed);return true;
  }
  if (name == "style") {
    if (value == "regular" || value == "small")
      setStyle(Style::regular);
    else if (value == "selection" || value == "large")
      setStyle(Style::selection);
    else
      return false;
    return true;
  }
  if(name=="liftedContentMode") {
    if(value=="preserveSource")setLiftedContentMode(LiftedContentMode::preserveSource);
    else if(value=="hideSource")setLiftedContentMode(LiftedContentMode::hideSource);
    else return false;
    return true;
  }
  if(name=="interactionSource") {
    if(value=="contact") setInteractionSource(InteractionSource::contact);
    else if(value=="viewMotion") setInteractionSource(InteractionSource::viewMotion);
    else return false;
    return true;
  }
  if (name == "isInteractive") {
    const auto parsed=valueToBool(value);
    if(!parsed) return false;
    setInteractive(*parsed); return true;
  }
  if (name == "contentVibrancyEnabled") {
    const auto parsed=valueToBool(value);if(!parsed)return false;
    setContentVibrancyEnabled(*parsed);return true;
  }
  if (name == "liftedContentMagnification") {
    const auto parsed=valueToFloat(value);
    if (!parsed || !std::isfinite(*parsed) || *parsed < 1) return false;
    setLiftedContentMagnification(*parsed); return true;
  }
  if (name == "isLifted") {
    const auto parsed = valueToBool(value);
    if (!parsed)
      return false;
    setLifted(*parsed);
    return true;
  }
  if (name == "restingBackgroundColor") {
    const auto parsed = valueToColor(value);
    if (!parsed)
      return false;
    setRestingBackgroundColor(*parsed);
    return true;
  }
  if(name=="restingBorderColor") {
    const auto parsed=valueToColor(value);if(!parsed)return false;
    setRestingBorderColor(*parsed);return true;
  }
  if(name=="restingShadowColor" || name=="restingShadowRadius" || name=="restingShadowOpacity" || name=="restingShadowOffsetX" || name=="restingShadowOffsetY") {
    auto properties=_restingShadow?_restingShadow->copy():std::make_shared<UIShadowProperties>();
    if(name=="restingShadowColor") {
      const auto color=valueToColor(value);if(!color)return false;properties->setColor(*color);
    } else {
      const auto number=valueToFloat(value);if(!number || !std::isfinite(*number))return false;
      if(name=="restingShadowRadius"){if(*number<0)return false;properties->setRadius(*number);}
      else if(name=="restingShadowOpacity")properties->setOpacity(*number);
      else {auto offset=properties->offset();if(name=="restingShadowOffsetX")offset.width=*number;else offset.height=*number;properties->setOffset(offset);}
    }
    setRestingShadowProperties(properties);return true;
  }
  return UIView::applyXMLAttribute(name, value);
}
