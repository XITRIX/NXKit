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

using namespace NXKit;

void makeApplication(VideoContext* context, const char* path) {
    auto application = new Application();
    application->setVideoContext(context);
    application->setResourcesPath(std::string(path) + "/");
    application->getDelegate()->applicationDidFinishLaunching();
}

void applicationLoop() {
    Application::shared()->mainLoop();
}