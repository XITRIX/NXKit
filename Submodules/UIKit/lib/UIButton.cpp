#include <UIButton.h>

namespace NXKit {

using namespace yoga::literals;

UIButton::UIButton(UIButtonStyle style) {
    _titleLabel = new_shared<UILabel>();
    _imageView = new_shared<UIImageView>();

    _titleLabel->setFontSize(17);
    _titleLabel->setFontWeight(600);

    _imageView->setHidden(true);
    _imageView->setContentMode(UIViewContentMode::center);

    addSubview(_imageView);
    addSubview(_titleLabel);

    _titleLabel->setAutolayoutEnabled(true);
    _imageView->setAutolayoutEnabled(true);

//    _titleLabel->setTextAlignment(NSTextAlignment::center);

    configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setAlignContent(YGAlignFlexEnd);
        layout->setJustifyContent(YGJustifyCenter);
        layout->setAllGap(8);
        layout->setHeight(50_pt);
        layout->setPaddingHorizontal(8_pt);
//        layout->setSize({ 250, 50 });
    });

    applyStyle(style);
}

void UIButton::tintColorDidChange() {
    UIView::tintColorDidChange();
}

void UIButton::setText(const std::string& text) {
    _titleLabel->setText(text);
    _titleLabel->setHidden(text.empty());
}

void UIButton::setImage(const std::shared_ptr<UIImage>& image) {
    _imageView->setImage(image);
    _imageView->setHidden(image == nullptr);
}

void UIButton::baseScaleMultiplierDidChange() {
    _titleLabel->setBaseScaleMultiplier(baseScaleMultiplier());
}

//void UIButton::willGainFocus() {
//    setTransform(NXAffineTransform::scale(1.04f));
//    _titleLabel->setBaseScaleMultiplier(1.04f);
//    layer()->setShadowColor(UIColor::black);
//    layer()->setShadowOpacity(0.2);
//    layer()->setShadowOffset({0, 8});
//    layer()->setShadowRadius(18);
//}
//
//void UIButton::willLoseFocus() {
//    setTransform(NXAffineTransform::identity);
//    _titleLabel->setBaseScaleMultiplier(1);
//    layer()->setShadowOpacity(0);
//    layer()->setShadowRadius(0);
//}

void UIButton::applyStyle(UIButtonStyle style) {
    switch (style) {
        case UIButtonStyle::plain:
            setBackgroundColor(UIColor::clear);
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(UIColor::tint);
            break;
        case UIButtonStyle::gray:
            setBackgroundColor(UIColor::systemGray);
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(UIColor::tint);
            break;
        case UIButtonStyle::tinted:
            setBackgroundColor(UIColor::tint.withAlphaComponent(0.2f));
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(UIColor::tint);
            break;
        case UIButtonStyle::filled:
            setBackgroundColor(UIColor::tint);
            _titleLabel->setTextColor(UIColor::white);
            _imageView->setTintColor(UIColor::white);
            break;
    }
}
}