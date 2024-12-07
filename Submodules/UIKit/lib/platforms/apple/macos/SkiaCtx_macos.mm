#include <platforms/macos/SkiaCtx_macos.h>

#include "include/core/SkColorSpace.h"
#include <include/core/SkGraphics.h>
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/ios/GrGLMakeIOSInterface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/private/base/SkTemplates.h"

#include "include/ports/SkFontMgr_mac_ct.h"

#include <GLES3/gl3.h>
#include <Cocoa/Cocoa.h>
#include <dlfcn.h>

using namespace NXKit;

SkiaCtx_macos::SkiaCtx_macos(SDL_Window* window): SkiaCtx_sdlBase(window) {
    SkGraphics::Init();
    initContext();

    fontMgr = SkFontMgr_New_CoreText(nullptr);
}

void SkiaCtx_macos::initContext() {
    _size = getSize();
    static const char kPath[] = "../Frameworks/libEGL.dylib";
    std::unique_ptr<void, SkFunctionObject<dlclose>> lib(dlopen(kPath, RTLD_LAZY));
    auto interface = GrGLMakeAssembledGLESInterface(lib.get(), [](void* ctx, const char* name) {
        return (GrGLFuncPtr)dlsym(ctx ? ctx : RTLD_DEFAULT, name); });

    context = GrDirectContexts::MakeGL(interface);
}

//float SkiaCtx_macos::getScaleFactor() {
//    return NSApplication.sharedApplication.keyWindow.backingScaleFactor;
//}

sk_sp<SkSurface> SkiaCtx_macos::getBackbufferSurface() {
    auto size = getSize();
    if (_size.width != size.width || _size.height != size.height) {
        _size = size;
        initContext();
    }

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;
    auto scaleFactor = getScaleFactor();
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(size.width * scaleFactor,
                                                                  size.height * scaleFactor,
                                                                  0, 8,
                                                                  framebuffer_info);

    SkSurfaceProps props;

    glViewport(0, 0, _size.width * scaleFactor, _size.height * scaleFactor);
    return SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                               kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                               nullptr, &props);
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx(SDL_Window* window) {
    return std::make_unique<SkiaCtx_macos>(window);
}

bool NXKit::platformRunLoop(std::function<bool()> loop) {
    @autoreleasepool {
        return loop();
    }
}
