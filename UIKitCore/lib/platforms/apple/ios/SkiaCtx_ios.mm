#include "../AppleSystemFont.hpp"
#include "../AppleGlassPreference.hpp"
#include "../MetalPipelineCache.hpp"
#include "IOSWindowGeometry.hpp"
#include "IOSFrameDiagnostics.hpp"
#include "../../../RenderPerformance.hpp"
#include <CALayer.h>
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
#include <QuartzCore/CATransaction.h>
#include <QuartzCore/CAMediaTiming.h>
#include <SDL3/SDL_metal.h>
#include <UIKit/UIKit.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string_view>

using namespace NXKit;

namespace {

#if __has_feature(objc_arc)
#define NX_OBJC_BRIDGE(type, value) ((__bridge type)(value))
#define NX_OBJC_BRIDGE_RETAIN(value) CFBridgingRetain(value)
#else
#define NX_OBJC_BRIDGE(type, value) ((type)(value))
#define NX_OBJC_BRIDGE_RETAIN(value) ((CFTypeRef)(value))
#endif

::UIWindow *currentWindow(SDL_Window *window) {
    if (window == nullptr) {
        return nil;
    }

    auto properties = SDL_GetWindowProperties(window);
    return NX_OBJC_BRIDGE(
        ::UIWindow *,
        SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr)
    );
}

void configureMetalLayer(CAMetalLayer* layer, id<MTLDevice> device) {
    if (layer == nil || device == nil) {
        return;
    }

    if (layer.device != device) layer.device = device;
    if (layer.pixelFormat != MTLPixelFormatBGRA8Unorm) layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    if (layer.framebufferOnly) layer.framebufferOnly = NO;
    if (![layer.contentsGravity isEqualToString:kCAGravityResize]) layer.contentsGravity = kCAGravityResize;
}

}

SkiaCtx_ios::SkiaCtx_ios(): SkiaCtx_sdlBase() {
    SkGraphics::Init();
    _windowGeometry = std::make_unique<Internal::IOSWindowGeometry>((__bridge void*)currentWindow(window));
    prepareFrame();
    initContext();

    fontMgr = SkFontMgr_New_CoreText(nullptr);
}

SkiaCtx_ios::~SkiaCtx_ios() {
    _windowGeometry.reset();
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
    backendContext.fQueue.reset(Internal::makeIOSDiagnosticQueue(queue.get()));

    skgpu::graphite::ContextOptions options;
    options.fRequireOrderedRecordings = true;
    // Device profiling control: compare Graphite's raster path antialiasing
    // with its default hardware MSAA. Normal app behavior is unchanged.
    if(const auto value=std::getenv("NXKIT_RASTER_PATHS");value&&std::string_view(value)=="1")
        options.fInternalMultisampleCount=skgpu::graphite::SampleCount::k1;
    _pipelineCache=std::make_shared<Internal::MetalPipelineCache>(device.get());
    backendContext.fPipelineCache=_pipelineCache;
    _pipelineCache->configure(backendContext,options);
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

    finishGraphiteWork(context.get(), recorder.get());
    if (recorder) {
        recorder->freeGpuResources();
    }
    // Recorder owns the client image provider. Let Recorder's destructor first
    // abandon its tracked devices and then release the promoted-image cache
    // while its resource provider and the Graphite context are still alive.
    recorder.reset();
    if (context) {
        context->freeGpuResources();
    }
    context.reset();

    _pipelineCache.reset();

    if (auto layer = NX_OBJC_BRIDGE(CAMetalLayer*, metalLayer())) {
        layer.device = nil;
    }

    queue.reset();
    device.reset();
}

void SkiaCtx_ios::prepareFrame() {
    if (!_windowGeometry) return;
    if (!_frameGeometry) _frameGeometry = std::make_unique<Internal::IOSWindowFrame>();
    *_frameGeometry = _windowGeometry->sample();
    if (_frameGeometry->transitioning) NXKit::CALayer::requestFramerate(screenFrameRate());
}

NXSize SkiaCtx_ios::getSize() {
    if (_frameGeometry && _frameGeometry->width > 0 && _frameGeometry->height > 0)
        return {_frameGeometry->width * _extraScaleFactor, _frameGeometry->height * _extraScaleFactor};
    return SkiaCtx_sdlBase::getSize() * _extraScaleFactor;
}

float SkiaCtx_ios::getScaleFactor() {
    // SDL pixel dimensions can already be the rotation endpoint while the
    // animated point bounds are still between orientations. Never derive
    // display density by dividing those two different geometry snapshots.
    if (_frameGeometry) return _frameGeometry->scale / _extraScaleFactor;
    return SkiaCtx_sdlBase::getScaleFactor() / _extraScaleFactor;
}

NXFloat SkiaCtx_ios::getGlassTintAmount() {
    return Internal::appleGlassTintAmount().value_or(UITraitCollection::defaultGlassTintAmount);
}

