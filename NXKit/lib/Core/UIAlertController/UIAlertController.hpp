//
//  UIAlertController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 04.09.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/UIStackView/UIStackView.hpp>

namespace NXKit {

class UIAlertController: public UIViewController {
public:
    void loadView() override;
    void viewDidLoad() override;

protected:
    UIStackView* alert = nullptr;

    void makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion = [](){}) override;
    void makeViewDisappear(bool animated, std::function<void(bool)> completion) override;
};

}
