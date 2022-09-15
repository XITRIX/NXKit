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

UIAlertAction::UIAlertAction(std::string title, UIAlertActionStyle style, std::function<void()> handler):
    title(title), style(style), handler(handler)
{ }

class UIAlertButton: public UIButton {
public:
    UIAlertButton(): UIButton() {
        setHeight(72);
        label->getFont()->fontSize = 24;
        UIButton::setStyle(UIButtonStyle::BORDERLESS);
    }

    void setStyle(UIAlertActionStyle style) {
        switch (style) {
            case UIAlertActionStyle::DEFAULT:
                label->textColor = label->getTintColor();
                break;
            case UIAlertActionStyle::DESTRUCTIVE:
                label->textColor = UIColor::red;
                break;
        }
    }
};

UIAlertController::UIAlertController(std::string title, std::string message):
    title(title), message(message)
{ }

void UIAlertController::loadView() {
    auto root = new UIStackView();
    root->backgroundColor = UIColor::black.withAlphaComponent(0.6f);
    root->setAlignItems(AlignItems::CENTER);
    root->setJustifyContent(JustifyContent::CENTER);

    alert = new UIStackView(Axis::VERTICAL);
    alert->cornerRadius = 4;
    alert->backgroundColor = UIColor::systemBackground;
    alert->setPercentWidth(60);
    alert->setAlignItems(AlignItems::STRETCH);
    alert->setJustifyContent(JustifyContent::CENTER);
    root->addSubview(alert);

    auto textsView = new UIStackView(Axis::VERTICAL);
    textsView->setPadding(45, 115, 45, 115);

    if (title != "") {
        titleLabel = new UILabel(title);
        titleLabel->horizontalAlign = HorizontalAlign::CENTER;
        titleLabel->getFont()->fontSize = 24;
        titleLabel->setMargins(20, 20, 20, 20);
        textsView->addSubview(titleLabel);
    }

    if (message != "") {
        messageLabel = new UILabel(message);
        messageLabel->horizontalAlign = HorizontalAlign::CENTER;
        messageLabel->getFont()->fontSize = 18;
        titleLabel->setMargins(20, 20, 20, 20);
        messageLabel->textColor = UIColor::gray;
        textsView->addSubview(messageLabel);
    }

    buttonsView = new UIStackView(Axis::HORIZONTAL);
    buttonsView->borderColor = UIColor::separator;
    buttonsView->setBorderTop(2);
    buttonsView->setHidden(true);
//    buttonsView->setBorderBottom(2);

    buttons[0] = new UIAlertButton();
    buttons[0]->setHidden(true);
    buttons[0]->setGrow(1);
    buttons[0]->setWidth(0);
    buttons[0]->addAction(BUTTON_A, UIAction([this]() {
        dismiss(true, actions[0].handler);
    }, "OK"));

    buttons[1] = new UIAlertButton();
    buttons[1]->setHidden(true);
    buttons[1]->setGrow(1);
    buttons[1]->setWidth(0);
    buttons[1]->setBorderLeft(2);
    buttons[1]->borderColor = UIColor::separator;
    buttons[1]->addAction(BUTTON_A, UIAction([this]() {
        dismiss(true, actions[1].handler);
    }, "OK"));

    buttons[2] = new UIAlertButton();
    buttons[2]->setHidden(true);
    buttons[2]->borderColor = UIColor::separator;
    buttons[2]->addAction(BUTTON_A, UIAction([this]() {
        dismiss(true, actions[2].handler);
    }, "OK"));

    buttonsView->addSubview(buttons[0]);
    buttonsView->addSubview(buttons[1]);

    alert->addSubview(textsView);
    alert->addSubview(buttonsView);
    alert->addSubview(buttons[2]);

    setView(root);
}

void UIAlertController::viewDidLoad() {
    UIViewController::addAction(BUTTON_B, UIAction([this]() {
        dismiss(true);
    }, "Back"));

    if (actions.size() > 0) {
        buttonsView->setHidden(false);
        buttons[0]->setHidden(false);
        buttons[0]->setTitle(actions[0].title);
        ((UIAlertButton*) buttons[0])->setStyle(actions[0].style);
    }
    if (actions.size() > 1) {
        buttons[1]->setHidden(false);
        buttons[1]->setTitle(actions[1].title);
        ((UIAlertButton*) buttons[1])->setStyle(actions[1].style);
    }
    if (actions.size() > 1) {
        buttonsView->setBorderBottom(2);
        buttons[2]->setHidden(false);
        buttons[2]->setTitle(actions[2].title);
        ((UIAlertButton*) buttons[2])->setStyle(actions[2].style);
    }
}

void UIAlertController::addAction(UIAlertAction action) {
    actions.push_back(action);
}

void UIAlertController::makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion) {
    getView()->alpha = 0;
    alert->transformSize = { 0.98f, 0.98f };

    UIView::animate({ getView(), alert }, 0.15f, [this]() {
        getView()->alpha = 1;
        alert->transformSize = { 1, 1 };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion();
    });
}

void UIAlertController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    UIView::animate({ getView(), alert}, 0.15f, [this]() {
        getView()->alpha = 0;
        alert->transformSize = { 0.98f, 0.98f };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion(true);
    });
}

}