NXKit::UIUserInterfaceStyle SkiaCtx_ios::getThemeMode() {
    auto uiWindow = currentWindow(window);
    if (uiWindow.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark) {
        return NXKit::UIUserInterfaceStyle::dark;
    } else {
        return NXKit::UIUserInterfaceStyle::light;
    }
}

std::pair<NXKit::UIUserInterfaceSizeClass, NXKit::UIUserInterfaceSizeClass> SkiaCtx_ios::getSizeClasses() {
    auto uiWindow = currentWindow(window);
    if (uiWindow == nil) return SkiaCtx::getSizeClasses();
    // UIKit owns device-, orientation-, and multitasking-specific rules. Read
    // them from the SDL window's root environment, without rescaling points.
    auto traits = uiWindow.rootViewController.traitCollection ?: uiWindow.traitCollection;
    const auto convert = [](::UIUserInterfaceSizeClass value) {
        switch (value) {
            case UIUserInterfaceSizeClassCompact: return NXKit::UIUserInterfaceSizeClass::compact;
            case UIUserInterfaceSizeClassRegular: return NXKit::UIUserInterfaceSizeClass::regular;
            default: return NXKit::UIUserInterfaceSizeClass::unspecified;
        }
    };
    return {convert(traits.horizontalSizeClass), convert(traits.verticalSizeClass)};
}

NXKit::UIEdgeInsets SkiaCtx_ios::deviceSafeAreaInsets() {
    return _frameGeometry ? NXKit::UIEdgeInsets{_frameGeometry->top, _frameGeometry->left, _frameGeometry->bottom, _frameGeometry->right} : NXKit::UIEdgeInsets{};
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

    const auto size = getSize() / _extraScaleFactor;
    const auto scale = getScaleFactor() * _extraScaleFactor;
    const int drawableWidth = int(std::lround(size.width * scale));
    const int drawableHeight = int(std::lround(size.height * scale));
    if (drawableWidth <= 0 || drawableHeight <= 0) return nullptr;

    // SDL/UIKit own the native view's frame and rotation transform. Changing
    // layer.frame here would overwrite the coordinator's animation endpoint.
    // The texture follows the probe's presented bounds instead, at native scale.
    const auto drawableSize = CGSizeMake(drawableWidth, drawableHeight);
    if (layer.contentsScale != scale || !CGSizeEqualToSize(layer.drawableSize, drawableSize)) {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        if (layer.contentsScale != scale) layer.contentsScale = scale;
        if (!CGSizeEqualToSize(layer.drawableSize, drawableSize)) layer.drawableSize = drawableSize;
        [CATransaction commit];
    }
    _size = size;

    auto currentDrawable = [layer nextDrawable];
    if (currentDrawable == nil || currentDrawable.texture == nil) {
        return nullptr;
    }

    drawable.reset(CFRetain((__bridge CFTypeRef)currentDrawable));
    Internal::observeIOSDrawable(drawable.get());

    auto backendTexture = skgpu::graphite::BackendTextures::MakeMetal(
            SkISize::Make(int(currentDrawable.texture.width), int(currentDrawable.texture.height)),
            (__bridge CFTypeRef)currentDrawable.texture);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry, 0, 1);
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
    if(const auto metrics=Internal::RenderPerformance::current;metrics&&_pipelineCache) {
        const auto now=CACurrentMediaTime();
        if(now-_diagnosticMemoryTime>=5) {
            _diagnosticMemoryTime=now;
            _diagnosticMemory={NX_OBJC_BRIDGE(id<MTLDevice>,device.get()).currentAllocatedSize,
                context->currentBudgetedBytes()+recorder->currentBudgetedBytes(),
                context->currentPurgeableBytes()+recorder->currentPurgeableBytes()};
        }
        metrics->metalAllocatedBytes=_diagnosticMemory[0];
        metrics->graphiteBudgetedBytes=_diagnosticMemory[1];
        metrics->graphitePurgeableBytes=_diagnosticMemory[2];
        metrics->memorySampleTime=_diagnosticMemoryTime;
        const auto stats=_pipelineCache->statistics();
        metrics->metalLibraryCompiles=stats.libraryCompiles;
        metrics->metalPipelineCompiles=stats.pipelineCompiles;
        metrics->metalArchiveHits=stats.archiveHits;
        Internal::observeIOSPipelineFailures(stats.failures);
    }

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

sk_sp<SkTypeface> SkiaCtx_ios::getDefaultTypeface(const SkFontStyle& style, NXFloat pointSize) const {
    return Internal::appleSystemTypeface(style, pointSize);
}

std::array<NXFloat,4> SkiaCtx_ios::windowCornerRadii() {
    auto radii = _frameGeometry ? _frameGeometry->cornerRadii : std::array<NXFloat,4>{};
    for (auto& radius : radii) radius *= _extraScaleFactor;
    return radii;
}
