//
//  UITableViewDefaultCell.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#include <Core/UITableViewDefaultCell/UITableViewDefaultCell.hpp>

namespace NXKit {

UITableViewDefaultCell::UITableViewDefaultCell() {
    setHeight(70);
    setPaddingTop(12.5f);
    setPaddingBottom(12.5f);

    setPadding(12.5f, 20, 12.5f, 20);

    setAxis(Axis::HORIZONTAL);
    setAlignItems(AlignItems::CENTER);

    imageView = new UIImageView();
    imageView->setSize({ 44, 44 });
    imageView->setMarginRight(20);

    label = new UILabel("Test text");
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;

    addSubview(imageView);
    addSubview(label);

    onEvent = [this](UIControlTouchEvent event) {
        if (event == NXKit::UIControlTouchEvent::touchUpInside) {
            getActions()[BUTTON_A].action();
        }
    };
}

void UITableViewDefaultCell::setText(std::string text) {
    label->setText(text);
}

void UITableViewDefaultCell::setImage(UIImage* image) {
    imageView->setImage(image);
}

bool UITableViewDefaultCell::canBecomeFocused() {
    return true;
}


}
