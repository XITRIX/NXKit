#include <platforms/ios/SkiaCtx_ios.h>

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
#include <UIKit/UIKit.h>
#include <dlfcn.h>

using namespace NXKit;

SkiaCtx_ios::SkiaCtx_ios(): SkiaCtx_sdlBase() {
    SkGraphics::Init();
    initContext();

    fontMgr = SkFontMgr_New_CoreText(nullptr);
}

void SkiaCtx_ios::initContext() {
    _size = getSize();
    static const char kPath[] = "./Frameworks/MetalANGLE.framework/MetalANGLE";
    std::unique_ptr<void, SkFunctionObject<dlclose>> lib(dlopen(kPath, RTLD_LAZY));
    auto interface = GrGLMakeAssembledGLESInterface(lib.get(), [](void* ctx, const char* name) {
        return (GrGLFuncPtr)dlsym(ctx ? ctx : RTLD_DEFAULT, name); });

    context = GrDirectContexts::MakeGL(interface);
}

NXSize SkiaCtx_ios::getSize() {
    auto window = UIApplication.sharedApplication.keyWindow;
    auto layer = window.layer.presentationLayer;
    if (layer == NULL) layer = window.layer;
    NXSize size = { static_cast<NXFloat>(layer.bounds.size.width), static_cast<NXFloat>(layer.bounds.size.height) };
    return size * _extraScaleFactor;
}

float SkiaCtx_ios::getScaleFactor() {
    return UIApplication.sharedApplication.keyWindow.traitCollection.displayScale / _extraScaleFactor;
}

NXKit::UIUserInterfaceStyle SkiaCtx_ios::getThemeMode() {
    auto window = UIApplication.sharedApplication.keyWindow;
    if (window.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark) {
        return NXKit::UIUserInterfaceStyle::dark;
    } else {
        return NXKit::UIUserInterfaceStyle::light;
    }
}

sk_sp<SkSurface> SkiaCtx_ios::getBackbufferSurface() {
    auto size = getSize() / _extraScaleFactor;
    if (_size.width == size.width && _size.height == size.height && surface != nullptr) { return surface; }

    _size = size;

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;
    auto pureScaleFactor = getScaleFactor() * _extraScaleFactor;
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(size.width * pureScaleFactor,
                                                                  size.height * pureScaleFactor,
                                                                  0, 8,
                                                                  framebuffer_info);

    SkSurfaceProps props;

    glViewport(0, 0, _size.width * pureScaleFactor, _size.height * pureScaleFactor);
    surface = SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                                       kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                                       nullptr, &props);
    
    return surface;
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_ios>();
}

bool NXKit::platformRunLoop(std::function<bool()> loop) {
    @autoreleasepool {
        return loop();
    }
}
