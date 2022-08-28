//
//  ListViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#pragma once

#include <NXKit.hpp>

class ListItemView: public UIStackView {
public:
    ListItemView();

    void setText(std::string text);

    bool canBecomeFocused() override;
private:
    UILabel* label;
};

class ListViewController: public UIViewController {
public:
    ListViewController();
    void loadView() override;
    void viewDidLoad() override;
private:
    UIScrollView* scrollView;
    UIStackView* stackView;
    
};
