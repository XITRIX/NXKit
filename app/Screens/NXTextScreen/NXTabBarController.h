//
// Created by Даниил Виноградов on 22.02.2025.
//


#pragma once
#include <UIKit.h>

using namespace NXKit;

class NXTabBarButton;
class NXTabBar: public UIScrollView {
public:
    NXTabBar();

    std::optional<UIAction> selectionDidChange;
    void setItems(const std::vector<std::vector<std::string>>& items);

    IndexPath selected() { return _selected; }
private:
    IndexPath _selected;
    std::shared_ptr<UIView> _container;
    std::vector<std::vector<std::shared_ptr<NXTabBarButton>>> _buttons;
};

class NXTabBarController: public UIViewController {
public:
//    NXTabBarController(std::shared_ptr<UIViewController> controller);
    NXTabBarController(const std::vector<std::shared_ptr<UIViewController>>& controllers);
    NXTabBarController(const std::vector<std::vector<std::shared_ptr<UIViewController>>>& controllers);

    void loadView() override;
    void viewDidLoad() override;
private:
    std::vector<std::vector<std::shared_ptr<UIViewController>>> _viewControllers;
    std::shared_ptr<UIViewController> _presentedViewController;
    IndexPath _selected;
    std::shared_ptr<NXTabBar> _tabBar;

    void updateTabSelection();
};
