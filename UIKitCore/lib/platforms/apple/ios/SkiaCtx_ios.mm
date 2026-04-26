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
#include <SDL_syswm.h>
#include <UIKit/UIKit.h>
#include <algorithm>
#include <dlfcn.h>

using namespace NXKit;

namespace {

UIWindow *currentWindow() {
    auto app = UIApplication.sharedApplication;
    auto uiWindow = app.keyWindow;
    if (uiWindow == nil && app.windows.count > 0) {
        uiWindow = app.windows.firstObject;
    }
    return uiWindow;
}

CGSize currentViewSize(UIWindow *uiWindow) {
    if (uiWindow == nil) {
        return CGSizeZero;
    }

    auto view = uiWindow.rootViewController.view;
    if (view != nil && !CGRectIsEmpty(view.bounds)) {
        return view.bounds.size;
    }

    if (!CGRectIsEmpty(uiWindow.bounds)) {
        return uiWindow.bounds.size;
    }

    return CGSizeZero;
}

}

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
    auto size = currentViewSize(currentWindow());
    if (CGSizeEqualToSize(size, CGSizeZero)) {
        return SkiaCtx_sdlBase::getSize() * _extraScaleFactor;
    }

    return {
        static_cast<NXFloat>(size.width * _extraScaleFactor),
        static_cast<NXFloat>(size.height * _extraScaleFactor)
    };
}

float SkiaCtx_ios::getScaleFactor() {
    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);

    auto size = currentViewSize(currentWindow());
    if (drawableWidth > 0 && drawableHeight > 0 && size.width > 0.0 && size.height > 0.0) {
        auto horizontalScale = static_cast<float>(drawableWidth / size.width);
        auto verticalScale = static_cast<float>(drawableHeight / size.height);
        return std::max(horizontalScale, verticalScale) / _extraScaleFactor;
    }

    auto uiWindow = currentWindow();
    if (uiWindow != nil) {
        return uiWindow.screen.nativeScale / _extraScaleFactor;
    }

    return SkiaCtx_sdlBase::getScaleFactor() / _extraScaleFactor;
}

NXKit::UIUserInterfaceStyle SkiaCtx_ios::getThemeMode() {
    auto uiWindow = currentWindow();
    if (uiWindow.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark) {
        return NXKit::UIUserInterfaceStyle::dark;
    } else {
        return NXKit::UIUserInterfaceStyle::light;
    }
}

NXKit::UIEdgeInsets SkiaCtx_ios::deviceSafeAreaInsets() {
    auto safeArea = currentWindow().safeAreaInsets;
    return { (NXFloat)safeArea.top, (NXFloat)safeArea.left, (NXFloat)safeArea.bottom, (NXFloat)safeArea.right };
}

sk_sp<SkSurface> SkiaCtx_ios::getBackbufferSurface() {
    auto size = getSize() / _extraScaleFactor;

    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);

    if (_size.width == size.width
        && _size.height == size.height
        && surface != nullptr
        && surface->width() == drawableWidth
        && surface->height() == drawableHeight)
    {
        return surface;
    }

    _size = size;

    GrGLFramebufferInfo framebuffer_info;
    framebuffer_info.fFormat = GL_RGBA8;
    framebuffer_info.fFBOID = 0;

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info) == SDL_TRUE && info.subsystem == SDL_SYSWM_UIKIT) {
        framebuffer_info.fFBOID = info.info.uikit.framebuffer;
    }

    if (drawableWidth <= 0 || drawableHeight <= 0) {
        auto pureScaleFactor = getScaleFactor() * _extraScaleFactor;
        drawableWidth = size.width * pureScaleFactor;
        drawableHeight = size.height * pureScaleFactor;
    }

    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(drawableWidth,
                                                                  drawableHeight,
                                                                  0, 8,
                                                                  framebuffer_info);

    SkSurfaceProps props;

    glViewport(0, 0, drawableWidth, drawableHeight);
    surface = SkSurfaces::WrapBackendRenderTarget(context.get(), target,
                                                       kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                                       nullptr, &props);
    
    return surface;
}


bool SkiaCtx_ios::platformRunLoop(std::function<bool()> loop) {
    @autoreleasepool {
        currentRunLoopStartTimer = Timer();
        return loop();
    }
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_ios>();
}
