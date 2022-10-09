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

    imageView = NXKit::make_shared<UIImageView>();
    imageView->setSize({ 44, 44 });
    imageView->setMarginRight(20);

    label = NXKit::make_shared<UILabel>("Test text");
    label->setGrow(1);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;

    detailLabel = NXKit::make_shared<UILabel>();
    detailLabel->verticalAlign = VerticalAlign::CENTER;
    detailLabel->horizontalAlign = HorizontalAlign::RIGHT;
    detailLabel->textColor = getTintColor();
    detailLabel->getFont()->fontSize = 22;

    addSubview(imageView);
    addSubview(label);
    addSubview(detailLabel);

    onEvent = [this](UIControlTouchEvent event) {
        if (event == NXKit::UIControlTouchEvent::touchUpInside) {
            getFirstAvailableAction(BUTTON_A).action();
        }
    };
}

void UITableViewDefaultCell::setText(std::string text) {
    label->setText(text);
}

void UITableViewDefaultCell::setDetailText(std::string text) {
    detailLabel->setText(text);
}

void UITableViewDefaultCell::setImage(std::shared_ptr<UIImage> image) {
    imageView->setHidden(!image);
    imageView->setImage(image);
}

void UITableViewDefaultCell::tintColorDidChange() {
    UIView::tintColorDidChange();
    detailLabel->textColor = getTintColor();
}

//bool UITableViewDefaultCell::canBecomeFocused() {
//    return true;
//}


}
