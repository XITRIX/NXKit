#pragma once

#include <UIViewController.h>

class ControllerTestViewController final : public NXKit::UIViewController {
public:
    ControllerTestViewController();

    void loadView() override;
    void viewSafeAreaInsetsDidChange() override;
    void viewDidLayoutSubviews() override;

private:
    void updateSafeAreaLayout();
};
