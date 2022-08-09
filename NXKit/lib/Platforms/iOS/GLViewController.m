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

#include "ios_main.h"

@interface GLViewController () {
    IOSVideoLayer* videoLayer;
    double t;
}

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation GLViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    videoLayer = [[IOSVideoLayer alloc] initWithPath:[NSBundle mainBundle].resourcePath];

    GLKView *view = (GLKView *)self.view;
    view.context = [videoLayer context];
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;

    self.preferredFramesPerSecond = 60;

    [self setupGL];
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

    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    int winWidth = self.view.frame.size.width;
    int winHeight = self.view.frame.size.height;
    float mx = 0; // mouse x and y
    float my = 0;
    int blowup = 0;

    t += 1.0/60.0;

//    [videoLayer setScale:(view.frame.size.width / 1.3) height:(view.frame.size.height / 1.3) scale:[[UIScreen mainScreen] scale] ];
    [videoLayer setScale:view.frame.size.width height:view.frame.size.height scale:[[UIScreen mainScreen] scale] ];

//    applicationLoop();
    [videoLayer applicationLoop];
//    nvgBeginFrame(vg, winWidth, winHeight, [[UIScreen mainScreen] scale]);

//    renderDemo(vg, mx,my, winWidth, winHeight, t, blowup, &data);
    //renderGraph(vg, 5,5, &fps);

//    nvgEndFrame(vg);
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{

}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    
}

- (void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    for (UIPress* press in presses) {
//        [[press key] keyCode];
    }
}


@end
