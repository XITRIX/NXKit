#include "GlassDemoScene.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace NXKit;
namespace {
// Keep controller focus feedback inside the fixed control. Touch inflation is
// supplied solely by UIGlassEffect, so no application transform moves the glass.
class FixedGlassControl final : public UIControl {
public:
    void willGainFocus() override {
        layer()->setBorderColor(UIColor::tint);
        layer()->setBorderWidth(2);
    }
    void willLoseFocus() override {
        layer()->setBorderWidth(0);
        setAlpha(1);
    }
    void willChangeFocusHighlight(bool highlighted) override { setAlpha(highlighted ? .65f : 1); }
};
}
GlassDemoScene::GlassDemoScene(GlassDemoID demo, const std::shared_ptr<UIImage>& artwork):
    UIView(NXRect(0,0,720,440)), _demo(demo) {
    if (demo<0 || demo>=GlassDemoCount) throw std::invalid_argument("Unknown glass demo");
    setClipsToBounds(true);
    _artwork = new_shared<UIImageView>(artwork);
    _artwork->setContentMode(UIViewContentMode::scaleToFill);
    _artwork->setFrame({0,0,720,440}); addSubview(_artwork);
    for (int i=0;i<3;++i) {
        auto panel=new_shared<UIView>(NXRect(i*240.0f,0,240,440));
        _panels.push_back(panel); addSubview(panel);
    }
    std::shared_ptr<UIView> host=shared_from_this();
    if (GlassDemos[demo].grouped) {
        UIGlassContainerEffect group; group.setSpacing(GlassDemoContainerSpacing(demo));
        _container=new_shared<GlassContainerView>(group); _container->setFrame(bounds());
        addSubview(_container); host=_container->contentView();
    }
    for (int i=0;i<GlassDemoElementCount(demo);++i) {
        const auto spec=GlassDemoElementAt(demo,i,0);
        UIGlassEffect glass(GlassDemos[demo].clear ? UIGlassEffect::Style::clear : UIGlassEffect::Style::regular);
        glass.setInteractive(GlassDemos[demo].interactive);
        glass.setAdaptsAppearanceToBackdrop(GlassDemos[demo].legacy);
        if (spec.tint) { unsigned char r,g,b; GlassDemoTint(spec.tint,&r,&g,&b); glass.setTintColor(UIColor(r,g,b)); }
        auto item=new_shared<BackdropEffectView>(glass);
        item->layer()->setCornerCurve(spec.circular ? CALayerCornerCurve::circular : CALayerCornerCurve::continuous);
        host->addSubview(item); _items.push_back(item);
        auto symbol=new_shared<UIView>(); symbol->setUserInteractionEnabled(false);
        if (GlassDemoHasButtons(demo)) {
            auto control=new_shared<FixedGlassControl>();
            const auto weak=weak_from_base<GlassDemoScene>();
            control->primaryAction=UIAction(i==0 && GlassDemoIsNavigationButtons(demo) ? "Back" : (i==1 ? "More" : "Add"),[weak,i] {
                if (const auto scene=weak.lock()) {
                    auto control=scene->_controls[i];
                    control->setSelected(!control->isSelected());
                    scene->updateSymbol(i);
                }
            });
            control->addSubview(symbol); item->addSubview(control); _controls.push_back(control);
        } else {
            for (const auto& frame : {NXRect(0,9,20,2),NXRect(9,0,2,20)}) {
                auto stroke=new_shared<UIView>(frame); stroke->setBackgroundColor(UIColor::label); symbol->addSubview(stroke);
            }
            item->addSubview(symbol);
        }
        _symbols.push_back(symbol);
        if (GlassDemoHasButtons(demo)) updateSymbol(i);
        if (GlassDemos[demo].movable) makeDraggable(item);
    }
    reset();
}
void GlassDemoScene::setBackdrop(int backdrop) {
    if (backdrop<0 || backdrop>4) throw std::invalid_argument("Unknown glass backdrop");
    _backdrop=backdrop; _artwork->setHidden(backdrop!=0);
    const UIColor colors[] = {UIColor(31,31,31),UIColor(31,31,31),UIColor::black,UIColor::white};
    setBackgroundColor(colors[std::min(backdrop,3)]);
    for (int i=0;i<3;++i) {
        _panels[i]->setHidden(backdrop!=4);
        _panels[i]->setBackgroundColor(i==0 ? UIColor::black : (i==1 ? UIColor::white : UIColor(127,127,127)));
    }
}
void GlassDemoScene::setProgress(float progress) {
    if (!std::isfinite(progress)) return;
    _progress=std::clamp(progress,0.0f,1.0f);
    for (size_t i=0;i<_items.size();++i) {
        const auto e=GlassDemoElementAtSize(_demo,i,_progress,bounds().width(),bounds().height());
        _items[i]->setFrame({e.x,e.y,e.width,e.height}); _items[i]->layer()->setCornerRadius(e.radius);
        _symbols[i]->setFrame({(e.width-20)*.5f,(e.height-20)*.5f,20,20});
        if (!_controls.empty()) {
            _controls[i]->setFrame({0,0,e.width,e.height});
            _controls[i]->layer()->setCornerRadius(e.radius);
        }
    }
}
void GlassDemoScene::reset() {
    for (const auto& item:_items) { item->layer()->removeAllAnimations(); }
    for (const auto& symbol:_symbols) symbol->layer()->removeAllAnimations();
    for (size_t i=0;i<_controls.size();++i) {
        _controls[i]->setSelected(false);
        _controls[i]->setHighlighted(false);
        updateSymbol(i);
    }
    setReversedOrder(false);
    setProgress(0); setBackdrop(_demo==GlassDemoContrast26 ? 4 : 0);
}
void GlassDemoScene::setReversedOrder(bool reversed) {
    if (_reversedOrder==reversed) return;
    _reversedOrder=reversed;
    const auto host=_items.front()->superview().lock();
    // Reorder both view and layer trees so touch delivery follows the visible front item.
    if (reversed) { for (auto i=_items.rbegin();i!=_items.rend();++i) host->addSubview(*i); }
    else { for (const auto& item:_items) host->addSubview(item); }
}
void GlassDemoScene::updateSymbol(size_t index) {
    const auto symbol=_symbols[index];
    const auto old=symbol->subviews();
    for (const auto& stroke:old) stroke->removeFromSuperview();
    const auto kind=_controls[index]->isSelected() ? GlassDemoCheck : GlassDemoSymbolForItem(_demo,index);
    const auto spec=GlassDemoElementAt(_demo,index,0);
    for (int i=0;i<GlassDemoStrokeCount(kind);++i) {
        const auto line=GlassDemoStrokeAt(kind,i);
        auto stroke=new_shared<UIView>(NXRect(line.x,line.y,line.width,line.height));
        stroke->layer()->setCornerRadius(line.radius);
        stroke->setBackgroundColor(spec.tint ? UIColor::white : UIColor::label);
        stroke->setTransform(NXAffineTransform::rotationBy(line.angle));
        symbol->addSubview(stroke);
    }
}
void GlassDemoScene::layoutSubviews() {
    UIView::layoutSubviews();
    if (!GlassDemoHasButtons(_demo)) return;
    _artwork->setFrame(bounds());
    if (_container) _container->setFrame(bounds());
    for (int i=0;i<3;++i) _panels[i]->setFrame({i*bounds().width()/3,0,bounds().width()/3,bounds().height()});
    setProgress(_progress);
}
void GlassDemoScene::makeDraggable(const std::shared_ptr<BackdropEffectView>& item) {
    auto pan=new_shared<UIPanGestureRecognizer>();
    const auto weakScene=weak_from_base<GlassDemoScene>();
    auto origin=std::make_shared<NXPoint>();
    pan->onStateChanged=[weakScene,origin](const auto& gesture) {
        const auto scene=weakScene.lock(); if (!scene) return;
        auto pan=std::static_pointer_cast<UIPanGestureRecognizer>(gesture); const auto item=pan->view().lock(); if (!item) return;
        if (pan->state()==UIGestureRecognizerState::began) *origin=item->frame().origin;
        if (pan->state()==UIGestureRecognizerState::changed) {
            auto frame=item->frame(); frame.origin=*origin+pan->translationInView(scene);
            frame.origin.x=std::clamp(frame.origin.x,0.0f,720-frame.width());
            frame.origin.y=std::clamp(frame.origin.y,0.0f,440-frame.height()); item->setFrame(frame);
        }
    };
    item->addGestureRecognizer(pan);
}
