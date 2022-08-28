//
//  UIAppDelegate.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 17.07.2022.
//

#include "NXKit.hpp"
#include "ViewController.hpp"
#include "TouchTestViewController.hpp"
#include "AnimationTestViewController.hpp"
#include "ListViewController.hpp"

void UIAppDelegate::applicationDidFinishLaunching() {
    // Test VC
    MyViewController* rootVC = new MyViewController();
    ListViewController* listVC = new ListViewController();
    TouchTestViewController* touchVC = new TouchTestViewController();
    AnimationTestViewController* animVC = new AnimationTestViewController();

    printf("Launching app...\n");

    UIWindow* window = new UIWindow();
    UITabBarController* tabsVC = new UITabBarController({listVC, animVC, nullptr, rootVC, touchVC});
    UINavigationController* vc = new UINavigationController(tabsVC);
    window->setRootViewController(vc);
    window->makeKeyAndVisible();

    // Focus test
    auto focus = window->getDefaultFocus();
    Application::shared()->setFocus(focus);
}
