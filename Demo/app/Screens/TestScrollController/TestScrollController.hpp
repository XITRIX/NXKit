#pragma once

#include <UIKit.h>

class TestScrollController: public NXKit::UIViewController {
public:
    TestScrollController();
    void loadView() override;
//    void viewDidLoad() override;

    void viewSafeAreaInsetsDidChange() override;
private:
std::shared_ptr<NXKit::UIScrollView> scrollView;
    std::shared_ptr<NXKit::UILabel> label;
    std::shared_ptr<NXKit::UIView> labelContainer;
};
