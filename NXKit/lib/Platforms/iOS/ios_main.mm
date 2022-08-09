//
//  ios_video_layer.m
//  NXKit
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#import "ios_main.h"
#import "ios_video.hpp"

#include "nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"


#import "nx_application.h"


#include "IOSInputManager.hpp"

using namespace NXKit;

@interface IOSVideoLayer () {
    NVGcontext* vg;
}

@property (nonatomic) IOSVideoContext *nxcontext;

@end

@implementation IOSVideoLayer

- (instancetype)initWithPath:(NSString *)path {
    self = [super init];

    if (self) {
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

        if (!self.context) {
            NSLog(@"Failed to create ES context");
        }

        [EAGLContext setCurrentContext:self.context];
        vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
        _nxcontext = new IOSVideoContext(vg);

        makeApplication(_nxcontext, [path UTF8String]);
    }

    return self;
}

- (void)setScale:(float)width height:(float)height scale:(float)scale {
    _nxcontext->setScale(width, height, scale * 2);
}

- (void)applicationLoop {
    _nxcontext->applicationLoop();
}

- (void)setKey:(int)key state:(bool)state
{
    IOSInputManager* manager = (IOSInputManager*) InputManager::shared();
    manager->setKey((BrlsKeyboardScancode) key, state);
}

@end
