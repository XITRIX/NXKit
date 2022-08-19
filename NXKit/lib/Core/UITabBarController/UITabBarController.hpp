//
//  UITabBarController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include "UIViewController.hpp"
#include "UIStackView.hpp"
#include "UILabel.hpp"

namespace NXKit {

class UITabBarItem: public UIStackView {
public:
    UITabBarItem();

    void setTitle(std::string text);

    bool isSelected();
    void setSelected(bool selected);

    void becomeFocused() override;
    void resignFocused() override;
private:
    UILabel* label = nullptr;
    UIView* selectionBar = nullptr;

    bool selected = false;
};

class UITabBarController: public UIViewController {
public:
    UITabBarController(UIViewController* content);
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;
private:
    UIStackView* tabs = nullptr;
    UIView* contentView = nullptr;
    UIViewController* content = nullptr;
};

}
