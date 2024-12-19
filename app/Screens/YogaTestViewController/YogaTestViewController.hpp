#pragma once

#include <UIKit.h>

class YogaTestViewController: public NXKit::UIViewController {
public:
    YogaTestViewController();
    void loadView() override;
//    void viewDidLoad() override;

    void viewDidLayoutSubviews() override;
private:
    std::shared_ptr<NXKit::UILabel> dragMeViewLabel;
};

