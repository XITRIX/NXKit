#pragma once

#include <CALayer.h>

namespace NXKit {

class CABlurLayer: public CALayer {
public:
    CABlurLayer();
    explicit CABlurLayer(CABlurLayer* layer);
    ~CABlurLayer() override = default;

    void draw(SkCanvas* context) override;
    std::shared_ptr<CALayer> copy() override;

    void setBlurRadius(NXFloat blurRadius);
    [[nodiscard]] NXFloat blurRadius() const { return _blurRadius; }

    void setBackgroundTintColor(const UIColor& backgroundTintColor) { _backgroundTintColor = backgroundTintColor; }
    [[nodiscard]] UIColor backgroundTintColor() const { return _backgroundTintColor; }

    std::optional<AnimatableProperty> value(std::string forKeyPath) override;
    
protected:
    void update(std::shared_ptr<CALayer> presentation, std::shared_ptr<CABasicAnimation> animation, float progress) override;
    UIColor _backgroundTintColor = UIColor::clear;

private:
    NXFloat _blurRadius = 16; // 10;
};

}
