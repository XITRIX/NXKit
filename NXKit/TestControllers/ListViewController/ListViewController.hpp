//
//  ListViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 27.08.2022.
//

#pragma once

#include <NXKit.hpp>

using namespace NXKit;

class ListItemView: public UIControl  {
public:
    ListItemView();

    void setText(std::string text);
    void setImage(std::shared_ptr<UIImage> image);

    bool canBecomeFocused() override;
private:
    std::shared_ptr<UILabel> label;
    std::shared_ptr<UIImageView> imageView;
};

class ListViewController: public UIViewController {
public:
    ListViewController();
    void loadView() override;
    void viewDidLoad() override;
private:
    std::shared_ptr<UIScrollView> scrollView;
    std::shared_ptr<UIStackView> stackView;
    
};
