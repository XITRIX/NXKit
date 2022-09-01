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
    UITabBarItemView(UITabBarController* parent, UIViewController* controller);
    virtual ~UITabBarItemView();

    void setTitle(std::string text);

    bool isSelected();
    void setSelected(bool selected);

    void becomeFocused() override;
    bool canBecomeFocused() override;
private:
    friend class UITabBarController;
    UILabel* label = nullptr;
    UIView* selectionBar = nullptr;
    UITabBarController* parent = nullptr;
    UIViewController* controller = nullptr;

    bool selected = false;
};

class UITabBarController: public UIViewController {
public:
    UITabBarController();
    UITabBarController(std::vector<UIViewController*> controllers);
    ~UITabBarController();

    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;

    std::vector<UIViewController*> getViewControllers() { return viewControllers; }
    void setViewControllers(std::vector<UIViewController*> controllers);

    int getSelectedIndex() { return selectedIndex; }
    void setSelectedIndex(int index);
private:
    friend class UITabBarItemView;

    std::vector<UIViewController*> viewControllers;
    int selectedIndex = -1;

    UIStackView* tabs = nullptr;
    UIView* contentView = nullptr;
    UIViewController* content = nullptr;
    std::vector<UITabBarItemView*> tabViews;

    void setSelected(UITabBarItemView* item);
    void reloadViewForViewControllers();
};

}
