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
    auto view = new UIStackView(Axis::VERTICAL);
    view->setJustifyContent(JustifyContent::CENTER);
    view->setAlignItems(AlignItems::CENTER);

    label = new UILabel("Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст Просто текст ");
    label->setMargins(UIEdgeInsets(216));
    label->horizontalAlign = HorizontalAlign::CENTER;
    label->backgroundColor = UIColor::red;
    label->cornerRadius = 16;

    view->addSubview(label);

    setView(view);
}

float scale = 1;
void TextTestController::viewDidLoad() {
    InputManager::shared()->getInputUpdated()->subscribe([this]() {
        auto manager = InputManager::shared();

        if (manager->getKey(BRLS_KBD_KEY_UP)) {
            scale += 0.01f;
        }
        if (manager->getKey(BRLS_KBD_KEY_DOWN)) {
            scale -= 0.01f;
        }

        label->transformSize = Size(scale, scale);
    });
}
