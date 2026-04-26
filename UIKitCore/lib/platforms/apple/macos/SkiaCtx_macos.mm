#include <platforms/macos/SkiaCtx_macos.h>

#include "include/core/SkColorSpace.h"
#include <include/core/SkGraphics.h>
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"

#include "include/ports/SkFontMgr_mac_ct.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <Cocoa/Cocoa.h>
#include <SDL_metal.h>

using namespace NXKit;

namespace {

#if __has_feature(objc_arc)
#define NX_OBJC_BRIDGE(type, value) ((__bridge type)(value))
#define NX_OBJC_BRIDGE_RETAIN(type, value) ((type)CFBridgingRetain(value))
#else
#define NX_OBJC_BRIDGE(type, value) ((type)(value))
#define NX_OBJC_BRIDGE_RETAIN(type, value) ((type)(value))
#endif

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
SkiaCtx_macos::SkiaCtx_macos(): SkiaCtx_sdlBase() {
    SkGraphics::Init();
    fontMgr = SkFontMgr_New_CoreText(nullptr);
}

SkiaCtx_macos::~SkiaCtx_macos() {
    destroyContext();
}

bool SkiaCtx_macos::ensureRenderThread() {
    auto currentThread = std::this_thread::get_id();
    if (renderThread == std::thread::id()) {
        renderThread = currentThread;
        return true;
    }

    return renderThread == currentThread;
}

void SkiaCtx_macos::initContext() {
    if (context || !ensureRenderThread()) {
        return;
    }

    _size = getSize();

    auto metalDevice = MTLCreateSystemDefaultDevice();
    if (metalDevice == nil) {
        NSLog(@"Failed to create Metal device");
        return;
    }

    device.reset(NX_OBJC_BRIDGE_RETAIN(GrMTLHandle, metalDevice));

    auto commandQueue = [metalDevice newCommandQueue];
    if (commandQueue == nil) {
        NSLog(@"Failed to create Metal command queue");
        device.reset();
        return;
    }

    queue.reset(NX_OBJC_BRIDGE_RETAIN(GrMTLHandle, commandQueue));

    configureMetalLayer(NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer()), metalDevice);

    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.retain(device.get());
    backendContext.fQueue.retain(queue.get());
    context = GrDirectContexts::MakeMetal(backendContext);
}

void SkiaCtx_macos::destroyContext() {
    std::lock_guard<std::recursive_mutex> guard(contextMutex);

    surface = nullptr;
    drawable.reset();

    if (context) {
        context->flushAndSubmit(GrSyncCpu::kYes);
        context->abandonContext();
        context.reset();
    }

    if (auto layer = NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer())) {
        layer.device = nil;
    }

    queue.reset();
    device.reset();
    renderThread = std::thread::id();
}

//float SkiaCtx_macos::getScaleFactor() {
//    return NSApplication.sharedApplication.keyWindow.backingScaleFactor;
//}

UIUserInterfaceStyle SkiaCtx_macos::getThemeMode() {
    auto isDark= [[NSUserDefaults.standardUserDefaults stringForKey: @"AppleInterfaceStyle"]  isEqual: @"Dark"];
    if (isDark) {
        return NXKit::UIUserInterfaceStyle::dark;
    } else {
        return NXKit::UIUserInterfaceStyle::light;
    }
}

sk_sp<SkSurface> SkiaCtx_macos::getBackbufferSurface() {
    std::lock_guard<std::recursive_mutex> guard(contextMutex);
    if (!ensureRenderThread()) {
        return nullptr;
    }

    initContext();
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
    SDL_Metal_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    if (drawableWidth <= 0 || drawableHeight <= 0) {
        return nullptr;
    }

    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    auto logicalWidth = static_cast<CGFloat>(windowWidth > 0 ? windowWidth : drawableWidth);
    auto logicalHeight = static_cast<CGFloat>(windowHeight > 0 ? windowHeight : drawableHeight);

    layer.frame = CGRectMake(0.0, 0.0, logicalWidth, logicalHeight);
    layer.contentsScale = logicalWidth > 0.0 ? drawableWidth / logicalWidth : 1.0;
    layer.drawableSize = CGSizeMake(drawableWidth, drawableHeight);

    _size = getSize();

    auto currentDrawable = [layer nextDrawable];
    if (currentDrawable == nil || currentDrawable.texture == nil) {
        return nullptr;
    }

    drawable.reset((GrMTLHandle)CFRetain((__bridge CFTypeRef)currentDrawable));

    GrMtlTextureInfo textureInfo;
    textureInfo.fTexture.retain((__bridge GrMTLHandle)currentDrawable.texture);

    GrBackendRenderTarget backendRenderTarget = GrBackendRenderTargets::MakeMtl(drawableWidth,
                                                                                drawableHeight,
                                                                                textureInfo);
    SkSurfaceProps props;
    surface = SkSurfaces::WrapBackendRenderTarget(context.get(),
                                                  backendRenderTarget,
                                                  kTopLeft_GrSurfaceOrigin,
                                                  kBGRA_8888_SkColorType,
                                                  nullptr,
                                                  &props);

    if (!surface) {
        drawable.reset();
    }

    return surface;
}

void SkiaCtx_macos::flushAndSubmit(sk_sp<SkSurface> surface) {
    std::lock_guard<std::recursive_mutex> guard(contextMutex);
    if (!ensureRenderThread() || !context || !surface) {
        return;
    }

    context->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
}

void SkiaCtx_macos::swapBuffers() {
    std::lock_guard<std::recursive_mutex> guard(contextMutex);
    if (!ensureRenderThread()) {
        return;
    }

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

bool SkiaCtx_macos::platformRunLoop(std::function<bool()> loop) {
    @autoreleasepool {
        std::lock_guard<std::recursive_mutex> guard(contextMutex);
        if (renderThread == std::thread::id()) {
            renderThread = std::this_thread::get_id();
        }
        currentRunLoopStartTimer = Timer();
        return loop();
    }
}

std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_macos>();
}

#undef NX_OBJC_BRIDGE
#undef NX_OBJC_BRIDGE_RETAIN
