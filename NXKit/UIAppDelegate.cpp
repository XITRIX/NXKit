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
#include "TableView2TestController.hpp"
#include "ControlTestViewController.hpp"
//#include "TextTestController.hpp"

void UIAppDelegate::applicationDidFinishLaunching() {
//    auto textVC = NXKit::make_shared<TextTestController>();
//
//    auto window = NXKit::make_shared<UIWindow>();
//    window->setRootViewController(textVC);
//    window->backgroundColor = UIColor::blue;
//    window->makeKeyAndVisible();

    // Test VC
    auto rootVC = NXKit::make_shared<MyViewController>();
    auto listVC = NXKit::make_shared<ListViewController>();
    auto touchVC = NXKit::make_shared<TouchTestViewController>();
    auto tableView = NXKit::make_shared<TableViewTestController>();
    auto tableView2 = NXKit::make_shared<TableView2TestController>();
    auto controlVC = NXKit::make_shared<ControlTestViewController>();
    auto animVC = NXKit::make_shared<AnimationTestViewController>();

    printf("Launching app...\n");

    auto window = NXKit::make_shared<UIWindow>();
    std::vector<std::shared_ptr<UIViewController>> controllers = {controlVC, tableView2, listVC, animVC, nullptr, rootVC, touchVC};
//    std::vector<std::shared_ptr<UIViewController>> controllers = {animVC, controlVC, listVC};
    auto tabsVC = NXKit::make_shared<UITabBarController>(controllers);
    auto vc = NXKit::make_shared<UINavigationController>(tabsVC);
    tabsVC->setTitle("Demo App");
    tabsVC->setImage(NXKit::make_shared<UIImage>("Images/logo.png"_res));
    window->setRootViewController(vc);
    window->makeKeyAndVisible();

    // Focus test
    auto focus = window->getDefaultFocus();
    Application::shared()->setFocus(focus);
}
