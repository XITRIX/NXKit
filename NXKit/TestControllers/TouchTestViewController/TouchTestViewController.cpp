//
//  TouchTestViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 13.08.2022.
//

#include "TouchTestViewController.hpp"

UIView* target;

void TouchTestViewController::loadView() {
    auto view = new UIStackView(Axis::HORIZONTAL);

    auto red = new UIView();
    red->backgroundColor = UIColor::red;
    red->setGrow(1);
    red->tag = "RED";

    auto green = new UIStackView();
    green->backgroundColor = UIColor::green;
    green->setGrow(1);
    green->tag = "GREEN";
    green->setJustifyContent(JustifyContent::CENTER);
    green->setAlignItems(AlignItems::CENTER);

    target = new UIView();
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
            printf("TEST");
        }

        if (manager->getKeyDown(BRLS_KBD_KEY_SPACE)) {
            auto res = target->convert(Point(), nullptr);
            printf("Space pressed!");
        }
    });
}
