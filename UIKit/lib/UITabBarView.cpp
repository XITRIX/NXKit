#include <UITabBarView.h>
#include "UITabBarViewInternal.hpp"
#include "TabSelectionMaterial.hpp"
#include "BarItemAccess.hpp"
#include "LayerCornerGeometry.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <UITouch.h>
#include <CADisplayLink.h>
#include <CABasicAnimation.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
using namespace NXKit;
namespace {
template<class Matrix> SkPoint mapLensPoint(const Matrix& matrix,NXPoint point) {
    if constexpr(requires { matrix.mapPoint(SkPoint::Make(point.x,point.y)); })
        return matrix.mapPoint(SkPoint::Make(point.x,point.y));
    else return matrix.mapXY(point.x,point.y);
}
SkMatrix lensToPlatter(const std::shared_ptr<LensView>& lens) {
    const auto layer=lens->layer()->presentationOrSelf();
    const auto size=layer->bounds().size;
    const auto position=layer->position(),anchor=layer->anchorPoint();
    auto matrix=CATransform3DMakeTranslation(position.x,position.y,0)
        .concat(layer->transform()).toSkM44().asM33();
    matrix.preTranslate(-size.width*anchor.x,-size.height*anchor.y);
    matrix.preConcat(NXTransform3DMakeAffineTransform(lens->interactionTransform()).toSkM44().asM33());
    return matrix;
}
// UIKit's lens punches the ordinary row out beneath its selected-content portal.
// Keep that composition in the containing control, rather than hiding arbitrary
// siblings inside the reusable LensView.
class NormalRowMask final : public CALayer {
public:
    std::weak_ptr<LensView> lens;
    NormalRowMask() = default;
    explicit NormalRowMask(NormalRowMask* old): CALayer(old), lens(old->lens) {}
    std::shared_ptr<CALayer> copy() override { return new_shared<NormalRowMask>(this); }
    void draw(SkCanvas* canvas) override {
        SkPathBuilder path;
        path.setFillType(SkPathFillType::kEvenOdd);
        path.addRect(SkRect::MakeWH(bounds().width(),bounds().height()));
        if (auto view=lens.lock(); view && !view->isHidden()) {
            const auto layer=view->layer()->presentationOrSelf();
            const auto size=layer->bounds().size;
            const auto matrix=lensToPlatter(view);
            const auto silhouette=Internal::layerCornerPath(size,layer->cornerRadius(),layer->cornerCurve());
            path.addPath(silhouette.makeTransform(matrix));
        }
        SkPaint paint; paint.setColor(SK_ColorWHITE); paint.setAntiAlias(true);
        canvas->drawPath(path.detach(),paint);
    }
};
// Preserve the entire contact lifetime without competing with tab controls or
// the selection pan. Coordinates stay in the stationary platter's space.
class LensContactObserver final : public UIGestureRecognizer {
public:
    std::function<void(UITouchPhase,const std::shared_ptr<UITouch>&)> observe;
    LensContactObserver() { setCancelsTouchesInView(false); }
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
class TabControl final : public UIControl {
public:
    bool point(NXPoint p,UIEvent*) override {return bounds().insetBy({0,4,0,4}).contains(p);}
    bool canBecomeFocused() override { return isEnabled() && UIControl::canBecomeFocused(); }
    std::function<void(bool)> onHighlight;
    void willChangeHighlight(bool highlighted) override { if (onHighlight) onHighlight(highlighted); }
    void willGainFocus() override { layer()->setBorderWidth(1); layer()->setBorderColor(UIColor::tint); }
    void willLoseFocus() override { layer()->setBorderWidth(0); }
};
}
UITabBarView::UITabBarView(NXRect frame) : UIView(frame),
    _state(std::make_unique<Internal::TabBarViewState>()) {
    UIGlassEffect material; material.setInteractive(true);
    _state->_glass=new_shared<BackdropEffectView>(material); addSubview(_state->_glass);
    _state->_normal=new_shared<UIView>(); _state->_glass->addSubview(_state->_normal);
    // Both complete icon rows stay fixed in platter coordinates. The selected
    // row sits below the lens, which owns its ordinary-output suppression and
    // samples its unmasked child so details outside the silhouette can refract
    // into it while dragging between items.
    _state->_selectedHost=new_shared<UIView>(); _state->_selectedHost->setUserInteractionEnabled(false);
    // The optical pass composites the source once, without using alpha/hidden
    // to fake a portal (those properties must retain their normal meaning).
    _state->_glass->addSubview(_state->_selectedHost);
    _state->_selectedContent=new_shared<UIView>(); _state->_selectedContent->setUserInteractionEnabled(false); _state->_selectedHost->addSubview(_state->_selectedContent);
    _state->_lens=new_shared<LensView>(LensView::Style::selection); _state->_lens->setUserInteractionEnabled(false);
    _state->_lens->setInteractive(true);
    _state->_lens->setContentVibrancyEnabled(true);
    _state->_lens->setInteractionSource(LensView::InteractionSource::viewMotion);
    _state->_selectionFill=new_shared<TabSelectionBackground>();
    _state->_lens->restingBackgroundView()->addSubview(_state->_selectionFill);_state->_glass->addSubview(_state->_lens);
    const auto weak = weak_from_base<UITabBarView>();
    auto pan=new_shared<UIPanGestureRecognizer>();
    _state->pan=pan;
    pan->onStateChanged=[weak](const auto& recognizer) {
        const auto self=weak.lock(); if (!self || self->_state->resetting || !self->_state->_count) return;
        const auto state=recognizer->state();
        if (state==UIGestureRecognizerState::began) {
            if(self->_state->_trackingTouch.expired()) return;
            self->_state->_dragging=true;
            // The touch-down approach animation gives way to direct tracking.
            self->_state->_lens->layer()->removeAnimation("position");
        }
        if (state==UIGestureRecognizerState::began || state==UIGestureRecognizerState::changed) {
            if(const auto touch=self->_state->_trackingTouch.lock()) self->setSelectionPosition(self->positionForFinger(touch->locationIn(self->_state->_glass)));
        }
        if (self->_state->_dragging && state==UIGestureRecognizerState::ended) { self->_state->_dragging=false; self->activateIndex(size_t(std::round(self->_state->_position))); self->releaseLift(); }
        if (self->_state->_dragging && (state==UIGestureRecognizerState::cancelled || state==UIGestureRecognizerState::failed)) { self->_state->_dragging=false; self->commitSelection(self->selectedItem(),true); self->releaseLift(true); }
    };
    addGestureRecognizer(pan);
    auto contact=new_shared<LensContactObserver>();
    contact->observe=[weak](auto phase,const auto& touch) { if(auto self=weak.lock()) self->observeTouch(phase,touch); };
    addGestureRecognizer(contact);
    installItems({},false);
}
void UITabBarView::observeTouch(UITouchPhase phase,const std::shared_ptr<UITouch>& touch) {
    if(!touch || !_state->_count || _state->resetting) return;
    const auto point=touch->locationIn(_state->_glass);
    if(!std::isfinite(point.x)||!std::isfinite(point.y)) return;
    if(phase==UITouchPhase::began) {
        if(!_state->_trackingTouch.expired()) return;
        // The platter is a capsule; its transparent corners are not contacts.
        const auto size=_state->_glass->bounds().size;
        const auto radius=std::min(size.width,size.height)*.5f;
        const auto x=std::max(0.0f,std::abs(point.x-size.width*.5f)-(size.width*.5f-radius));
        const auto y=std::max(0.0f,std::abs(point.y-size.height*.5f)-(size.height*.5f-radius));
        if(size.width<=0||size.height<=0||x*x+y*y>radius*radius) return;
        _state->_trackingTouch=touch; _state->_lastContact=point; _state->_lastContactTime=touch->timestamp();
        _state->_dragStart=_state->_selected;
        const auto lensLayer=_state->_lens->layer()->presentationOrSelf();
        const auto lensSize=lensLayer->bounds().size;
        const auto placement=lensToPlatter(_state->_lens);
        const auto glassTransform=_state->_glass->interactionTransform();
        const auto inverseGlass=glassTransform.inverted();
        const auto surfacePoint=inverseGlass?point.applying(*inverseGlass):point;
        SkMatrix inverseLens;
        _state->_grabbedLens=false; _state->_grabOffsetX=0;
        if(placement.invert(&inverseLens)) {
            const auto local=mapLensPoint(inverseLens,surfacePoint);
            _state->_grabbedLens=Internal::layerCornerPath(lensSize,lensLayer->cornerRadius(),lensLayer->cornerCurve()).contains(local.x(),local.y());
        }
        if(_state->_grabbedLens) {
            // Freeze the presentation placement, retaining the independent
            // elastic affine. Folding its drift into model position would
            // apply that drift twice when catching a travelling lens.
            const auto center=lensLayer->position();
            const auto visibleCenter=center.applying(glassTransform);
            _state->_grabOffsetX=point.x-visibleCenter.x;
            // Catch the visible lens during an in-flight approach as well.
            _state->_lens->layer()->removeAnimation("position");
            setSelectionPosition(positionForContact(center.x));
        }
        beginLift();
        if(!_state->_grabbedLens) animateSelectionPosition(positionForFinger(point));
        _state->_lens->beginInteraction(point);
    } else if(_state->_trackingTouch.lock()==touch) {
        if(phase==UITouchPhase::moved) {
            const auto seconds=(touch->timestamp()-_state->_lastContactTime)/1000.0;
            const auto velocity=seconds>0 ? (point-_state->_lastContact)/std::max(float(seconds),1.0f/240) : NXPoint();
            _state->_lastContact=point; _state->_lastContactTime=touch->timestamp();
            _state->_lens->updateInteraction(point,velocity);
            if(!_state->_dragging) {
                if(_state->_grabbedLens) setSelectionPosition(positionForFinger(point));
                else animateSelectionPosition(positionForFinger(point));
            }
        } else {
            _state->_trackingTouch.reset();
            // Release the driving force while preserving the lens spring's
            // current velocity; a held finger must not inject a stale fling.
            _state->_lens->endInteraction({},phase==UITouchPhase::cancelled);
            // A stationary contact never begins the pan. Keep the committed
            // slot (including the descendant's ordinary tap action) and let
            // the raised approach finish before lowering on release.
            if(!_state->_dragging) { commitSelection(selectedItem(),true); releaseLift(phase==UITouchPhase::cancelled); }
        }
    }
}
float UITabBarView::positionForFinger(NXPoint point) const {
    point.x-=_state->_grabOffsetX;
    if(const auto inverse=_state->_glass->interactionTransform().inverted()) point=point.applying(*inverse);
    return positionForContact(point.x);
}
float UITabBarView::positionForContact(NXFloat x) const {
    if(!_state->_count||_state->_selectionCenters.size()!=_state->_count||!std::isfinite(x)) return _state->_position;
    if(x<=_state->_selectionCenters.front()) return 0;
    for(size_t i=1;i<_state->_count;++i) if(x<=_state->_selectionCenters[i]) {
        const auto step=_state->_selectionCenters[i]-_state->_selectionCenters[i-1];
        return float(i-1)+(step>0?(x-_state->_selectionCenters[i-1])/step:0);
    }
    return float(_state->_count-1);
}
void UITabBarView::animateSelectionPosition(float position) {
    // A tap action arrives after touch-down has already targeted the same
    // slot. Keep that in-flight approach instead of starting it a second time.
    if(std::abs(_state->_position-position)<1e-5f) return;
    const auto from=_state->_lens->layer()->presentationOrSelf()->position();
    UIView::animate(.6,0,static_cast<UIViewAnimationOptions>(UIViewAnimationOptions::allowUserInteraction|UIViewAnimationOptions::beginFromCurrentState),
        [this,position] { setSelectionPosition(position); });
    auto movement=new_shared<CABasicAnimation>("position");
    movement->duration=.6; movement->fromValue=from; movement->toValue=_state->_lens->layer()->position();
    // Empirical fit to native UITabBar presentation positions, not Apple's
    // private spring implementation. Small overshoot is intentional.
    movement->timingFunction=new_shared<CAMediaTimingFunction>([](double fraction) {
        if(fraction<=0) return 0.0;
        if(fraction>=1) return 1.0;
        constexpr double damping=.85,frequency=15.8;
        const auto damped=std::sqrt(1-damping*damping);
        const auto time=fraction*.6;
        return 1-std::exp(-damping*frequency*time)*(std::cos(frequency*damped*time)+damping/damped*std::sin(frequency*damped*time));
    });
    _state->_lens->layer()->add(movement,"position");
}
void UITabBarView::beginLift() {
    _state->_minimumLiftRemaining=.2;
    _state->_liftParent=superview(); _state->_liftWindow=window();
    setLifted(true,true);
    if(_state->_liftLink) return;
    _state->_lastLiftFrame=std::chrono::steady_clock::now();
    const auto weak=weak_from_base<UITabBarView>();
    _state->_liftLink=std::make_shared<CADisplayLink>([weak] {
        if(auto self=weak.lock()) {
            const auto now=std::chrono::steady_clock::now();
            const auto elapsed=std::chrono::duration<double>(now-self->_state->_lastLiftFrame).count();
            self->_state->_lastLiftFrame=now; self->advanceLift(elapsed);
        }
    });
    CALayer::requestFramerate(120);
}
void UITabBarView::releaseLift(bool cancelled) {
    if(cancelled) setLifted(false,true);
    else advanceLift(0);
}
void UITabBarView::advanceLift(double seconds) {
    if(!_state->_liftLink || !std::isfinite(seconds) || seconds<0) return;
    bool invalid=_state->_liftParent.lock()!=superview().lock() || _state->_liftWindow.lock()!=window();
    for(auto view=shared_from_this();view;view=view->superview().lock())
        invalid=invalid||view->isHidden()||view->alpha()<.01f||!view->isUserInteractionEnabled();
    if(invalid) { setLifted(false); return; }
    _state->_minimumLiftRemaining=std::max(0.0,_state->_minimumLiftRemaining-seconds);
    const auto layer=_state->_lens->layer();
    const auto remaining=std::abs(layer->presentationOrSelf()->position().x-layer->position().x);
    // Native short taps hold the raised 111x70 lens through transit and
    // lower within about eight points of arrival, after at least 200ms.
    if(_state->_trackingTouch.expired()&&!_state->_dragging&&_state->_minimumLiftRemaining==0&&remaining<=8)
        setLifted(false,true);
    else CALayer::requestFramerate(120);
}
void UITabBarView::setSelectionPosition(float position) {
    if (!_state->_count || !std::isfinite(position)) return;
    _state->_position=std::clamp(position,0.0f,float(_state->_count-1)); arrange();
}
void UITabBarView::setLifted(bool lifted, bool animated) {
    if(!lifted && _state->_liftLink) { _state->_liftLink->invalidate(); _state->_liftLink.reset(); _state->_minimumLiftRemaining=0; }
    _state->_lifted=lifted; _state->_lens->setLifted(lifted,animated);
    if (animated) UIView::animate(.28,0,static_cast<UIViewAnimationOptions>(UIViewAnimationOptions::allowUserInteraction|UIViewAnimationOptions::beginFromCurrentState),[this] { arrange(); });
    else arrange();
}
void UITabBarView::setComparisonContent(std::shared_ptr<UIImage> normal, std::shared_ptr<UIImage> selected) {
    if (!normal || !selected) throw std::invalid_argument("Both lens comparison content images are required");
    if (!_state->_normalImage) { _state->_normalImage=new_shared<UIImageView>(); _state->_normalImage->setUserInteractionEnabled(false); _state->_normal->addSubview(_state->_normalImage); }
    if (!_state->_selectedImage) { _state->_selectedImage=new_shared<UIImageView>(); _state->_selectedContent->addSubview(_state->_selectedImage); }
    _state->_normalImage->setImage(normal); _state->_selectedImage->setImage(selected);
    for (size_t i=0;i<_state->_count;i++) { _state->_normalSymbols[i]->setHidden(true); _state->_normalLabels[i]->setHidden(true); _state->_selectedSymbols[i]->setHidden(true); _state->_selectedLabels[i]->setHidden(true); }
    arrange();
}
void UITabBarView::setComparisonGeometry(NXRect frame) { _state->_comparisonFrame=frame; arrange(); }

UITabBarView::~UITabBarView() {
    if (_state->_liftLink) _state->_liftLink->invalidate();
}
std::vector<std::shared_ptr<UITabBarItem>> UITabBarView::items() const { return _state->items; }
std::shared_ptr<UITabBarItem> UITabBarView::selectedItem() const {
    return _state->hasSelection && _state->_selected < _state->items.size()
        ? _state->items[_state->_selected] : nullptr;
}
std::shared_ptr<UITabBarDelegate> UITabBarView::delegate() const { return _state->delegate.lock(); }
void UITabBarView::setDelegate(const std::shared_ptr<UITabBarDelegate>& value) {
    if (!_state->owner.expired()) throw std::logic_error("A managed tab bar's delegate is its controller");
    _state->delegate = value;
}
void UITabBarView::setItems(std::vector<std::shared_ptr<UITabBarItem>> values, bool animated) {
    if (!_state->owner.expired()) throw std::logic_error("Change a managed tab bar through its controller");
    installItems(std::move(values), animated);
}
void UITabBarView::resetContact() {
    _state->resetting = true;
    if (_state->pan) _state->pan->setEnabled(false);
    _state->_trackingTouch.reset();
    _state->_dragging = false;
    _state->_grabbedLens = false;
    _state->_lens->endInteraction({}, true);
    setLifted(false);
    if (_state->pan) _state->pan->setEnabled(true);
    _state->resetting = false;
}
void UITabBarView::installItems(std::vector<std::shared_ptr<UITabBarItem>> values, bool animated) {
    std::unordered_set<const UITabBarItem*> unique;
    for (const auto& item : values) {
        if (!item || !unique.insert(item.get()).second)
            throw std::invalid_argument("Tab bar items must be non-null and distinct");
    }
    const auto selected = selectedItem();
    resetContact();
    ++_state->generation;
    const auto generation = _state->generation;
    const auto oldNormal = _state->_normal;
    const auto oldSelected = _state->_selectedHost;
    if (oldNormal) oldNormal->setUserInteractionEnabled(false);
    if (oldSelected) oldSelected->setUserInteractionEnabled(false);
    _state->items = std::move(values);
    _state->_count = _state->items.size();
    _state->hasSelection = false;
    _state->_selected = 0;
    if (const auto found = std::find(_state->items.begin(),_state->items.end(),selected);
        selected && found != _state->items.end()) {
        _state->hasSelection = true;
        _state->_selected = size_t(found - _state->items.begin());
    }
    _state->_position = float(_state->_selected);
    _state->_normal = new_shared<UIView>();
    _state->_glass->insertSubviewBelow(_state->_normal, _state->_lens);
    auto mask = new_shared<NormalRowMask>(); mask->lens = _state->_lens;
    _state->_normal->setMask(new_shared<UIView>(NXRect{}, mask));
    _state->_selectedHost = new_shared<UIView>();
    _state->_selectedHost->setUserInteractionEnabled(false);
    _state->_glass->insertSubviewBelow(_state->_selectedHost, _state->_lens);
    _state->_selectedContent = new_shared<UIView>();
    _state->_selectedContent->setUserInteractionEnabled(false);
    _state->_selectedHost->addSubview(_state->_selectedContent);
    _state->_lens->setLiftedContentView(_state->_selectedContent);
    _state->_normalImage.reset(); _state->_selectedImage.reset();
    _state->_controls.clear(); _state->_normalSymbols.clear(); _state->_selectedSymbols.clear();
    _state->_normalLabels.clear(); _state->_selectedLabels.clear(); _state->_selectedItems.clear();
    for (const auto& badge : _state->badges) badge->removeFromSuperview();
    _state->badges.clear(); _state->badgeLabels.clear();
    const auto weak = weak_from_base<UITabBarView>();
    for (size_t i = 0; i < _state->_count; ++i) {
        auto control = new_shared<TabControl>(); _state->_normal->addSubview(control);
        control->primaryAction = UIAction(_state->items[i]->title(), [weak,i,generation] {
            if (const auto view = weak.lock(); view && view->_state->generation == generation)
                view->activateIndex(i);
        });
        control->onHighlight = [weak](bool active) {
            if (const auto view = weak.lock(); view && !view->_state->resetting
                && !view->_state->_dragging && view->_state->_trackingTouch.expired()) {
                active ? view->beginLift() : view->releaseLift();
            }
        };
        _state->_controls.push_back(control);
        auto selectedView = new_shared<UIView>(); selectedView->setUserInteractionEnabled(false);
        _state->_selectedContent->addSubview(selectedView); _state->_selectedItems.push_back(selectedView);
        for (bool selected : {false,true}) {
            auto image = new_shared<UIImageView>(); image->setContentMode(UIViewContentMode::scaleAspectFit);
            auto label = new_shared<UILabel>(); label->setUserInteractionEnabled(false);
            label->setFontSize(10); label->setFontWeight(selected ? 500 : 600);
            label->setTextAlignment(NSTextAlignment::center);
            (selected ? selectedView : std::static_pointer_cast<UIView>(control))->addSubview(image);
            (selected ? selectedView : std::static_pointer_cast<UIView>(control))->addSubview(label);
            (selected ? _state->_selectedSymbols : _state->_normalSymbols).push_back(image);
            (selected ? _state->_selectedLabels : _state->_normalLabels).push_back(label);
        }
        auto badge = new_shared<UIView>(); badge->setUserInteractionEnabled(false);
        badge->layer()->setCornerRadius(8); _state->_glass->addSubview(badge);
        auto label = new_shared<UILabel>(); label->setFontSize(11); label->setFontWeight(600);
        label->setTextAlignment(NSTextAlignment::center); label->setTextColor(UIColor::white);
        label->setUserInteractionEnabled(false); badge->addSubview(label);
        _state->badges.push_back(badge); _state->badgeLabels.push_back(label);
        Internal::BarItemAccess::observe(*_state->items[i], shared_from_this(), [weak,generation] {
            if (const auto view = weak.lock(); view && view->_state->generation == generation) {
                view->resetContact(); view->refreshItems();
            }
        });
    }
    refreshItems();
    const bool fade = animated && window() && oldNormal && !_state->items.empty();
    if (fade) {
        _state->_normal->setAlpha(0); _state->_selectedHost->setAlpha(0);
        UIView::animate(.2, 0, UIViewAnimationOptions::allowUserInteraction,
            [normal = _state->_normal, selected = _state->_selectedHost,oldNormal,oldSelected] {
                normal->setAlpha(1); selected->setAlpha(1);
                oldNormal->setAlpha(0); if (oldSelected) oldSelected->setAlpha(0);
            }, [oldNormal,oldSelected](bool) {
                oldNormal->removeFromSuperview(); if (oldSelected) oldSelected->removeFromSuperview();
            });
    } else {
        if (oldNormal) oldNormal->removeFromSuperview();
        if (oldSelected) oldSelected->removeFromSuperview();
    }
}
void UITabBarView::refreshItems() {
    const auto color = tintColor();
    const auto unselected = _state->unselectedTint.value_or(UIColor::label);
    const UIColor semantic[] = {UIColor::systemBlue,UIColor::systemRed,UIColor::systemGreen,
        UIColor::systemOrange,UIColor::systemPurple,UIColor::systemMint,UIColor::systemYellow,
        UIColor::systemIndigo,UIColor::systemGray,UIColor::systemTeal,UIColor::systemCyan,
        UIColor::systemPink,UIColor::systemBrown,UIColor::tint};
    _state->_lens->setContentVibrancyEnabled(std::any_of(std::begin(semantic),std::end(semantic),
        [&](const auto& value) { return value == color; }));
    auto sizing=new_shared<UILabel>();sizing->setFontSize(17);sizing->setFontWeight(500);
    _state->_itemContentWidths.assign(_state->_count,81);
    _state->_minimumItemWidth=0;
    for (size_t i = 0; i < _state->_count; ++i) {
        const auto& item = _state->items[i];
        _state->_controls[i]->setEnabled(item->isEnabled());
        _state->_controls[i]->setSelected(_state->hasSelection && _state->_selected == i);
        _state->_controls[i]->setAlpha(item->isEnabled() ? 1 : .4f);
        _state->_selectedItems[i]->setAlpha(item->isEnabled() ? 1 : .4f);
        _state->_normalLabels[i]->setText(item->title()); _state->_selectedLabels[i]->setText(item->title());
        _state->_normalLabels[i]->setTextColor(unselected); _state->_selectedLabels[i]->setTextColor(color);
        _state->_normalSymbols[i]->setTintColor(unselected); _state->_selectedSymbols[i]->setTintColor(color);
        if (const auto image = std::dynamic_pointer_cast<UIImageView>(_state->_normalSymbols[i]))
            image->setImage(item->image());
        if (const auto image = std::dynamic_pointer_cast<UIImageView>(_state->_selectedSymbols[i]))
            image->setImage(item->selectedImage() ? item->selectedImage() : item->image());
        _state->badges[i]->setHidden(!item->badgeValue());
        _state->badges[i]->setBackgroundColor(item->badgeColor().value_or(UIColor::systemRed));
        _state->badgeLabels[i]->setText(item->badgeValue().value_or(""));
        if(SkiaCtx::main()) {
            // Cache font-dependent sizing outside the 120Hz lens layout path.
            sizing->setText(item->title());
            _state->_itemContentWidths[i]=std::max(81.0f,sizing->sizeThatFits({10000,100}).width+15);
            _state->_minimumItemWidth=std::max(_state->_minimumItemWidth,
                _state->_selectedLabels[i]->sizeThatFits({10000,100}).width+(_state->_count>=5?34:40));
        }
    }
    _state->_lens->setHidden(!_state->hasSelection || !_state->_count);
    arrange(); setNeedsLayout();
}
void UITabBarView::setSelectedItem(const std::shared_ptr<UITabBarItem>& item) {
    if (item && std::find(_state->items.begin(),_state->items.end(),item) == _state->items.end())
        throw std::invalid_argument("The selected item must belong to the tab bar");
    if (!_state->owner.expired() && _state->managedSelection) {
        _state->managedSelection(item, false); return;
    }
    resetContact(); commitSelection(item, false);
}
void UITabBarView::commitSelection(const std::shared_ptr<UITabBarItem>& item, bool animated) {
    const auto found = std::find(_state->items.begin(),_state->items.end(),item);
    if (item && found == _state->items.end()) return;
    const bool changed = selectedItem() != item;
    _state->hasSelection = bool(item);
    _state->_selected = item ? size_t(found-_state->items.begin()) : 0;
    _state->_lens->setHidden(!item);
    if (item && animated) {
        if (changed && !_state->_liftLink) beginLift();
        animateSelectionPosition(float(_state->_selected));
    } else if (item) {
        if (_state->_liftLink && _state->_trackingTouch.expired() && !_state->_dragging) setLifted(false);
        _state->_lens->layer()->removeAnimation("position");
        UIView::performWithoutAnimation([&] { setSelectionPosition(float(_state->_selected)); });
    }
    for (size_t i=0;i<_state->_count;++i)
        _state->_controls[i]->setSelected(item && i==_state->_selected);
    if (changed && item && _state->legacySelectionChanged) _state->legacySelectionChanged(_state->_selected);
}
bool UITabBarView::activateIndex(size_t index) {
    if (_state->resetting || index >= _state->items.size() || !_state->items[index]->isEnabled()
        || !isActionDispatchAllowed()) { commitSelection(selectedItem(),true); return false; }
    const auto item = _state->items[index];
    if (!_state->owner.expired() && _state->managedSelection) {
        const bool accepted = _state->managedSelection(item, true);
        if (!accepted) commitSelection(selectedItem(),true);
        return accepted;
    }
    commitSelection(item,true);
    if (const auto target = delegate()) target->tabBarDidSelect(shared_from_base<UITabBarView>(),item);
    return true;
}
std::optional<UIColor> UITabBarView::unselectedItemTintColor() const { return _state->unselectedTint; }
void UITabBarView::setUnselectedItemTintColor(std::optional<UIColor> value) {
    _state->unselectedTint = std::move(value); refreshItems();
}
void UITabBarView::tintColorDidChange() { UIView::tintColorDidChange(); if (_state) refreshItems(); }
void UITabBarView::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previous) {
    UIView::traitCollectionDidChange(std::move(previous)); if (_state) refreshItems();
}
void UITabBarView::layoutSubviews() { UIView::layoutSubviews(); arrange(); }
NXSize UITabBarView::sizeThatFits(NXSize size) {
    // Native phone bars with fewer than four items hug their 86pt item pitch.
    const auto available=std::max(0.0f,size.width);
    return {_state->_count<4?std::min(available,16.0f+86*_state->_count):available,62};
}
bool UITabBarView::point(NXPoint p, UIEvent*) {
    p = p - bounds().origin;
    const auto size = bounds().size;
    if (p.x < 0 || p.y < 0 || p.x > size.width || p.y > size.height || _state->items.empty()) return false;
    const auto radius = std::min(size.width,size.height)*.5f;
    const auto x=std::max(0.0f,std::abs(p.x-size.width*.5f)-(size.width*.5f-radius));
    const auto y=std::max(0.0f,std::abs(p.y-size.height*.5f)-(size.height*.5f-radius));
    return x*x+y*y<=radius*radius;
}
std::vector<std::shared_ptr<UIFocusEnvironment>> UITabBarView::preferredFocusEnvironments() {
    if (_state->hasSelection && _state->_controls[_state->_selected]->isEnabled())
        return {_state->_controls[_state->_selected]};
    for (const auto& control : _state->_controls) if (control->isEnabled()) return {control};
    return {};
}
void UITabBarView::arrange() {
    if (_state->customLayout) { _state->customLayout(); return; }
    auto& s = *_state;
    const auto w=std::max(0.0f,bounds().width()),h=std::max(0.0f,bounds().height());
    s._glass->setFrame({bounds().origin.x,bounds().origin.y,w,h}); s._glass->layer()->setCornerRadius(h*.5f);
    s._normal->setFrame({0,0,w,h});
    if (s._normal->mask()) s._normal->mask()->setFrame({0,0,w,h});
    s._selectedHost->setFrame({0,0,w,h}); s._selectedContent->setFrame({0,0,w,h});
    if (!s._count) { s._lens->setHidden(true); return; }
    const auto slot=w/s._count;
    const auto inset=std::min(4.0f,w*.5f);
    const auto available=std::max(0.0f,w-2*inset);
    auto& widths=s._itemWidths;auto& left=s._itemLefts;
    widths.resize(s._count);left.resize(s._count);
    if(s._count>=4&&slot<70&&w>=28) {
        // Narrow UIKit layouts compress content proportionally, retaining
        // 20pt cell padding/overlap. Longer titles receive wider cells.
        NXFloat total=0;for(const auto width:s._itemContentWidths)total+=width;
        NXFloat x=inset;
        for(size_t i=0;i<s._count;++i){widths[i]=std::min(available,20+(w-28)*s._itemContentWidths[i]/std::max(1.0f,total));left[i]=x;x+=widths[i]-20;}
    } else {
        const auto preferredWidth=s._count<4?94.0f:std::max(slot+5,s._minimumItemWidth);
        const auto width=std::max(0.0f,std::min(preferredWidth,available));
        const auto step=s._count>1?(available-width)/(s._count-1):0;
        for(size_t i=0;i<s._count;++i){widths[i]=width;left[i]=s._count>1?inset+i*step:(w-width)*.5f;}
    }
    s._selectionCenters.resize(s._count);
    for (size_t i=0;i<s._count;++i)
        s._selectionCenters[i]=left[i]+widths[i]*.5f;
    s._selectionStep=s._count>1?s._selectionCenters[1]-s._selectionCenters[0]:0;
    const auto a=std::min(s._count-1,size_t(std::max(0.0f,s._position)));
    const auto b=std::min(s._count-1,a+1); const auto t=s._position-float(a);
    const auto center=s._selectionCenters[a]*(1-t)+s._selectionCenters[b]*t;
    const auto growth=s._lifted?16.0f:0.0f;
    const auto lensWidth=std::max(0.0f,widths[a]*(1-t)+widths[b]*t+growth);
    const auto lensHeight=std::max(0.0f,std::min(56.0f,h-8)+growth);
    s._lens->setFrame({center-lensWidth*.5f,(h-lensHeight)*.5f,lensWidth,lensHeight});
    s._selectionFill->setFrame(s._lens->restingBackgroundView()->bounds());
    for (size_t i=0;i<s._count;++i) {
        const auto itemWidth=widths[i];
        const auto centerX=s._selectionCenters[i];
        auto itemLayer=s._selectedItems[i]->layer(); itemLayer->setAnchorPoint({0,0});
        itemLayer->setBounds({0,0,w,h}); itemLayer->setPosition({0,0});
        const auto zoom=s._lifted?1.16f:1.f;
        s._selectedItems[i]->setTransform({zoom,0,0,zoom,(1-zoom)*centerX,(1-zoom)*h*.5f});
        const auto itemLeft=centerX-itemWidth*.5f;
        s._controls[i]->setFrame({itemLeft,0,itemWidth,h}); s._controls[i]->layer()->setCornerRadius(h*.5f);
        for (bool selected : {false,true}) {
            const auto& icon=selected?s._selectedSymbols[i]:s._normalSymbols[i];
            const auto& label=selected?s._selectedLabels[i]:s._normalLabels[i];
            const auto offset=selected?0:itemLeft;
            const auto image=selected&&s.items[i]->selectedImage()?s.items[i]->selectedImage():s.items[i]->image();
            const auto original=image?image->size():NXSize{};
            const auto iconHeight=std::min(28.0f,std::max(0.0f,h-32));
            const auto ratio=original.height>0?std::min(1.0f,iconHeight/original.height):0;
            const auto width=std::min(itemWidth,std::max(0.0f,original.width*ratio));
            const auto imageHeight=std::min(iconHeight,original.height*ratio);
            icon->setFrame({centerX-offset-width*.5f,(h-14-imageHeight)*.5f,width,imageHeight});
            label->setFrame({centerX-offset-itemWidth*.5f,h-23,itemWidth,12});
        }
        const auto textSize=SkiaCtx::main()?s.badgeLabels[i]->sizeThatFits({1000,16}):NXSize(8,16);
        const auto badgeWidth=std::min(slot,std::max(16.0f,textSize.width+8));
        s.badges[i]->setFrame({centerX+8,4,badgeWidth,16});
        s.badgeLabels[i]->setFrame({0,0,badgeWidth,16});
    }
}
