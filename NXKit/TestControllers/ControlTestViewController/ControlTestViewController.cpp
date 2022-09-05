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

    auto control = new UIButton("Test button");
    control->setSize({ 280, 60 });
//    control->setStyle(NXKit::UIButtonStyle::HIGHLIGHT);
    view->addSubview(control);

    auto task = new GroupTask([]() {
        printf("Group test");
    });
    task->enter();
    
    view->animate(3, []() {}, EasingFunction::linear, [task](bool res) {
        task->leave();
    });

//    control->addAction(BUTTON_A, UIAction([this]() {
//        TouchTestViewController* touchVC = new TouchTestViewController();
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//
//        UITabBarController* tabsVC = new UITabBarController({animVC, nullptr, touchVC });
//        UINavigationController* vc = new UINavigationController(tabsVC);
//
//        present(vc, true);
//    }));

    control->addAction(BUTTON_A, UIAction([this]() {
        auto alert = new UIAlertController("Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title", "Just some text to fill message label Just some text to fill message label Just some text to fill message label Just some text to fill message label");
        alert->addAction(UIAlertAction("Cancel", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("Cancel\n"); }));
        alert->addAction(UIAlertAction("Ok", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("OK\n"); }));
        alert->addAction(UIAlertAction("Delete", NXKit::UIAlertActionStyle::DESTRUCTIVE, []() { printf("Delete\n");


            
        }));
        present(alert, true);
    }));

    setView(view);
}
