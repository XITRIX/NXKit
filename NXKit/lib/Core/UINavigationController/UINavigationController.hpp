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
    std::shared_ptr<UILabel> titleLabel;
    std::shared_ptr<UIImageView> imageView;
};

class UINavigationController: public UIViewController {
public:
    UINavigationController(std::shared_ptr<UIViewController> rootController);
    virtual ~UINavigationController();
    
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;

    void show(std::shared_ptr<UIViewController> controller, void* sender = nullptr) override;
    void childNavigationItemDidChange(std::shared_ptr<UIViewController> controller) override;

    std::vector<std::shared_ptr<UIViewController>> getViewControllers() { return viewControllers; }

    void pushViewController(std::shared_ptr<UIViewController> otherViewController, bool animated);
    std::shared_ptr<UIViewController> popViewController(bool animated, bool free = true);
private:
    float headerHeight = 88;
    float footerHeight = 73;
    bool isTranslucent = false;
    
    std::vector<std::shared_ptr<UIViewController>> viewControllers;
    
    std::shared_ptr<UIView> rootView;
    std::shared_ptr<UIStackView> overlay;

    std::shared_ptr<UINavigationBar> navigationBar = nullptr;

    std::shared_ptr<UIView> buildFooter();
};

}
