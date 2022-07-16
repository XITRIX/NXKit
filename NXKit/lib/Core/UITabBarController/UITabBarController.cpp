//
//  UITabBarController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include "UITabBarController.hpp"
#include "UIStackView.hpp"
#include "UILabel.hpp"

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
}

void UITabBarItem::setTitle(std::string text) {
    label->setText(text);
}

bool UITabBarItem::isSelected() {
    return selected;
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

void UITabBarController::loadView() {
    UIStackView* view = new UIStackView(Axis::HORIZONTAL);

    tabs = new UIStackView(Axis::VERTICAL);
    tabs->setPercentWidth(32);
    tabs->backgroundColor = UIColor(240, 240, 240);
    tabs->setPadding(32, 40, 47, 80);

    UIView* content = new UIView();
    content->setGrow(1);
//    content->backgroundColor = UIColor(0, 255, 0);

    view->addSubview(tabs);
    view->addSubview(content);

    setView(view);
}

void UITabBarController::viewDidLoad() {
    for (int i = 0; i < 5; i++) {
        UITabBarItem* item = new UITabBarItem();
        item->setTitle("Hello world #" + std::to_string(i));
        if (i == 2) item->setSelected(true);
        tabs->addSubview(item);
    }
}
