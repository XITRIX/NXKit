//
//  UIAppDelegate.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include "NXKit.hpp"

void UIAppDelegate::applicationDidFinishLaunching() {
    //    RootView* root = new RootView();
        UIWindow* window = new UIWindow();
    //    ViewController* rootVC = new ViewController();
        UITabBarController* tabsVC = new UITabBarController();
        UINavigationController* vc = new UINavigationController(tabsVC);
        window->setRootViewController(vc);
        window->makeKeyAndVisible();
}
