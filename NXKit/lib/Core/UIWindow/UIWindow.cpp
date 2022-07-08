//
//  UIWindow.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIWindow.hpp"
#include "Application.hpp"

UIViewController* UIWindow::getRootViewController() {
    return rootViewController;
}

void UIWindow::setRootViewController(UIViewController *viewController) {
    rootViewController = viewController;
    if (rootViewController)
        rootViewController->next = this;
}

void UIWindow::makeKeyAndVisible() {
    Application::shared()->setKeyWindow(this);

    if (rootViewController) {
        rootViewController->loadViewIfNeeded();
        rootViewController->next = this;
        rootViewController->viewWillAppear(false);
        addSubview(rootViewController->getView());
        rootViewController->viewDidAppear(false);
    }
}

void UIWindow::layoutSubviews() {
    UIView::layoutSubviews();
    rootViewController->getView()->setSize(this->frame.size);
}
