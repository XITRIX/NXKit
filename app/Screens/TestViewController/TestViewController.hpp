#pragma once

#include <UIKit.h>

class TestViewController: public NXKit::UIViewController {
public:
    TestViewController();
    void loadView() override;
    void viewDidLoad() override;

    void viewDidLayoutSubviews() override;
private:
    std::shared_ptr<NXKit::UILabel> label;
    std::shared_ptr<NXKit::UILabel> label2;
    std::shared_ptr<NXKit::UIView> bottomBar;
};
