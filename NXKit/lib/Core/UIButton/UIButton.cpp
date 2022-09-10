//
//  UIButton.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 04.09.2022.
//

#include <Core/UIButton/UIButton.hpp>

namespace NXKit {

UIButton::UIButton(std::string title) {
    setAlignItems(NXKit::AlignItems::CENTER);
    setJustifyContent(NXKit::JustifyContent::CENTER);
    label = new UILabel(title);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 18;
    addSubview(label);

    cornerRadius = 4;

    highlightCornerRadius = 4;
    clipToBounds = false;

    setStyle(UIButtonStyle::PRIMARY);
}

void UIButton::setTitle(std::string title) {
    label->setText(title);
}

void UIButton::becomeFocused() {
    UIControl::becomeFocused();
    cornerRadius = 2;
}

void UIButton::resignFocused() {
    UIControl::resignFocused();
    cornerRadius = 4;
}

void UIButton::setStyle(UIButtonStyle style) {
    this->style = style;
    switch (style) {
        case UIButtonStyle::DEFAULT:
            backgroundColor = UIColor::white;
            label->textColor = UIColor::label;
            highlightSpacing = 0;
            borderThickness = 0;
            showShadow = true;
            break;
        case UIButtonStyle::PRIMARY:
            backgroundColor = UIColor::systemTint;
            label->textColor = UIColor::white;
            highlightSpacing = 2;
            borderThickness = 0;
            showShadow = true;
            break;
        case UIButtonStyle::HIGHLIGHT:
            backgroundColor = UIColor::white;
            label->textColor = UIColor::systemTint;
            highlightSpacing = 0;
            borderThickness = 0;
            showShadow = true;
            break;
        case UIButtonStyle::BORDERED:
            backgroundColor = UIColor::clear;
            label->textColor = UIColor::label;
            borderColor = UIColor::black;
            highlightSpacing = 0;
            borderThickness = 1.5f;
            showShadow = false;
            break;
        case UIButtonStyle::BORDERLESS:
            backgroundColor = UIColor::clear;
            label->textColor = UIColor::label;
            highlightSpacing = 0;
            borderThickness = 0;
            showShadow = false;
            break;
    }
}

}
