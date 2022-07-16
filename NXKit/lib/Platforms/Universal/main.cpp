//
//  main2..cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include "NXKit.hpp"
#include "UINavigationController.hpp"
#include "UITabBarController.hpp"
#include "ViewController.hpp"
#include "glfw_video.hpp"

constexpr uint32_t ORIGINAL_WINDOW_WIDTH  = 1280;
constexpr uint32_t ORIGINAL_WINDOW_HEIGHT = 720;

#ifdef __SWITCH__
#include <switch.h>
#endif

int main(int argc, const char * argv[]) {
    Application* application = new Application();
    auto videoContext = new GLFWVideoContext("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
    application->setVideoContext(videoContext);

//    RootView* root = new RootView();
    UIWindow* window = new UIWindow();
//    ViewController* rootVC = new ViewController();
    UITabBarController* tabsVC = new UITabBarController();
    UINavigationController* vc = new UINavigationController(tabsVC);
    window->setRootViewController(vc);
    window->makeKeyAndVisible();

    while (application->mainLoop());

    delete application;
    return 0;
}
