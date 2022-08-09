//
//  UINavigationController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#pragma once

#include "UIViewController.hpp"

namespace NXKit {

class UINavigationController: public UIViewController {
public:
    UINavigationController(UIViewController* rootController);
    void loadView() override;
    void viewDidLoad() override;
    void viewDidLayoutSubviews() override;
private:
    UIView* rootView;
    UIViewController* rootController;
};

}
