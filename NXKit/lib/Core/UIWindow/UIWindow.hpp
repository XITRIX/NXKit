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
    UIEdgeInsets additionalSafeAreaInsets;

    UIWindow();
    ~UIWindow();

    std::shared_ptr<UIViewController> getRootViewController();
    void setRootViewController(std::shared_ptr<UIViewController> viewController);

    void makeKeyAndVisible();
    void sendEvent(std::shared_ptr<UIEvent> event);

    void layoutSubviews() override;
    std::shared_ptr<UIView> getDefaultFocus() override;
    UIEdgeInsets safeAreaInsets() override;

    UITraitCollection getTraitCollection() override;

private:
    friend class UIViewController;
    std::shared_ptr<UIViewController> rootViewController = nullptr;
    std::vector<std::shared_ptr<UIViewController>> presentedViewControllers;

    void addPresentedViewController(std::shared_ptr<UIViewController> controller);
    void removePresentedViewController(std::shared_ptr<UIViewController> controller);
};

}
