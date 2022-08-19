//
//  UIAppDelegate.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include "NXKit.hpp"
#include "ViewController.hpp"
#include "TouchTestViewController.hpp"

void UIAppDelegate::applicationDidFinishLaunching() {
    // Test VC
    MyViewController* rootVC = new MyViewController();

    UIWindow* window = new UIWindow();
    UITabBarController* tabsVC = new UITabBarController(rootVC);
    UINavigationController* vc = new UINavigationController(tabsVC);
    window->setRootViewController(vc);
    window->makeKeyAndVisible();

    // Focus test
    auto focus = window->getDefaultFocus();
    Application::shared()->setFocus(focus);
}
