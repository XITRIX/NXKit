//
//  UIWindow.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include "UIViewController.hpp"

class UIWindow: public UIView {
public:
    UIViewController* getRootViewController();
    void setRootViewController(UIViewController* viewController);

    void makeKeyAndVisible();

    void layoutSubviews() override;
private:
    UIViewController* rootViewController = nullptr;
};
