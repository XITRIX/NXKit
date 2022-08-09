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
    //    RootView* root = new RootView();
    UIWindow* window = new UIWindow();
    MyViewController* rootVC = new MyViewController();
    UITabBarController* tabsVC = new UITabBarController(rootVC);
    UINavigationController* vc = new UINavigationController(tabsVC);
    window->setRootViewController(vc);
    window->makeKeyAndVisible();


//    UIWindow* window = new UIWindow();
//    auto vc = new TouchTestViewController();
//    window->setRootViewController(vc);
//    window->makeKeyAndVisible();

    auto focus = window->getDefaultFocus();
    Application::shared()->setFocus(focus);
}
