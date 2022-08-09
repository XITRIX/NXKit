//
//  UITabBarController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include "UITabBarController.hpp"
#include "UIStackView.hpp"
#include "UILabel.hpp"

namespace NXKit {

UITabBarItem::UITabBarItem() {
    setAxis(Axis::HORIZONTAL);
    setHeight(70);

    selectionBar = new UIView();
    selectionBar->backgroundColor = UIColor(49, 79, 235);
    selectionBar->setWidth(4);
    selectionBar->setMargins(9, 8, 9, 8);

    label = new UILabel("Test Item");
    label->setGrow(1);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;
    label->setMargins(9, 8, 9, 0);

    addSubview(selectionBar);
    addSubview(label);

    setSelected(false);
    canBecomeFocused = true;
}

void UITabBarItem::setTitle(std::string text) {
    label->setText(text);
}

bool UITabBarItem::isSelected() {
    return selected;
}

void UITabBarItem::becomeFocused() {
    setSelected(true);
}

void UITabBarItem::resignFocused() {
    setSelected(false);
}

void UITabBarItem::setSelected(bool selected) {
    this->selected = selected;
    if (selected) {
        label->textColor = UIColor(49, 79, 235);
        selectionBar->backgroundColor = UIColor(49, 79, 235);
    } else {
        label->textColor = UIColor(0, 0, 0);
        selectionBar->backgroundColor = UIColor(0, 0, 0, 0);
    }
}

UITabBarController::UITabBarController(UIViewController* content): content(content) { }

void UITabBarController::loadView() {
    UIStackView* view = new UIStackView(Axis::HORIZONTAL);

    tabs = new UIStackView(Axis::VERTICAL);
    tabs->setPercentWidth(32);
    tabs->backgroundColor = UIColor(240, 240, 240);
    tabs->setPadding(32, 40, 47, 80);

    contentView = new UIView();
    contentView->setGrow(1);
    //    contentView->clipToBounds = true;
    //    contentView->backgroundColor = UIColor(255, 0, 0);
    contentView->addSubview(this->content->getView());
    //    contentView->setPaddingRight(140);
    //    contentView->setMarginRight(140);
    //    contentView->backgroundColor = UIColor(0, 255, 0);

    view->addSubview(tabs);
    view->addSubview(contentView);

    setView(view);
}

void UITabBarController::viewDidLoad() {
    for (int i = 0; i < 5; i++) {
        UITabBarItem* item = new UITabBarItem();
        item->setTitle("Hello world #" + std::to_string(i));
//        if (i == 2) item->setSelected(true);
        item->tag = "Num" + std::to_string(i);
        tabs->addSubview(item);
    }
    tabs->clipToBounds = true;
//    tabs->setBounds({ {0, 80}, tabs->frame.size() });
}


void UITabBarController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    content->getView()->setSize(contentView->frame.size());
}

}
