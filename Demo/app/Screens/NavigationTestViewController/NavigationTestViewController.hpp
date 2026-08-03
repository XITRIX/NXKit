#pragma once

#include <UIViewController.h>

#include <memory>

namespace NXKit {
class UIView;
class UIScrollView;
}

class NavigationTestViewController final : public NXKit::UIViewController {
public:
    NavigationTestViewController();

    void loadView() override;
    void viewSafeAreaInsetsDidChange() override;
    void viewDidLayoutSubviews() override;

private:
    std::shared_ptr<NXKit::UIScrollView> _scrollView;
    std::shared_ptr<NXKit::UIView> _contentContainer;
};
