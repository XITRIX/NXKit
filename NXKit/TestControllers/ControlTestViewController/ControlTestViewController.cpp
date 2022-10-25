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
    auto view = NXKit::make_shared<UIStackView>();
    view->setAlignItems(AlignItems::CENTER);
    view->setJustifyContent(JustifyContent::CENTER);

    auto control = NXKit::make_shared<UIButton>("Present example");
    control->setSize({ 280, 60 });
    view->addSubview(control);

    auto control2 = NXKit::make_shared<UIButton>("Show example");
    control2->setSize({ 280, 60 });
    control2->setMarginTop(9);
    view->addSubview(control2);

    auto control3 = NXKit::make_shared<UIButton>("Alert example");
    control3->setSize({ 280, 60 });
    control3->setMarginTop(9);
    view->addSubview(control3);

    auto control4 = NXKit::make_shared<UIButton>("Bottom sheet example");
    control4->setSize({ 280, 60 });
    control4->setMarginTop(9);
    view->addSubview(control4);

    auto control5 = NXKit::make_shared<UIButton>("Selector example");
    control5->setSize({ 280, 60 });
    control5->setMarginTop(9);
    view->addSubview(control5);

    control->addAction(BUTTON_A, UIAction([this]() {
        auto touchVC = NXKit::make_shared<TouchTestViewController>();
        auto animVC = NXKit::make_shared<AnimationTestViewController>();

        std::vector<std::shared_ptr<UIViewController>> controllers = {animVC, nullptr, touchVC };
        auto tabsVC = NXKit::make_shared<UITabBarController>(controllers);
        auto vc = NXKit::make_shared<UINavigationController>(tabsVC);
        tabsVC->setTitle("Presentation test");

        present(vc, true);
    }, "OK"));

    control2->addAction(BUTTON_A, UIAction([this]() {
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//        TouchTestViewController* touchVC = new TouchTestViewController();
        auto listVC = NXKit::make_shared<ListViewController>();
        show(listVC);
    }, "OK"));

    control3->addAction(BUTTON_A, UIAction([this]() {
        auto alert = NXKit::make_shared<UIAlertController>("Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title", "Just some text to fill message label Just some text to fill message label Just some text to fill message label Just some text to fill message label");
        alert->addAction(UIAlertAction("Cancel", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("Cancel\n"); }));
        alert->addAction(UIAlertAction("Ok", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("OK\n"); }));
        alert->addAction(UIAlertAction("Delete", NXKit::UIAlertActionStyle::DESTRUCTIVE, []() { printf("Delete\n"); }));
        present(alert, true);
    }, "OK"));

    control4->addAction(BUTTON_A, UIAction([this]() {
        auto listVC = NXKit::make_shared<ListViewController>();

        auto nvc = NXKit::make_shared<UINavigationController>(listVC);
        auto vc = NXKit::make_shared<UIBottomSheetController>(nvc);
//        tabsVC->setTitle("Presentation test");

        present(vc, true);
    }, "OK"));

    static int selectedCell = -1;
    control5->addAction(BUTTON_A, UIAction([this]() {
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//        TouchTestViewController* touchVC = new TouchTestViewController();
        std::vector<std::string> data;
        for (int i = 0; i < 200; i++)
            data.push_back("Cell #" + std::to_string(i + 1));

        auto selector = NXKit::make_shared<UISelectorViewController>("Demo select", data, [this](int index) {
            selectedCell = index;
            auto alert = NXKit::make_shared<UIAlertController>("Select successful", "You selected cell #" + std::to_string(index));
            alert->addAction(UIAlertAction("Oke"));
            present(alert, true);
        }, selectedCell);
        present(selector, true);
    }, "OK"));

    setView(view);
}
