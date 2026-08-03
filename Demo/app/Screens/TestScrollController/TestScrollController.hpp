#pragma once

#include <UIKit.h>

class TestScrollController: public NXKit::UIViewController {
public:
    TestScrollController();
    void loadView() override;

private:
    std::shared_ptr<NXKit::UITextView> textView;
};
