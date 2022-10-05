//
//  IOSMain.m
//  NXKit-iOS2
//
//  Created by Даниил Виноградов on 05.10.2022.
//

#import "IOSMain.hpp"
#import <Foundation/Foundation.h>
#include <string>

using namespace NXKit;

IOSMain::IOSMain(GLViewController* controller): controller(controller)
{
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    [EAGLContext setCurrentContext: context];

    ((GLKView *)controller.view).context = context;

    application = new NXKit::Application();
    application->setResourcesPath(std::string([[NSBundle mainBundle].resourcePath UTF8String]) + "/resource/");
    
    videoContext = new NXKit::IOSVideoContext(controller);

    application->setVideoContext(videoContext);

    inputManager = new IOSInputManager();
    InputManager::initWith(inputManager);

    application->getDelegate()->applicationDidFinishLaunching();
}

IOSMain::~IOSMain() {
    delete application;
}

void IOSMain::applicationLoop() {
    application->mainLoop();
}
