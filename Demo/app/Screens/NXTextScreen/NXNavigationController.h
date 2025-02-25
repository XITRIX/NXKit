//
// Created by Даниил Виноградов on 22.02.2025.
//

#pragma once
#include <NXKit.h>

namespace NXKit {

class NXNavigationController : public UIViewController {
public:
    explicit NXNavigationController(const std::shared_ptr<UIViewController> &rootViewController);

    void loadView() override;

    void viewDidLoad() override;

    void viewDidLayoutSubviews() override;

    std::vector<std::shared_ptr<UIViewController>> viewControllers() const { return _viewControllers; }

    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> viewControllers, bool animated);

    std::shared_ptr<UIViewController> topViewController();

private:
    std::shared_ptr<UIView> _navigationBar;
    std::shared_ptr<UIView> _toolBar;
    std::shared_ptr<UIViewController> presentedViewController;
    std::vector<std::shared_ptr<UIViewController>> _viewControllers;

    void updatePresentedViewController(bool animated);
};

}
