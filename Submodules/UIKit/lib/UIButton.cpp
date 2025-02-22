#include <UIButton.h>
#include <tools/IBTools.h>

namespace NXKit {

using namespace yoga::literals;

UIButton::UIButton(UIButtonStyle style) {
    _titleLabel = new_shared<UILabel>();
    _imageView = new_shared<UIImageView>();

    _titleLabel->setHidden(true);
    _titleLabel->setFontSize(17);
    _titleLabel->setFontWeight(600);

    _imageView->setHidden(true);
    _imageView->setContentMode(UIViewContentMode::scaleAspectFit);

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

void UIButton::applyStyle(UIButtonStyle style) {
    switch (style) {
        case UIButtonStyle::plain:
            setBackgroundColor(UIColor::clear);
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(std::nullopt);
            break;
        case UIButtonStyle::gray:
            setBackgroundColor(UIColor::systemGray);
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(std::nullopt);
            break;
        case UIButtonStyle::tinted:
            setBackgroundColor(UIColor::tint.withAlphaComponent(0.2f));
            _titleLabel->setTextColor(UIColor::tint);
            _imageView->setTintColor(std::nullopt);
            break;
        case UIButtonStyle::filled:
            // Check why it not work without `withAlphaComponent` part
            setBackgroundColor(UIColor::tint.withAlphaComponent(1));
            _titleLabel->setTextColor(UIColor::white);
            _imageView->setTintColor(UIColor::white);
            break;
    }
}

bool UIButton::applyXMLAttribute(const std::string& name, const std::string& value) {
    if (UIControl::applyXMLAttribute(name, value)) return true;

    REGISTER_XIB_ATTRIBUTE(text, valueToString, setText)
    REGISTER_XIB_ATTRIBUTE(style, valueToButtonStyle, applyStyle)
    REGISTER_XIB_ATTRIBUTE(imagePath, valuePathToImage, setImage)
    REGISTER_XIB_ATTRIBUTE(image, valueResToImage, setImage)

    return false;
}
}