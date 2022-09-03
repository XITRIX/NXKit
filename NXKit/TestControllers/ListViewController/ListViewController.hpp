//
//  ListViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#pragma once

#include <NXKit.hpp>

class ListItemView: public UIControl  {
public:
    ListItemView();

    void setText(std::string text);
    void setImage(UIImage* image);

    bool canBecomeFocused() override;
private:
    UILabel* label;
    UIImageView* imageView;
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
