//
//  UIAlertController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 04.09.2022.
//

#include <Core/UIAlertController/UIAlertController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIButton/UIButton.hpp>

namespace NXKit {

void UIAlertController::loadView() {
    auto root = new UIStackView();
    root->backgroundColor = UIColor::black.withAlphaComponent(0.6f);
    root->setAlignItems(AlignItems::CENTER);
    root->setJustifyContent(JustifyContent::CENTER);

    alert = new UIStackView();
    alert->cornerRadius = 4;
    alert->backgroundColor = UIColor::systemBackground;
    alert->setSize({ 700, 300 });
    alert->setAlignItems(AlignItems::CENTER);
    alert->setJustifyContent(JustifyContent::CENTER);
    root->addSubview(alert);

    auto button = new UIButton("Close");
    alert->addSubview(button);
    button->addAction(BUTTON_A, UIAction([this]() {
        dismiss(true);
    }));

    setView(root);
}

void UIAlertController::viewDidLoad() {
    addAction(BUTTON_B, UIAction([this]() {
        dismiss(true);
    }));
}

void UIAlertController::makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion) {
    getView()->alpha = 0;
    getView()->animate(0.1f, [this]() {
        getView()->alpha = 1;
    }, EasingFunction::quadraticOut);

    alert->transformSize = { 0.95f, 0.95f };
    alert->animate(0.15f, [this]() {
        alert->transformSize = { 1, 1 };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion();
    });
}

void UIAlertController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    getView()->animate(0.1f, [this]() {
        getView()->alpha = 0;
    }, EasingFunction::quadraticOut);

    alert->transformSize = { 1, 1 };
    alert->animate(0.15f, [this]() {
        alert->transformSize = { 0.95f, 0.95f };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion(true);
    });
}

}
