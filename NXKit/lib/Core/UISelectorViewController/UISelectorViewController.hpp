//
//  UISelectorViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 14.09.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>

namespace NXKit {

class UISelectorViewController: public UIViewController {
public:
    UISelectorViewController();

    void loadView() override;
    void viewWillAppear(bool animated) override;
    void viewDidLayoutSubviews() override;

protected:
    void makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion = [](){}) override;
    void makeViewDisappear(bool animated, std::function<void(bool)> completion) override;

private:
    float headerHeight = 70;
    float footerHeight = 73;
    
    UIView* scrollView = nullptr;
    UIView* contentView = nullptr;
    UIView* containerView = nullptr;

    UIView* makeContentView();
    UIView* makeFooter();
};

}
