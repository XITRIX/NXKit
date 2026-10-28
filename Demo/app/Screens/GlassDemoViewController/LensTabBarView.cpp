#include "LensTabBarView.hpp"
#include "GlassScreenTypography.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <UITouch.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "../../../../UIKitCore/lib/LayerCornerGeometry.hpp"
using namespace NXKit;
namespace {
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
        if (auto view=lens.lock()) {
            const auto layer=view->layer()->presentationOrSelf();
            const auto size=layer->bounds().size;
            const auto position=layer->position(), anchor=layer->anchorPoint();
            auto matrix=CATransform3DMakeTranslation(position.x,position.y,0)
                .concat(layer->transform()).toSkM44().asM33();
            matrix.preTranslate(-size.width*anchor.x,-size.height*anchor.y);
            matrix.preConcat(NXTransform3DMakeAffineTransform(view->interactionTransform()).toSkM44().asM33());
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
    std::function<void(bool)> onHighlight;
    void willChangeHighlight(bool highlighted) override { if (onHighlight) onHighlight(highlighted); }
    void willGainFocus() override { layer()->setBorderWidth(1); layer()->setBorderColor(UIColor::tint); }
    void willLoseFocus() override { layer()->setBorderWidth(0); }
};
// Portable filled tab symbols for the full-screen phone example. The older
// three-item optical fixtures keep their existing independent source artwork.
class PhoneTabSymbolLayer final : public CALayer {
public:
    int shape; UIColor ink;
    PhoneTabSymbolLayer(int shape,UIColor ink):shape(shape),ink(ink) {}
    explicit PhoneTabSymbolLayer(PhoneTabSymbolLayer* old):CALayer(old),shape(old->shape),ink(old->ink) {}
    std::shared_ptr<CALayer> copy() override { return new_shared<PhoneTabSymbolLayer>(this); }
    void draw(SkCanvas* canvas) override {
        SkPaint p; p.setColor(ink.raw()); p.setAntiAlias(true);
        if(shape==0) {
            SkPathBuilder roof; roof.moveTo(1,11);roof.lineTo(12,1);roof.lineTo(23,11);
            p.setStyle(SkPaint::kStroke_Style);p.setStrokeWidth(2.2f);p.setStrokeCap(SkPaint::kRound_Cap);p.setStrokeJoin(SkPaint::kRound_Join);canvas->drawPath(roof.detach(),p);
            p.setStyle(SkPaint::kFill_Style);SkPathBuilder house;house.setFillType(SkPathFillType::kEvenOdd);house.moveTo(4,11);house.lineTo(12,4);house.lineTo(20,11);house.lineTo(20,23);house.lineTo(4,23);house.close();house.addRect(SkRect::MakeXYWH(10,15,4,8));canvas->drawPath(house.detach(),p);
        } else if(shape==1) {
            SkPathBuilder star;for(int i=0;i<10;++i){float angle=-1.57079633f+i*.62831853f,r=i%2?4.8f:11.5f;float x=12+std::cos(angle)*r,y=12+std::sin(angle)*r;if(i)star.lineTo(x,y);else star.moveTo(x,y);}star.close();canvas->drawPath(star.detach(),p);
        } else if(shape==3) {
            canvas->drawRoundRect(SkRect::MakeXYWH(1,2,5,21),1,1,p);canvas->drawRoundRect(SkRect::MakeXYWH(8,4,5,19),1,1,p);
            canvas->save();canvas->rotate(-10,18,13);canvas->drawRoundRect(SkRect::MakeXYWH(16,2,5,21),1,1,p);canvas->restore();
        } else {
            for(int i=0;i<3;++i){canvas->drawRoundRect(SkRect::MakeXYWH(1,4+i*7,22,2),1,1,p);canvas->drawRoundRect(SkRect::MakeXYWH(i==1?6:15,1+i*7,4,8),1,1,p);}
        }
    }
};
class PhoneTabSymbol final : public UIView {
public:
    PhoneTabSymbol(int shape,UIColor color):UIView({},new_shared<PhoneTabSymbolLayer>(shape,color)) { setUserInteractionEnabled(false); }
    void setColor(UIColor color) {
        auto ink=std::static_pointer_cast<PhoneTabSymbolLayer>(layer());ink->ink=color;
        if(auto active=ink->presentation())std::static_pointer_cast<PhoneTabSymbolLayer>(active)->ink=color;
        ink->setNeedsDisplay();
    }
};
std::shared_ptr<UIView> symbol(int index, const UIColor& color) {
    auto view = new_shared<UIView>(); view->setUserInteractionEnabled(false);
    auto line = [&](NXRect rect, float angle = 0) {
        auto stroke=new_shared<UIView>(rect); stroke->setBackgroundColor(color);
        stroke->layer()->setCornerRadius(1); stroke->setTransform(NXAffineTransform::rotationBy(angle)); view->addSubview(stroke);
    };
    if (index==0) { line({2,7,12,2},-40); line({10,7,12,2},40); line({5,11,2,11}); line({17,11,2,11}); line({5,20,14,2}); }
    if (index==1) { line({11,1,2,22}); line({1,11,22,2}); line({4,4,16,2},45); line({4,18,16,2},-45); }
    if (index==2) { for (int i=0;i<3;i++) { line({2,4+i*7.0f,20,2}); auto knob=new_shared<UIView>(NXRect(i==1?6:14,1+i*7.0f,4,8)); knob->setBackgroundColor(color); knob->layer()->setCornerRadius(2); view->addSubview(knob); } }
    if (index==3) { line({3,2,2,20}); line({9,2,2,20}); line({16,3,2,20},-14); }
    return view;
}
}
LensTabBarView::LensTabBarView(bool compact, size_t itemCount): _count(itemCount), _compact(compact) {
    if (itemCount != 3 && itemCount != 4) throw std::invalid_argument("Demo tab bar supports three or four items");
    UIGlassEffect material; material.setInteractive(true);
    _glass=new_shared<BackdropEffectView>(material); addSubview(_glass);
    _normal=new_shared<UIView>(); _glass->addSubview(_normal);
    // Both complete icon rows stay fixed in platter coordinates. The selected
    // row sits below the lens, which owns its ordinary-output suppression and
    // samples its unmasked child so details outside the silhouette can refract
    // into it while dragging between items.
    _selectedHost=new_shared<UIView>(); _selectedHost->setUserInteractionEnabled(false);
    // The optical pass composites the source once, without using alpha/hidden
    // to fake a portal (those properties must retain their normal meaning).
    _glass->addSubview(_selectedHost);
    _selectedContent=new_shared<UIView>(); _selectedContent->setUserInteractionEnabled(false); _selectedHost->addSubview(_selectedContent);
    _lens=new_shared<LensView>(LensView::Style::selection); _lens->setUserInteractionEnabled(false);
    _lens->setInteractive(true);
    _lens->setInteractionSource(LensView::InteractionSource::viewMotion);
    _lens->setRestingBackgroundColor(UIColor::label.withAlphaComponent(.10f)); _glass->addSubview(_lens);
    auto rowMask=new_shared<NormalRowMask>(); rowMask->lens=_lens;
    _normal->setMask(new_shared<UIView>(NXRect(),rowMask));
    _lens->setLiftedContentView(_selectedContent);
    const char* titles3[]={"Home","Favorites","Settings"}; const char* titles4[]={"Home","Favorites","Library","Settings"}; const auto titles=_count==4?titles4:titles3; const auto weak=weak_from_base<LensTabBarView>();
    for (size_t i=0;i<_count;i++) {
        auto control=new_shared<TabControl>(); _normal->addSubview(control); _controls.push_back(control);
        control->primaryAction=UIAction(titles[i],[weak,i] { if (auto self=weak.lock()) self->setSelectedIndex(i,true); });
        control->onHighlight=[weak](bool highlighted) { if (auto self=weak.lock(); self && !self->_dragging) self->setLifted(highlighted,true); };
        const auto icon=_count==4?(i==2?3:(i==3?2:int(i))):int(i);
        std::shared_ptr<UIView> normal=_count==4?std::static_pointer_cast<UIView>(new_shared<PhoneTabSymbol>(icon,UIColor::label)):symbol(icon,UIColor::label);
        std::shared_ptr<UIView> selected=_count==4?std::static_pointer_cast<UIView>(new_shared<PhoneTabSymbol>(icon,UIColor(0,122,255))):symbol(icon,UIColor(0,122,255)); control->addSubview(normal); _selectedContent->addSubview(selected);
        _normalSymbols.push_back(normal); _selectedSymbols.push_back(selected);
        for (bool blue : {false,true}) {
            auto label=new_shared<UILabel>(); label->setText(titles[i]); if(_count==4) GlassScreenTypography::apply(label,GlassScreenTypography::caption2); else label->setFontSize(compact?17:11);
            label->setTextAlignment(NSTextAlignment::center); label->setTextColor(blue?UIColor(0,122,255):UIColor::label); label->setUserInteractionEnabled(false);
            if (blue) { _selectedContent->addSubview(label); _selectedLabels.push_back(label); }
            else { control->addSubview(label); _normalLabels.push_back(label); }
        }
    }
    auto pan=new_shared<UIPanGestureRecognizer>();
    pan->onStateChanged=[weak](const auto& recognizer) {
        const auto self=weak.lock(); if (!self) return;
        const auto gesture=std::static_pointer_cast<UIPanGestureRecognizer>(recognizer);
        const auto state=gesture->state();
        if (state==UIGestureRecognizerState::began) { self->_dragStart=self->_selected; self->_dragging=true; self->setLifted(true,true); }
        if (state==UIGestureRecognizerState::began || state==UIGestureRecognizerState::changed) {
            const auto x=float(self->_dragStart)+gesture->translationInView(self).x/std::max(1.0f,self->_selectionStep);
            self->setSelectionPosition(x);
        }
        if (state==UIGestureRecognizerState::ended) { self->_dragging=false; self->setSelectedIndex(size_t(std::round(self->_position)),true); self->setLifted(false,true); }
        if (self->_dragging && (state==UIGestureRecognizerState::cancelled || state==UIGestureRecognizerState::failed)) { self->_dragging=false; self->setSelectedIndex(self->_dragStart,true); self->setLifted(false,true); }
    };
    addGestureRecognizer(pan);
    auto contact=new_shared<LensContactObserver>();
    contact->observe=[weak](auto phase,const auto& touch) { if(auto self=weak.lock()) self->observeTouch(phase,touch); };
    addGestureRecognizer(contact);
}
void LensTabBarView::observeTouch(UITouchPhase phase,const std::shared_ptr<UITouch>& touch) {
    if(!touch) return;
    const auto point=touch->locationIn(_glass);
    if(!std::isfinite(point.x)||!std::isfinite(point.y)) return;
    if(phase==UITouchPhase::began) {
        if(!_trackingTouch.expired()) return;
        // The platter is a capsule; its transparent corners are not contacts.
        const auto size=_glass->bounds().size;
        const auto radius=std::min(size.width,size.height)*.5f;
        const auto x=std::max(0.0f,std::abs(point.x-size.width*.5f)-(size.width*.5f-radius));
        const auto y=std::max(0.0f,std::abs(point.y-size.height*.5f)-(size.height*.5f-radius));
        if(size.width<=0||size.height<=0||x*x+y*y>radius*radius) return;
        _trackingTouch=touch; _lastContact=point; _lastContactTime=touch->timestamp();
        _lens->beginInteraction(point);
    } else if(_trackingTouch.lock()==touch) {
        if(phase==UITouchPhase::moved) {
            const auto seconds=(touch->timestamp()-_lastContactTime)/1000.0;
            const auto velocity=seconds>0 ? (point-_lastContact)/std::max(float(seconds),1.0f/240) : NXPoint();
            _lastContact=point; _lastContactTime=touch->timestamp();
            _lens->updateInteraction(point,velocity);
        } else {
            _trackingTouch.reset();
            // Release the driving force while preserving the lens spring's
            // current velocity; a held finger must not inject a stale fling.
            _lens->endInteraction({},phase==UITouchPhase::cancelled);
        }
    }
}
void LensTabBarView::setSelectedIndex(size_t index, bool animated) {
    if (index>=_count) throw std::out_of_range("Lens tab index");
    const auto changed=_selected!=index; _selected=index;
    auto apply=[this,index] { setSelectionPosition(float(index)); };
    if (animated) UIView::animate(.28,0,static_cast<UIViewAnimationOptions>(UIViewAnimationOptions::allowUserInteraction|UIViewAnimationOptions::beginFromCurrentState),apply);
    else apply();
    for (size_t i=0;i<_count;i++) _controls[i]->setSelected(i==index);
    if (changed && onSelectionChanged) onSelectionChanged(index);
}
void LensTabBarView::setSelectionPosition(float position) {
    if (!std::isfinite(position)) return;
    _position=std::clamp(position,0.0f,float(_count-1)); arrange();
}
void LensTabBarView::setLifted(bool lifted, bool animated) {
    _lifted=lifted; _lens->setLifted(lifted,animated);
    if (animated) UIView::animate(.28,0,static_cast<UIViewAnimationOptions>(UIViewAnimationOptions::allowUserInteraction|UIViewAnimationOptions::beginFromCurrentState),[this] { arrange(); });
    else arrange();
}
void LensTabBarView::setComparisonContent(std::shared_ptr<UIImage> normal, std::shared_ptr<UIImage> selected) {
    if (!normal || !selected) throw std::invalid_argument("Both lens comparison content images are required");
    if (!_normalImage) { _normalImage=new_shared<UIImageView>(); _normalImage->setUserInteractionEnabled(false); _normal->addSubview(_normalImage); }
    if (!_selectedImage) { _selectedImage=new_shared<UIImageView>(); _selectedContent->addSubview(_selectedImage); }
    _normalImage->setImage(normal); _selectedImage->setImage(selected);
    for (size_t i=0;i<_count;i++) { _normalSymbols[i]->setHidden(true); _normalLabels[i]->setHidden(true); _selectedSymbols[i]->setHidden(true); _selectedLabels[i]->setHidden(true); }
    arrange();
}
void LensTabBarView::setComparisonGeometry(NXRect frame) { _comparisonFrame=frame; arrange(); }
void LensTabBarView::arrange() {
    const auto w=std::max(0.0f,bounds().width()),h=std::max(0.0f,bounds().height());
    _glass->setFrame({bounds().origin.x,bounds().origin.y,w,h}); _glass->layer()->setCornerRadius(h*.5f); _normal->setFrame({0,0,w,h});
    _normal->mask()->setFrame({0,0,w,h});
    _selectedHost->setFrame({0,0,w,h});
    std::vector<float> left(_count),width(_count);
    if (_compact && _count==3) { const auto scale=w/293.5f; left[0]=4*scale; left[1]=92*scale; left[2]=195.5f*scale; width[0]=84*scale; width[1]=99.5f*scale; width[2]=94*scale; }
    else for (size_t i=0;i<_count;i++) { left[i]=i*w/_count+(w/_count-std::min(116.0f,w/_count-4))*.5f; width[i]=std::max(0.0f,std::min(116.0f,w/_count-4)); }
    _selectionStep=w/_count;
    if(_count==4) {
        const auto itemWidth=std::max(0.0f,std::min(116.0f,w/4+5));
        _selectionStep=std::max(0.0f,(w-8-itemWidth)/3);
        for(size_t i=0;i<_count;++i) { left[i]=4+i*_selectionStep; width[i]=itemWidth; }
    }
    const int a=std::min(int(_count)-2,int(_position)),b=a+1; const float t=_position-a;
    const auto growth=_lifted?8.0f:0.0f;
    const auto lensW=std::max(0.0f,width[a]+(width[b]-width[a])*t+growth);
    const auto lensH=std::max(0.0f,(_compact?h-8:std::min(56.0f,h-8))+growth);
    const auto centerX=(left[a]+width[a]*.5f)*(1-t)+(left[b]+width[b]*.5f)*t;
    const auto lensFrame=_comparisonFrame.value_or(NXRect(centerX-lensW*.5f,(h-lensH)*.5f,lensW,lensH));
    _lens->setFrame(lensFrame); _selectedContent->setFrame({0,0,w,h});
    for (size_t i=0;i<_count;i++) {
        _controls[i]->setFrame({i*w/_count,0,w/_count,h}); _controls[i]->layer()->setCornerRadius(h*.5f);
        float localX=w/(_count*2)-12;
        if (_compact && _count==3) {
            const float textWidth=i==0?45.0f:(i==1?70.5f:65.0f);
            const auto start=left[i]+(width[i]-textWidth-28)*.5f;
            _normalSymbols[i]->setFrame({start-i*w/_count,(h-24)*.5f,24,24}); _selectedSymbols[i]->setFrame({start,(h-24)*.5f,24,24});
            _normalLabels[i]->setFrame({start+28-i*w/_count,0,textWidth,h}); _selectedLabels[i]->setFrame({start+28,0,textWidth,h});
        } else if(_count==4) {
            const auto center=left[i]+width[i]*.5f, slot=w/_count;
            _normalSymbols[i]->setFrame({center-i*slot-12,7,24,24});
            _selectedSymbols[i]->setFrame({center-12,7,24,24});
            _normalLabels[i]->setFrame({center-i*slot-slot*.5f,h-27,slot,18});
            _selectedLabels[i]->setFrame({center-slot*.5f,h-27,slot,18});
        } else {
            _normalSymbols[i]->setFrame({localX,std::min(13.0f,h-49),24,24}); _selectedSymbols[i]->setFrame({i*w/_count+localX,std::min(13.0f,h-49),24,24});
            _normalLabels[i]->setFrame({0,h-32,w/_count,18}); _selectedLabels[i]->setFrame({i*w/_count,h-32,w/_count,18});
        }
    }
    if (_normalImage) _normalImage->setFrame({0,0,w,h});
    if (_selectedImage) _selectedImage->setFrame({0,0,w,h});
}
void LensTabBarView::layoutSubviews() { UIView::layoutSubviews(); arrange(); }

void LensTabBarView::setAccentColor(UIColor color) {
    for(const auto& label:_selectedLabels) label->setTextColor(color);
    const auto recolor=[&](const auto& self,const std::shared_ptr<UIView>& view)->void {
        if(auto symbol=std::dynamic_pointer_cast<PhoneTabSymbol>(view)) symbol->setColor(color);
        if(view->backgroundColor()) view->setBackgroundColor(color);
        for(const auto& child:view->subviews()) self(self,child);
    };
    for(const auto& icon:_selectedSymbols) recolor(recolor,icon);
}
