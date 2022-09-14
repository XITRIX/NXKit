//
//  UINavigationController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIImageView/UIImageView.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

class UINavigationBar: public UIStackView {
public:
    UINavigationBar();
    virtual ~UINavigationBar();

    void pushNavigationItem(UINavigationItem navigationItem);

//private:
    UILabel* titleLabel;
    UIImageView* imageView;
};

class UINavigationController: public UIViewController {
public:
    UINavigationController(UIViewController* rootController);
    virtual ~UINavigationController();
    
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;

    void show(UIViewController* controller, void* sender = nullptr) override;
    void childNavigationItemDidChange(UIViewController* controller) override;

    std::vector<UIViewController*> getViewControllers() { return viewControllers; }

    void pushViewController(UIViewController* otherViewController, bool animated);
    UIViewController* popViewController(bool animated, bool free = true);
private:
    float headerHeight = 88;
    float footerHeight = 73;
    bool isTranslucent = false;
    
    std::vector<UIViewController*> viewControllers;
    
    UIView* rootView;
    UIStackView* overlay;

    UINavigationBar* navigationBar = nullptr;

    UIView* buildFooter();
};

}
