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
}

void UIWindow::makeKeyAndVisible() {
    Application::shared()->setKeyWindow(this);

    if (rootViewController) {
        rootViewController->loadViewIfNeeded();
        rootViewController->viewWillAppear(false);
        addSubview(rootViewController->getView());
        layoutSubviews();
        rootViewController->viewDidAppear(false);
    }
}

void UIWindow::sendEvent(UIEvent* event) {
    auto allTouches = event->allTouches;
    if (allTouches.size() < 1) { return; }
    auto currentTouch = allTouches[0];

    UIView* hitView = nullptr;
    if (currentTouch->view) hitView = currentTouch->view;
    else hitView = hitTest(currentTouch->locationIn(nullptr), nullptr);
    if (hitView == nullptr) return;

    switch (currentTouch->phase) {
        default:
            break;
    }
}

void UIWindow::layoutSubviews() {
    UIView::layoutSubviews();
    rootViewController->getView()->setSize(((Rect) this->frame).size);
}
