//
// Created by Даниил Виноградов on 23.01.2025.
//

#pragma once

#include <UIControl.h>
#include <UILabel.h>
#include <UIImageView.h>

namespace NXKit {

enum class UIButtonStyle {
    plain,
    gray,
    tinted,
    filled
};

class UIButton: public UIControl {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIButton>(); }
    explicit UIButton(UIButtonStyle style = UIButtonStyle::plain);

    void tintColorDidChange() override;

    std::string text() { return _titleLabel->text(); }
    void setText(const std::string& text);

    std::shared_ptr<UIImage> image() { return _imageView->image(); }
    void setImage(const std::shared_ptr<UIImage>& image);

    std::shared_ptr<UILabel> titleLabel() { return _titleLabel; }
    std::shared_ptr<UIImageView> imageView() { return _imageView; }

    void baseScaleMultiplierDidChange() override;
//    void willGainFocus() override;
//    void willLoseFocus() override;

    void applyStyle(UIButtonStyle style);
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;

private:
    std::shared_ptr<UILabel> _titleLabel;
    std::shared_ptr<UIImageView> _imageView;
};

}