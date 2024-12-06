#include <platforms/SkiaInit.h>

#import "include/core/SkSurface.h"
#import "include/gpu/ganesh/GrBackendSurface.h"

#ifdef PLATFORM_DESKTOP
#import "tools/window/mac/MacWindowInfo.h"
#import "tools/window/mac/GaneshANGLEWindowContext_mac.h"

std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window) {
    auto metalView = SDL_Metal_CreateView(window);

    skwindow::MacWindowInfo info{};
    info.fMainView = (__bridge NSView *) metalView;
    return skwindow::MakeGaneshANGLEForMac(info, std::make_unique<skwindow::DisplayParams>());
}
#endif

#ifdef PLATFORM_IOS
#include "tools/window/ios/WindowContextFactory_ios.h"
#import "UIKit/UIKit.h"

std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window) {
    auto metalView = SDL_Metal_CreateView(window);

    auto appWindow = UIApplication.sharedApplication.keyWindow;
    skwindow::IOSWindowInfo info{};
    info.fWindow = appWindow;
    info.fViewController = appWindow.rootViewController;
    return skwindow::MakeMetalForIOS(info, std::make_unique<skwindow::DisplayParams>());
}

#endif

bool platformRunLoop(const std::function<bool()>& runLoop) {
    @autoreleasepool {
        return runLoop();
    }
}
