//
//  UITabBarController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIControl/UIControl.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

class UITabBarController;

class UITabBarSeparatorView: public UIStackView {
public:
    UITabBarSeparatorView();
};

class UITabBarItemView: public UIControl {
public:
    UITabBarItemView(std::shared_ptr<UITabBarController> parent, std::shared_ptr<UIViewController> controller);
    virtual ~UITabBarItemView();

    void setTitle(std::string text);
    void setSelected(bool selected);

    void becomeFocused() override;
    bool isEnabled() override;
private:
    friend class UITabBarController;
    std::shared_ptr<UILabel> label;
    std::shared_ptr<UIView> selectionBar;
    std::shared_ptr<UITabBarController> parent;
    std::shared_ptr<UIViewController> controller;

//    bool selected = false;
};

class UITabBarController: public UIViewController {
public:
    UITabBarController();
    UITabBarController(std::vector<std::shared_ptr<UIViewController>> controllers);
    virtual ~UITabBarController();

    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;
    void childNavigationItemDidChange(std::shared_ptr<UIViewController> controller) override;

    std::vector<std::shared_ptr<UIViewController>> getViewControllers() { return viewControllers; }
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers);

    int getSelectedIndex() { return selectedIndex; }
    void setSelectedIndex(int index);
private:
    friend class UITabBarItemView;

    std::vector<std::shared_ptr<UIViewController>> viewControllers;
    int selectedIndex = -1;

    std::shared_ptr<UIStackView> tabs = nullptr;
    std::shared_ptr<UIView> contentView = nullptr;
    std::shared_ptr<UIViewController> content = nullptr;
    std::vector<std::shared_ptr<UITabBarItemView>> tabViews;

    void setSelected(std::shared_ptr<UITabBarItemView> item);
    void reloadViewForViewControllers();
};

}
