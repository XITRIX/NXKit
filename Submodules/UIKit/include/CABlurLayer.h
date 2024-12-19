#pragma once

#include <CALayer.h>

namespace NXKit {

class CABlurLayer: public CALayer {
public:
    CABlurLayer();
    CABlurLayer(CABlurLayer* layer);
    virtual ~CABlurLayer() = default;

    void draw(SkCanvas* context) override;
    std::shared_ptr<CALayer> copy() override;

    void setBlurValue(NXFloat blurValue);
    [[nodiscard]] NXFloat blurValue() const { return _blurValue; }

    std::optional<AnimatableProperty> value(std::string forKeyPath) override;
protected:
    void update(std::shared_ptr<CALayer> presentation, std::shared_ptr<CABasicAnimation> animation, float progress) override;

private:
    NXFloat _blurValue = 10;
};

}
