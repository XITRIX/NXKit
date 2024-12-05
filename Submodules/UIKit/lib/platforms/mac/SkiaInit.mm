#include <platforms/mac/SkiaInit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import "include/core/SkColorSpace.h"
#import "include/core/SkSurface.h"

#import "include/gpu/ganesh/GrBackendSurface.h"
#import "include/gpu/ganesh/SkSurfaceGanesh.h"

// Metal
#import "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"

// Window Metal
#import "tools/window/mac/GaneshMetalWindowContext_mac.h"
#import "tools/window/mac/MacWindowInfo.h"

#import "tools/window/mac/GaneshANGLEWindowContext_mac.h"
#import "tools/window/mac/GraphiteNativeMetalWindowContext_mac.h"

//std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window) {
//    auto metalView = SDL_Metal_CreateView(window);
//
//    skwindow::MacWindowInfo info{};
//    info.fMainView = (__bridge NSView *) metalView;
//    return skwindow::MakeGaneshMetalForMac(info, std::make_unique<skwindow::DisplayParams>());
//}

std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window) {
    auto metalView = SDL_Metal_CreateView(window);

    skwindow::MacWindowInfo info{};
    info.fMainView = (__bridge NSView *) metalView;
    return skwindow::MakeGaneshANGLEForMac(info, std::make_unique<skwindow::DisplayParams>());
}

//std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window) {
//    auto metalView = SDL_Metal_CreateView(window);
//
//    skwindow::MacWindowInfo info{};
//    info.fMainView = (__bridge NSView *) metalView;
//    return skwindow::MakeGraphiteNativeMetalForMac(info, std::make_unique<skwindow::DisplayParams>());
//}

bool platformRunLoop(const std::function<bool()>& runLoop) {
    @autoreleasepool {
        return runLoop();
    }
}
