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
    auto view = std::make_shared<UIStackView>();
    view->setAlignItems(AlignItems::CENTER);
    view->setJustifyContent(JustifyContent::CENTER);

    auto control = std::make_shared<UIButton>("Present example");
    control->setSize({ 280, 60 });
    view->addSubview(control);

    auto control2 = std::make_shared<UIButton>("Show example");
    control2->setSize({ 280, 60 });
    control2->setMarginTop(9);
    view->addSubview(control2);

    auto control3 = std::make_shared<UIButton>("Alert example");
    control3->setSize({ 280, 60 });
    control3->setMarginTop(9);
    view->addSubview(control3);

    auto control4 = std::make_shared<UIButton>("Bottom sheet example");
    control4->setSize({ 280, 60 });
    control4->setMarginTop(9);
    view->addSubview(control4);

    auto control5 = std::make_shared<UIButton>("Selector example");
    control5->setSize({ 280, 60 });
    control5->setMarginTop(9);
    view->addSubview(control5);

    control->addAction(BUTTON_A, UIAction([this]() {
        auto touchVC = std::make_shared<TouchTestViewController>();
        auto animVC = std::make_shared<AnimationTestViewController>();

        std::vector<std::shared_ptr<UIViewController>> controllers = {animVC, nullptr, touchVC };
        auto tabsVC = std::make_shared<UITabBarController>(controllers);
        auto vc = std::make_shared<UINavigationController>(tabsVC);
        tabsVC->setTitle("Presentation test");

        present(vc, true);
    }, "OK"));

    control2->addAction(BUTTON_A, UIAction([this]() {
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//        TouchTestViewController* touchVC = new TouchTestViewController();
        auto listVC = std::make_shared<ListViewController>();
        show(listVC);
    }, "OK"));

    control3->addAction(BUTTON_A, UIAction([this]() {
        auto alert = std::make_shared<UIAlertController>("Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title Hello title", "Just some text to fill message label Just some text to fill message label Just some text to fill message label Just some text to fill message label");
        alert->addAction(UIAlertAction("Cancel", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("Cancel\n"); }));
        alert->addAction(UIAlertAction("Ok", NXKit::UIAlertActionStyle::DEFAULT, []() { printf("OK\n"); }));
        alert->addAction(UIAlertAction("Delete", NXKit::UIAlertActionStyle::DESTRUCTIVE, []() { printf("Delete\n"); }));
        present(alert, true);
    }, "OK"));

    control4->addAction(BUTTON_A, UIAction([this]() {
        auto listVC = std::make_shared<ListViewController>();

        auto nvc = std::make_shared<UINavigationController>(listVC);
        auto vc = std::make_shared<UIBottomSheetController>(nvc);
//        tabsVC->setTitle("Presentation test");

        present(vc, true);
    }, "OK"));

    static int selectedCell = -1;
    control5->addAction(BUTTON_A, UIAction([this]() {
//        AnimationTestViewController* animVC = new AnimationTestViewController();
//        TouchTestViewController* touchVC = new TouchTestViewController();
        std::vector<std::string> data;
        for (int i = 0; i < 15; i++)
            data.push_back("Cell #" + std::to_string(i + 1));

        auto selector = std::make_shared<UISelectorViewController>("Demo select", data, [this](int index) {
            selectedCell = index;
            auto alert = std::make_shared<UIAlertController>("Select successful", "You selected cell #" + std::to_string(index));
            alert->addAction(UIAlertAction("Oke"));
            present(alert, true);
        }, selectedCell);
        present(selector, true);
    }, "OK"));

    setView(view);
}
