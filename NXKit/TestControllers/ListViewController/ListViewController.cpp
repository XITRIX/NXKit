//
//  ListViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#include "ListViewController.hpp"

ListItemView::ListItemView() {
    setHeight(70);
    setPaddingTop(12.5f);
    setPaddingBottom(12.5f);

    setPadding(12.5f, 20, 12.5f, 20);

    setAxis(Axis::HORIZONTAL);
    setAlignItems(AlignItems::CENTER);

//    backgroundColor = UIColor::red;

    label = new UILabel("Test text");
    label->verticalAlign = VerticalAlign::CENTER;
//    label->backgroundColor = UIColor::blue;
    label->getFont()->fontSize = 22;
    addSubview(label);
}

void ListItemView::setText(std::string text) {
    label->setText(text);
}

bool ListItemView::canBecomeFocused() {
    return true;
}

ListViewController::ListViewController() {
    setTitle("List view");
}

void ListViewController::loadView() {
    scrollView = new UIScrollView();
    scrollView->setFixWidth(true);
    scrollView->scrollingMode = UIScrollViewScrollingMode::scrollingEdge;

    stackView = new UIStackView();
    stackView->setPadding(32, 80, 47, 40);

    scrollView->addSubview(stackView);

    setView(scrollView);
}

void ListViewController::viewDidLoad() {
    for (int i = 0; i < 20; i++) {
        auto item = new ListItemView();
        item->setText("Test text #" + std::to_string(i + 1));
        stackView->addSubview(item);
    }
}
