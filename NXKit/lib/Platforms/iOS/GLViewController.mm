//
//  ViewController.m
//  NXKit-iOS
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#import "GLViewController.h"
#import <OpenGLES/ES2/glext.h>
//#include "nanovg.h"
//#define NANOVG_GLES2_IMPLEMENTATION
//#include "nanovg_gl.h"
//#include "nanovg_gl_utils.h"

#include "IOSMain.hpp"

@interface GLViewController () {
//    IOSVideoLayer* videoLayer;
    IOSMain* main;
    double t;
}

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation GLViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    main = new IOSMain(self);

//    videoLayer = [[IOSVideoLayer alloc] initWithPath:[NSBundle mainBundle].resourcePath];

    GLKView *view = (GLKView *)self.view;
//    view.context = [videoLayer context];
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;

    self.preferredFramesPerSecond = 120;

    [self setupGL];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    main->application->onWindowResized(self.view.frame.size.width, self.view.frame.size.height, self.view.window.screen.scale);
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    main->application->onWindowResized(size.width, size.height, self.view.window.screen.scale);
}

- (void)setupGL
{
//    [EAGLContext setCurrentContext:[videoLayer context]];

//    assert(vg);
}

- (void)tearDownGL
{
//    [EAGLContext setCurrentContext:[videoLayer context]];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    int winWidth = self.view.frame.size.width;
    int winHeight = self.view.frame.size.height;
    float mx = 0; // mouse x and y
    float my = 0;
    int blowup = 0;

    t += 1.0/60.0;

    main->applicationLoop();
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    if (@available(iOS 13.4, *)) {
        for (UIPress* press in presses) {
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardDownArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_DOWN] = true;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardUpArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_UP] = true;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardLeftArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_LEFT] = true;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardRightArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_RIGHT] = true;
            }
        }
    }
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    if (@available(iOS 13.4, *)) {
        for (UIPress* press in presses) {
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardDownArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_DOWN] = false;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardUpArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_UP] = false;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardLeftArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_LEFT] = false;
            }
            if (press.key.keyCode == UIKeyboardHIDUsage::UIKeyboardHIDUsageKeyboardRightArrow) {
                main->inputManager->currentKeys[NXKit::BRLS_KBD_KEY_RIGHT] = false;
            }
        }
    }
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    for (UITouch* touch in touches) {
        CGPoint position = [touch locationInView:self.view];
        main->inputManager->currentTouches[touch.hash] = NXKit::Point(position.x, position.y);
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    for (UITouch* touch in touches) {
        CGPoint position = [touch locationInView:self.view];
        main->inputManager->currentTouches[touch.hash] = NXKit::Point(position.x, position.y);
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    for (UITouch* touch in touches) {
        main->inputManager->currentTouches.erase(touch.hash);
    }
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    for (UITouch* touch in touches) {
        main->inputManager->currentTouches.erase(touch.hash);
    }
}


@end
