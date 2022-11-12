//
//  UISelectorViewController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 14.09.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/UIScrollView/UIScrollView.hpp>

#include <vector>
#include <optional>

namespace NXKit {

class UISelectorViewController: public UIViewController {
public:
    UISelectorViewController(std::string title, std::vector<std::string> data, std::function<void(int)> onComplete, int selectedIndex = -1);

    void loadView() override;
    void viewDidLayoutSubviews() override;

protected:
    void makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion = [](){}) override;
    void makeViewDisappear(bool animated, std::function<void(bool)> completion) override;

private:
    float headerHeight = 70;
    float footerHeight = 65;
    
    std::shared_ptr<UIScrollView> scrollView = nullptr;
    std::shared_ptr<UIView> contentView = nullptr;
    std::shared_ptr<UIView> containerView = nullptr;
    
    std::shared_ptr<UIView> selectedView = nullptr;
    std::shared_ptr<UIView> acctionsBar = nullptr;

    std::string title;
    std::function<void(int)> onComplete;
    std::vector<std::string> data;
    int selectedIndex;

    std::shared_ptr<UIView> makeContentView();
    std::shared_ptr<UIView> makeFooter();
};

}
