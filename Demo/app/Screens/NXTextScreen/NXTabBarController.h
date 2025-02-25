//
// Created by Даниил Виноградов on 22.02.2025.
//


#pragma once
#include <NXKit.h>

namespace NXKit {

class NXTabBarButton: public NXControl {
public:
    NXTabBarButton();

    void setSelected(bool selected) override;

    std::string text() { return _titleLabel->text(); }
    void setText(const std::string& text);

    std::shared_ptr<UILabel> titleLabel() { return _titleLabel; }

private:
    std::shared_ptr<UILabel> _titleLabel;
    std::shared_ptr<UIView> _rectView;
};

class NXTabBar : public UIScrollView {
public:
    NXTabBar();

    std::optional<UIAction> selectionDidChange;

    void setItems(const std::vector<std::vector<std::string>> &items);

    IndexPath selected() { return _selected; }

private:
    IndexPath _selected;
    std::shared_ptr<UIView> _container;
    std::vector<std::vector<std::shared_ptr<NXTabBarButton>>> _buttons;
};

class NXTabBarController : public UIViewController {
public:
//    NXTabBarController(std::shared_ptr<UIViewController> controller);
    explicit NXTabBarController(const std::vector<std::shared_ptr<UIViewController>> &controllers);

    explicit NXTabBarController(const std::vector<std::vector<std::shared_ptr<UIViewController>>> &controllers);

    void loadView() override;

    void viewDidLoad() override;

private:
    std::vector<std::vector<std::shared_ptr<UIViewController>>> _viewControllers;
    std::shared_ptr<UIViewController> _presentedViewController;
    IndexPath _selected;
    std::shared_ptr<NXTabBar> _tabBar;

    void updateTabSelection();
};

}