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
#include "TableViewTestController.hpp"
#include "ControlTestViewController.hpp"
//#include "TextTestController.hpp"

void UIAppDelegate::applicationDidFinishLaunching() {
//    auto textVC = std::make_shared<TextTestController>();
//
//    auto window = std::make_shared<UIWindow>();
//    window->setRootViewController(textVC);
//    window->makeKeyAndVisible();

    // Test VC
    auto rootVC = std::make_shared<MyViewController>();
    auto listVC = std::make_shared<ListViewController>();
    auto touchVC = std::make_shared<TouchTestViewController>();
    auto tableView = std::make_shared<TableViewTestController>();
    auto controlVC = std::make_shared<ControlTestViewController>();
    auto animVC = std::make_shared<AnimationTestViewController>();

    printf("Launching app...\n");

    auto window = make_shared<UIWindow>();
    std::vector<std::shared_ptr<UIViewController>> controllers = {controlVC, tableView, listVC, animVC, nullptr, rootVC, touchVC};
    auto tabsVC = std::make_shared<UITabBarController>(controllers);
//    auto vc = std::make_shared<UINavigationController>(tabsVC);
    tabsVC->setTitle("Demo App");
    tabsVC->setImage(std::make_shared<UIImage>("Images/logo.png"_res));
    window->setRootViewController(tabsVC);
    window->makeKeyAndVisible();

    // Focus test
    auto focus = window->getDefaultFocus();
    Application::shared()->setFocus(focus);
}
