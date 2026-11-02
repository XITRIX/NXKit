#include <UIButton.h>
#include <tools/IBTools.h>

namespace NXKit {

using namespace yoga::literals;

namespace {
std::optional<UIButtonStyle> valueToButtonStyle(const std::string& value) {
    if (value == "plain") {
        return UIButtonStyle::plain;
    }
    if (value == "gray") {
        return UIButtonStyle::gray;
    }
    if (value == "tinted") {
        return UIButtonStyle::tinted;
    }
    if (value == "filled") {
        return UIButtonStyle::filled;
    }
    return std::nullopt;
}
}


UIButton::UIButton(UIButtonStyle style) {
    _titleLabel = new_shared<UILabel>();
    _imageView = new_shared<UIImageView>();

    _titleLabel->setHidden(true);
    _titleLabel->setFontSize(17);
    _titleLabel->setFontWeight(600);
//    _titleLabel->setContentMode(UIViewContentMode::redraw);

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
    _titleLabel->setScaleModifier(baseScaleMultiplier());
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

namespace NXKit {

void UIButton::willGainFocus() {
    setTransform(NXAffineTransform::scale(1.06f));
    setBaseScaleMultiplier(1.1f);
    layer()->setShadowColor(UIColor::black);
    layer()->setShadowOpacity(0.4);
    layer()->setShadowOffset({0, 6});
    layer()->setShadowRadius(18);
    layer()->setZPosition(1);
}

void UIButton::willLoseFocus() {
    setTransform(NXAffineTransform::identity);
    setBaseScaleMultiplier(1);
    layer()->setShadowOpacity(0);
    layer()->setShadowRadius(0);
    layer()->setZPosition(0);
}

void UIButton::focusFailedToMove(UIFocusHeading heading) {

}

void UIButton::willChangeFocusHighlight(bool highlighted) {
    if (highlighted) {
        setTransform(NXAffineTransform::scale(1.02f));
        setBaseScaleMultiplier(1.0f);
        layer()->setShadowOffset({0, 3});
        layer()->setShadowRadius(4);
    } else {
        setTransform(NXAffineTransform::scale(1.06f));
        setBaseScaleMultiplier(1.1f);
        layer()->setShadowOffset({0, 6});
        layer()->setShadowRadius(18);
    }
}

void UIButton::willChangeHighlight(bool highlighted) {
    setAlpha(highlighted ? 0.8 : 1);
}


}
