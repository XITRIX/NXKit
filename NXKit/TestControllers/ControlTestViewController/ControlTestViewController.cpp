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

    auto control = new UIButton("Present example");
    control->setSize({ 280, 60 });
    view->addSubview(control);

    auto control2 = new UIButton("Show example");
    control2->setSize({ 280, 60 });
    control2->setMarginTop(9);
    view->addSubview(control2);

    auto control3 = new UIButton("Alert example");
    control3->setSize({ 280, 60 });
    control3->setMarginTop(9);
    view->addSubview(control3);

//    auto task = new GroupTask([]() {
//        printf("Group test");
//    });
//    task->enter();
//
//    view->animate(3, []() {}, EasingFunction::linear, [task](bool res) {
//        task->leave();
//    });

    control->addAction(BUTTON_A, UIAction([this]() {
        TouchTestViewController* touchVC = new TouchTestViewController();
        AnimationTestViewController* animVC = new AnimationTestViewController();

        UITabBarController* tabsVC = new UITabBarController({animVC, nullptr, touchVC });
        UINavigationController* vc = new UINavigationController(tabsVC);

        present(vc, true);
    }, "OK"));

    control2->addAction(BUTTON_A, UIAction([this]() {
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//        TouchTestViewController* touchVC = new TouchTestViewController();
        ListViewController* listVC = new ListViewController();
        show(listVC);
    }, "OK"));

    control3->addAction(BUTTON_A, UIAction([this]() {
        auto alert = new UIAlertController("Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title", "Just some text to fill message label Just some text to fill message label Just some text to fill message label Just some text to fill message label");
        alert->addAction(UIAlertAction("Cancel", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("Cancel\n"); }));
        alert->addAction(UIAlertAction("Ok", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("OK\n"); }));
        alert->addAction(UIAlertAction("Delete", NXKit::UIAlertActionStyle::DESTRUCTIVE, []() { printf("Delete\n"); }));
        present(alert, true);
    }, "OK"));

    setView(view);
}
