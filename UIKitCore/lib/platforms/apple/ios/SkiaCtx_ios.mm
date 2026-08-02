#include <platforms/ios/SkiaCtx_ios.h>

#include "include/core/SkColorSpace.h"
#include <include/core/SkGraphics.h>
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"

#include "include/ports/SkFontMgr_mac_ct.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <SDL3/SDL_metal.h>
#include <UIKit/UIKit.h>
#include <algorithm>
#include <cmath>

using namespace NXKit;

namespace {

#if __has_feature(objc_arc)
#define NX_OBJC_BRIDGE(type, value) ((__bridge type)(value))
#define NX_OBJC_BRIDGE_RETAIN(value) CFBridgingRetain(value)
#else
#define NX_OBJC_BRIDGE(type, value) ((type)(value))
#define NX_OBJC_BRIDGE_RETAIN(value) ((CFTypeRef)(value))
#endif

UIWindow *currentWindow(SDL_Window *window) {
    if (window == nullptr) {
        return nil;
    }

    auto properties = SDL_GetWindowProperties(window);
    return NX_OBJC_BRIDGE(
        UIWindow *,
        SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr)
    );
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

void configureMetalLayer(CAMetalLayer* layer, id<MTLDevice> device) {
    if (layer == nil || device == nil) {
        return;
    }

    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = NO;
    layer.contentsGravity = kCAGravityTopLeft;
}

}

SkiaCtx_ios::SkiaCtx_ios(): SkiaCtx_sdlBase() {
    SkGraphics::Init();
    initContext();

    fontMgr = SkFontMgr_New_CoreText(nullptr);
}

SkiaCtx_ios::~SkiaCtx_ios() {
    destroyContext();
}

void SkiaCtx_ios::initContext() {
    _size = getSize();

    auto metalDevice = MTLCreateSystemDefaultDevice();
    if (metalDevice == nil) {
        NSLog(@"Failed to create Metal device");
        return;
    }

    device.reset(NX_OBJC_BRIDGE_RETAIN(metalDevice));

    auto commandQueue = [metalDevice newCommandQueue];
    if (commandQueue == nil) {
        NSLog(@"Failed to create Metal command queue");
        device.reset();
        return;
    }

    queue.reset(NX_OBJC_BRIDGE_RETAIN(commandQueue));

    configureMetalLayer(NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer()), metalDevice);

    skgpu::graphite::MtlBackendContext backendContext = {};
    backendContext.fDevice.retain(device.get());
    backendContext.fQueue.retain(queue.get());

    skgpu::graphite::ContextOptions options;
    options.fRequireOrderedRecordings = true;
    context = skgpu::graphite::ContextFactory::MakeMetal(backendContext, options);
    if (!context) {
        NSLog(@"Failed to create Graphite Metal context");
        queue.reset();
        device.reset();
        return;
    }

    recorder = createGraphiteRecorder(context.get());
    if (!recorder) {
        NSLog(@"Failed to create Graphite recorder");
        context.reset();
        queue.reset();
        device.reset();
    }
}

void SkiaCtx_ios::destroyContext() {
    surface = nullptr;
    drawable.reset();

    if (context) {
        context->submit(skgpu::graphite::SyncToCpu::kYes);
    }
    recorder.reset();
    context.reset();

    if (auto layer = NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer())) {
        layer.device = nil;
    }

    queue.reset();
    device.reset();
}

NXSize SkiaCtx_ios::getSize() {
    auto size = currentViewSize(currentWindow(window));
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
    SDL_GetWindowSizeInPixels(window, &drawableWidth, &drawableHeight);

    auto size = currentViewSize(currentWindow(window));
    if (drawableWidth > 0 && drawableHeight > 0 && size.width > 0.0 && size.height > 0.0) {
        auto horizontalScale = static_cast<float>(drawableWidth / size.width);
        auto verticalScale = static_cast<float>(drawableHeight / size.height);
        return std::max(horizontalScale, verticalScale) / _extraScaleFactor;
    }

    auto uiWindow = currentWindow(window);
    if (uiWindow != nil) {
        return uiWindow.screen.nativeScale / _extraScaleFactor;
    }

    return SkiaCtx_sdlBase::getScaleFactor() / _extraScaleFactor;
}

