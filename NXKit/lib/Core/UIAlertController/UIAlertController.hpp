//
//  UIAlertController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 04.09.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

enum class UIAlertActionStyle {
    DEFAULT,
    DESTRUCTIVE,
};

struct UIAlertAction {
    std::string title;
    UIAlertActionStyle style;
    std::function<void()> handler;

    UIAlertAction(std::string title, UIAlertActionStyle style = UIAlertActionStyle::DEFAULT, std::function<void()> handler = [](){});
    UIAlertAction(): UIAlertAction("", UIAlertActionStyle::DEFAULT) {}
};

class UIButton;
class UIAlertController: public UIViewController {
public:
    UIAlertController(std::string title, std::string message);

    void loadView() override;
    void viewDidLoad() override;

    void addAction(UIAlertAction action);

protected:
    std::string title;
    std::string message;

    std::vector<UIAlertAction> actions;

    std::shared_ptr<UILabel> titleLabel = nullptr;
    std::shared_ptr<UILabel> messageLabel = nullptr;
    std::shared_ptr<UIButton> buttons[3];

    std::shared_ptr<UIStackView> alert = nullptr;
    std::shared_ptr<UIStackView> buttonsView = nullptr;

    void makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion = [](){}) override;
    void makeViewDisappear(bool animated, std::function<void(bool)> completion) override;
};

}
