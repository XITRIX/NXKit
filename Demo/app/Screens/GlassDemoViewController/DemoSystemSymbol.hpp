#pragma once
#include <UIKit.h>
#include <algorithm>
#include <utility>

// The system supplies SF Symbols on Apple platforms. Keep a caller-provided
// portable fallback instead of packaging an Apple system font in the Demo.
class DemoSystemSymbol final : public NXKit::UIView {
public:
    DemoSystemSymbol(std::string name,NXKit::NXFloat points,NXKit::NXFloat weight,
                     NXKit::UIColor color,std::shared_ptr<NXKit::UIView> fallback):
        _name(std::move(name)),_points(points),_weight(weight),_fallback(std::move(fallback)) {
        setUserInteractionEnabled(false); setTintColor(color);
        if(_fallback) addSubview(_fallback);
        _image=NXKit::new_shared<NXKit::UIImageView>(); addSubview(_image); updateSymbol();
    }
    void layoutSubviews() override {
        UIView::layoutSubviews();
        if(_fallback) _fallback->setFrame(bounds());
        if(const auto image=_image->image()) {
            const auto size=image->size();
            _image->setFrame(_fitsBounds?bounds():NXKit::NXRect((bounds().width()-size.width)*.5f,(bounds().height()-size.height)*.5f,size.width,size.height));
        }
    }
    void traitCollectionDidChange(std::shared_ptr<NXKit::UITraitCollection> previous) override {
        UIView::traitCollectionDidChange(previous); updateSymbol();
    }
    bool hasSystemImage() const { return bool(_image->image()); }
    void setFitsSymbolToBounds(bool value) { _fitsBounds=value;_image->setContentMode(value?NXKit::UIViewContentMode::scaleAspectFit:NXKit::UIViewContentMode::scaleToFill);setNeedsLayout(); }
private:
    void updateSymbol() {
        const auto traits=traitCollection();
        const auto scale=traits?std::max(1.0f,traits->displayScale()):1;
        if(_scale==scale) return;
        _scale=scale; _image->setImage(NXKit::UIImage::systemImageNamed(_name,_points,_weight,scale));
        if(_fallback) _fallback->setHidden(hasSystemImage());
        setNeedsLayout();
    }
    std::string _name;
    NXKit::NXFloat _points,_weight,_scale=0;
    bool _fitsBounds=false;
    std::shared_ptr<NXKit::UIView> _fallback;
    std::shared_ptr<NXKit::UIImageView> _image;
};
