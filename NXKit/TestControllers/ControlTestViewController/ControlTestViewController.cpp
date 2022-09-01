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

    auto control = new UIControl();
    control->setAlignItems(NXKit::AlignItems::CENTER);
    control->setJustifyContent(NXKit::JustifyContent::CENTER);
    control->setSize({ 280, 44 });

    auto label = new UILabel("Test text");
    control->addSubview(label);

    view->addSubview(control);

    setView(view);
}
