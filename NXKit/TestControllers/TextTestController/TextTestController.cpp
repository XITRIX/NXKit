//
//  TextTestController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 07.10.2022.
//

#include "TextTestController.hpp"

//TextTestController::TextTestController() {
//
//}

void TextTestController::loadView() {
    auto view = NXKit::make_shared<UIStackView>(Axis::VERTICAL);
//    view->setJustifyContent(JustifyContent::FLEX_START);
//    view->setAlignItems(AlignItems::FLEX_START);
    view->setJustifyContent(JustifyContent::CENTER);
    view->setAlignItems(AlignItems::CENTER);
    view->cornerRadius = 48;
    view->backgroundColor = UIColor::green;

    label = NXKit::make_shared<UILabel>("Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст ");
//    label->setMargins(UIEdgeInsets(124));
    label->horizontalAlign = HorizontalAlign::CENTER;
    label->backgroundColor = UIColor::orange;
    label->cornerRadius = 16;
//    label->setPosition(Point(40, 0));
    label->tag = "Test";

    view->addSubview(label);

    auto test = NXKit::make_shared<UIView>();
    test->setSize(Size(280, 280));
    test->backgroundColor = UIColor::orange;
    test->cornerRadius = 38;
    test->tag = "Test";
//    view->addSubview(test);

//    view->transformOrigin = Point(10, 20);
//    setView(label);
    setView(view);
}

float scale = 1;
void TextTestController::viewDidLoad() {
//    InputManager::shared()->getInputUpdated()->subscribe([this]() {
//        auto manager = InputManager::shared();
//
//        if (manager->getKey(BRLS_KBD_KEY_UP)) {
//            scale += 0.01f;
//        }
//        if (manager->getKey(BRLS_KBD_KEY_DOWN)) {
//            scale -= 0.01f;
//        }
//
//        label->transformSize = Size(scale, scale);
//    });
}
