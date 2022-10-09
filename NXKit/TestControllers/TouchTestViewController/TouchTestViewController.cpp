//
//  TouchTestViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 13.08.2022.
//

#include "TouchTestViewController.hpp"

std::shared_ptr<UIView> target;

TouchTestViewController::TouchTestViewController() {
    setTitle("Touch test");
}

void TargetView::layoutSubviews() {
    UIView::layoutSubviews();
}

void TouchTestViewController::viewWillLayoutSubviews() {
    UIViewController::viewWillLayoutSubviews();
}

void TouchTestViewController::loadView() {
    auto view = std::make_shared<UIStackView>(Axis::HORIZONTAL);

    auto red = std::make_shared<UIView>();
    red->backgroundColor = UIColor::red;
    red->setGrow(1);
    red->tag = "RED";

    auto green = std::make_shared<UIStackView>();
    green->backgroundColor = UIColor::green;
    green->setGrow(1);
    green->tag = "GREEN";
    green->setJustifyContent(JustifyContent::CENTER);
    green->setAlignItems(AlignItems::CENTER);

    target = std::make_shared<TargetView>();
    target->highlightOnFocus = false;
    target->tag = "TARGET";
    target->backgroundColor = UIColor::blue;
    target->setSize(Size(50, 50));

    green->addSubview(target);

    view->addSubview(red);
    view->addSubview(green);

    setView(view);
}

void TouchTestViewController::viewDidLoad() {
    InputManager* manager = InputManager::shared();
    manager->getInputUpdated()->subscribe([manager]() {
        if (manager->getMouseButtonDown(BRLS_MOUSE_LKB)) {
            auto pos = manager->getCoursorPosition();
//            printf("X: %f, Y: %f\n", pos.x, pos.y);
            auto view = Application::shared()->getKeyWindow()->hitTest(pos, nullptr);
            printf("View touched: %s\n", view->tag.c_str());
        }

        if (manager->getKeyDown(BRLS_KBD_KEY_SPACE)) {
//            auto res = target->convert(Point(), nullptr);
            printf("Space pressed!\n");
        }
    });
}
