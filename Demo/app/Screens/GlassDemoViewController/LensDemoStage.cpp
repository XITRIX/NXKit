#include "LensDemoStage.hpp"
#include <algorithm>
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
using namespace NXKit;
namespace {
class RefractionGridLayer final : public CALayer {
public:
    RefractionGridLayer() = default;
    explicit RefractionGridLayer(RefractionGridLayer* old):CALayer(old) {}
    std::shared_ptr<CALayer> copy() override { return new_shared<RefractionGridLayer>(this); }
    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        for (float x=0;x<bounds().width();x+=40)
            canvas->drawRect(SkRect::MakeXYWH(x,0,3,bounds().height()),paint);
        paint.setColor(SK_ColorBLACK);
        for (float y=0;y<bounds().height();y+=40)
            canvas->drawRect(SkRect::MakeXYWH(0,y,bounds().width(),3),paint);
    }
};
class LensTouchControl final : public UIControl {
public:
    std::function<void(bool)> onHighlight;
    void willChangeHighlight(bool pressed) override { if (onHighlight) onHighlight(pressed); }
};
}
LensDemoStage::LensDemoStage(bool tabBarOnly): UIView({0,0,720,440}), _tabBarOnly(tabBarOnly) {
    setClipsToBounds(true);
    _artwork=new_shared<UIImageView>(UIImage::fromRes("img/glass_demo_scene.png",1)); addSubview(_artwork);
    for (int i=0;i<12;i++) { auto panel=new_shared<UIView>(); panel->setBackgroundColor(i%2?UIColor(240,100,70):UIColor(40,170,220)); addSubview(panel); _stripes.push_back(panel); }
    _grid=new_shared<UIView>(NXRect(),new_shared<RefractionGridLayer>()); _grid->setUserInteractionEnabled(false); addSubview(_grid);
    if (!_tabBarOnly) { _compact=new_shared<LensTabBarView>(true); addSubview(_compact); }
    _wide=new_shared<LensTabBarView>(false); addSubview(_wide);
    const auto weak=weak_from_base<LensDemoStage>();
    if (_compact) _compact->onSelectionChanged=[weak](size_t i) { if (auto self=weak.lock()) self->_wide->setSelectedIndex(i,true); };
    _wide->onSelectionChanged=[weak](size_t i) { if (auto self=weak.lock(); self && self->_compact) self->_compact->setSelectedIndex(i,true); };
    for (int i=0;i<(_tabBarOnly?0:3);i++) {
        auto lens=new_shared<LensView>(i==0?LensView::Style::regular:LensView::Style::selection);
        lens->setInteractive(true);
        lens->setRestingBackgroundColor(i==0?UIColor::white:UIColor::label.withAlphaComponent(.10f)); addSubview(lens); _samples.push_back(lens);
        auto control=new_shared<LensTouchControl>(); lens->contentView()->addSubview(control);
        const auto weakLens=std::weak_ptr<LensView>(lens);
        control->onHighlight=[weak,weakLens](bool pressed) {
            if (auto self=weak.lock()) if (auto sample=weakLens.lock()) sample->setLifted(pressed||self->_lifted,true);
        };
        control->primaryAction=UIAction("Lift lens",[weak] { if (auto self=weak.lock()) self->setLifted(!self->_lifted); });
    }
    setBackdrop(0);
}
void LensDemoStage::setBackdrop(int value) {
    _backdrop=((value%6)+6)%6; _artwork->setHidden(_backdrop!=0);
    _grid->setHidden(_backdrop!=5);
    setBackgroundColor(_backdrop==2?UIColor::black:(_backdrop==3?UIColor::white:UIColor(128,128,128)));
    for (const auto& view:_stripes) view->setHidden(_backdrop!=4);
}
void LensDemoStage::setLifted(bool lifted) {
    _lifted=lifted; if (_compact) _compact->setLifted(lifted,true); _wide->setLifted(lifted,true);
    for (const auto& sample:_samples) sample->setLifted(lifted,true);
}
void LensDemoStage::selectNext() { _wide->setSelectedIndex((_wide->selectedIndex()+1)%3,true); }
void LensDemoStage::toggleSize() { _large=!_large; setNeedsLayout(); layoutIfNeeded(); }
void LensDemoStage::layoutSubviews() {
    UIView::layoutSubviews(); const auto w=bounds().width(),h=bounds().height(); _artwork->setFrame(bounds()); _grid->setFrame(bounds());
    for (size_t i=0;i<_stripes.size();i++) _stripes[i]->setFrame({w*i/12,0,w/12+1,h});
    if (_tabBarOnly) {
        const auto barWidth=std::max(0.0f,std::min(640.0f,w-32));
        const auto barHeight=_large?96.0f:76.0f;
        _wide->setFrame({(w-barWidth)*.5f,std::max(0.0f,h-barHeight-24),barWidth,barHeight});
        return;
    }
    const auto compactW=std::max(0.0f,std::min(293.5f,w-24)); _compact->setFrame({(w-compactW)*.5f,40,compactW,44});
    const auto wideW=std::max(0.0f,w-40); _wide->setFrame({20,145,wideW,_large?96.0f:76.0f});
    const NXSize sizes[]={{48,48},{80,80},{_large?160.0f:120.0f,_large?96.0f:64.0f}};
    for (int i=0;i<3;i++) {
        const auto size=sizes[i]; _samples[i]->setFrame({w*(i+.5f)/3-size.width*.5f,340-size.height*.5f,size.width,size.height});
        _samples[i]->contentView()->subviews().front()->setFrame({0,0,size.width,size.height});
    }
}
