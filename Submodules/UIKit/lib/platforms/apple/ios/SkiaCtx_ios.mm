#include <platforms/ios/SkiaCtx_ios.h>

#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/ios/GrGLMakeIOSInterface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrBackendSurface.h"

#include <GLES3/gl3.h>
#include <UIKit/UIKit.h>

SkiaCtx_ios::SkiaCtx_ios(SDL_Window* window): SkiaCtx_sdlBase(window) {
    initContext();
}

void SkiaCtx_ios::initContext() {
    auto interface = GrGLInterfaces::MakeIOS();
    _size = getSize();
    interface->fFunctions.fViewport(0, 0, _size.width, _size.height);
    context = GrDirectContexts::MakeGL(interface);
}

float SkiaCtx_ios::getScaleFactor() {
    return UIApplication.sharedApplication.keyWindow.traitCollection.displayScale;
}

sk_sp<SkSurface> SkiaCtx_ios::getBackbufferSurface() {
    auto size = getSize();
    if (_size.width != size.width || _size.height != size.width)
        initContext();

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;
    auto frame = UIApplication.sharedApplication.keyWindow.frame;
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(frame.size.width, frame.size.height, 0, 8, framebuffer_info);

    SkSurfaceProps props;

    return SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                                       kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                                       nullptr, &props);
}

std::unique_ptr<SkiaCtx> MakeSkiaCtx(SDL_Window* window) {
    return std::make_unique<SkiaCtx_ios>(window);
}

bool platformRunLoop(std::function<bool()> loop) {
    @autoreleasepool {
        return loop();
    }
}
