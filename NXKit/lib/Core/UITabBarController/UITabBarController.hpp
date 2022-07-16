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

class UITabBarItem: public UIStackView {
public:
    UITabBarItem();

    void setTitle(std::string text);

    bool isSelected();
    void setSelected(bool selected);
private:
    UILabel* label = nullptr;
    UIView* selectionBar = nullptr;


    bool selected = false;
};

class UITabBarController: public UIViewController {
public:
//    UITabBarController();
    void loadView() override;
    void viewDidLoad() override;
private:
    UIStackView* tabs;
};