NXKit::UIUserInterfaceStyle SkiaCtx_ios::getThemeMode() {
    auto uiWindow = currentWindow(window);
    if (uiWindow.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark) {
        return NXKit::UIUserInterfaceStyle::dark;
    } else {
        return NXKit::UIUserInterfaceStyle::light;
    }
}

NXKit::UIEdgeInsets SkiaCtx_ios::deviceSafeAreaInsets() {
    auto safeArea = currentWindow(window).safeAreaInsets;
    return { (NXFloat)safeArea.top, (NXFloat)safeArea.left, (NXFloat)safeArea.bottom, (NXFloat)safeArea.right };
}

sk_sp<SkSurface> SkiaCtx_ios::getBackbufferSurface() {
    if (!context) {
        return nullptr;
    }

    auto layer = NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer());
    if (layer == nil) {
        return nullptr;
    }

    configureMetalLayer(layer, NX_OBJC_BRIDGE(id<MTLDevice>, device.get()));

    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_GetWindowSizeInPixels(window, &drawableWidth, &drawableHeight);
    if (drawableWidth <= 0 || drawableHeight <= 0) {
        auto contentScale = layer.contentsScale;
        if (contentScale <= 0.0) {
            auto uiWindow = currentWindow(window);
            if (uiWindow != nil) {
                contentScale = uiWindow.screen.nativeScale;
            }
        }

        if (layer.bounds.size.width > 0.0 && layer.bounds.size.height > 0.0 && contentScale > 0.0) {
            drawableWidth = static_cast<int>(std::lround(layer.bounds.size.width * contentScale));
            drawableHeight = static_cast<int>(std::lround(layer.bounds.size.height * contentScale));
        }
    }

    if (drawableWidth <= 0 || drawableHeight <= 0) {
        return nullptr;
    }

    auto logicalSize = currentViewSize(currentWindow(window));
    if (CGSizeEqualToSize(logicalSize, CGSizeZero)) {
        logicalSize = CGSizeMake(drawableWidth, drawableHeight);
    }

    layer.frame = CGRectMake(0.0, 0.0, logicalSize.width, logicalSize.height);
    layer.contentsScale = logicalSize.width > 0.0 ? drawableWidth / logicalSize.width : 1.0;
    layer.drawableSize = CGSizeMake(drawableWidth, drawableHeight);

    _size = getSize() / _extraScaleFactor;

    auto currentDrawable = [layer nextDrawable];
    if (currentDrawable == nil || currentDrawable.texture == nil) {
        return nullptr;
    }

    drawable.reset(CFRetain((__bridge CFTypeRef)currentDrawable));

    auto backendTexture = skgpu::graphite::BackendTextures::MakeMetal(
            SkISize::Make(drawableWidth, drawableHeight),
            (__bridge CFTypeRef)currentDrawable.texture);
    SkSurfaceProps props;
    surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                             backendTexture,
                                             nullptr,
                                             &props,
                                             nullptr,
                                             nullptr,
                                             "NXKit iOS backbuffer");

    if (!surface) {
        drawable.reset();
    }
    
    return surface;
}

void SkiaCtx_ios::swapBuffers() {
    if (!queue || !drawable) {
        return;
    }

    auto commandQueue = NX_OBJC_BRIDGE(id<MTLCommandQueue>, queue.get());
    auto currentDrawable = NX_OBJC_BRIDGE(id<CAMetalDrawable>, drawable.get());
    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    commandBuffer.label = @"Present";
    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];

    surface = nullptr;
    drawable.reset();
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

#undef NX_OBJC_BRIDGE
#undef NX_OBJC_BRIDGE_RETAIN
