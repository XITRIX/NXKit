//
//  nx_application.c
//  NXKit
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#import "nx_application.h"
#include "Application.hpp"

#include "UINavigationController.hpp"
#include "UITabBarController.hpp"

#import "video.hpp"

void makeApplication(VideoContext* context, const char* path) {
    auto application = new Application();
    application->setVideoContext(context);
    application->setResourcesPath(std::string(path) + "/");

    UIWindow* window = new UIWindow();
//    ViewController* rootVC = new ViewController();
    UITabBarController* tabsVC = new UITabBarController();
    UINavigationController* vc = new UINavigationController(tabsVC);
    window->setRootViewController(vc);
    window->makeKeyAndVisible();
}

void applicationLoop() {
    Application::shared()->mainLoop();
}
