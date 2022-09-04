//
//  UIWindow.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#pragma once

#include <Core/UIViewController/UIViewController.hpp>

namespace NXKit {

class UIWindow: public UIView {
public:
    ~UIWindow();

    UIViewController* getRootViewController();
    void setRootViewController(UIViewController* viewController);

    void makeKeyAndVisible();
    void sendEvent(UIEvent* event);

    void layoutSubviews() override;
    UIView* getDefaultFocus() override;

private:
    friend class UIViewController;
    UIViewController* rootViewController = nullptr;
    std::vector<UIViewController*> presentedViewControllers;

    void addPresentedViewController(UIViewController* controller);
    void removePresentedViewController(UIViewController* controller);
};

}
