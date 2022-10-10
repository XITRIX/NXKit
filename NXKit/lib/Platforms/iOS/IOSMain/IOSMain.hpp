//
//  IOSMain.h
//  NXKit-iOS2
//
//  Created by Даниил Виноградов on 05.10.2022.
//

#pragma once

#import "GLViewController.h"
#import "IOSVideoContext.hpp"
#import "IOSInputManager.hpp"
#import <GLKit/GLKit.h>
#import "NXKit.hpp"

//class GLViewController;

class IOSMain {
public:
    IOSMain(GLViewController* controller);
    ~IOSMain();

    void applicationLoop();
    
    std::shared_ptr<NXKit::Application> application;
    NXKit::IOSInputManager* inputManager;
private:
    GLViewController* controller;
    std::shared_ptr<NXKit::IOSVideoContext> videoContext;
};
