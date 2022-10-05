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
    
    NXKit::Application* application;
    NXKit::IOSInputManager* inputManager;
private:
    GLViewController* controller;
    NXKit::IOSVideoContext* videoContext;
};
