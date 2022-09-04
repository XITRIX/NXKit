//
//  ControlTestViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include "ControlTestViewController.hpp"

#include "ViewController.hpp"
#include "TouchTestViewController.hpp"
#include "AnimationTestViewController.hpp"
#include "ListViewController.hpp"
#include "TableViewTestController.hpp"

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

    control->addAction(BUTTON_A, UIAction([this]() {
        TouchTestViewController* touchVC = new TouchTestViewController();
        AnimationTestViewController* animVC = new AnimationTestViewController();

        UITabBarController* tabsVC = new UITabBarController({animVC, nullptr, touchVC });
        UINavigationController* vc = new UINavigationController(tabsVC);

        present(vc, true);
    }));

    setView(view);
}
