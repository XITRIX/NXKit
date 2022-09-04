//
//  ControlTestViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include "ControlTestViewController.hpp"

ControlTestViewController::ControlTestViewController() {
    setTitle("Control test");
}

void ControlTestViewController::loadView() {
    UIStackView* view = new UIStackView();
    view->setAlignItems(AlignItems::CENTER);
    view->setJustifyContent(JustifyContent::CENTER);

    auto control = new UIButton();
    control->setSize({ 280, 60 });
//    control->setStyle(NXKit::UIButtonStyle::HIGHLIGHT);
    view->addSubview(control);

    setView(view);
}
