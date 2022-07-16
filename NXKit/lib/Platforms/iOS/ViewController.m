//
//  ViewController.m
//  NXKit-iOS
//
//  Created by Даниил Виноградов on 16.07.2022.
//

#import "ViewController.h"
#import <OpenGLES/ES2/glext.h>
#include "nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "ios_video_layer.h"

@interface ViewController () {
    NVGcontext* vg;
    double t;
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }

    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;

    self.preferredFramesPerSecond = 60;

    [self setupGL];
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];

    vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    assert(vg);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
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

    nvgBeginFrame(vg, winWidth, winHeight, [[UIScreen mainScreen] scale]);

//    renderDemo(vg, mx,my, winWidth, winHeight, t, blowup, &data);
    //renderGraph(vg, 5,5, &fps);

    nvgEndFrame(vg);
}


@end
