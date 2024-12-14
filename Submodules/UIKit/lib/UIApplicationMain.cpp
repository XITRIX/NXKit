#include <UIApplicationMain.h>
#include <UIApplication.h>
#include <Timer.h>
// #include <UIRenderer.h>
#include <tools/SharedBase.hpp>
#include <DispatchQueue.h>
#include <SkiaCtx.h>

namespace NXKit {

bool applicationRunLoop() {
    auto currentTime = Timer();
    UIApplication::shared->handleEventsIfNeeded();
    DispatchQueue::main()->performAll(); // TODO: May be need to be after rendering loop
//        UIRenderer::main()->render(UIApplication::shared->keyWindow.lock(), currentTime);


    UIView::animateIfNeeded(currentTime);


    // Move to UIRenderer
    auto surface = SkiaCtx::_main->getBackbufferSurface();
    if (!surface) return true;

    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    canvas->save();
    auto scale = SkiaCtx::_main->getScaleFactor();
    canvas->scale(scale, scale);
    auto keyWindow = UIApplication::shared->keyWindow.lock();

    keyWindow->layer()->setBounds({ NXPoint::zero, SkiaCtx::_main->getSize() } );
    keyWindow->layer()->presentationOrSelf()->skiaRender(canvas);
    canvas->restore();

    SkiaCtx::_main->flushAndSubmit(surface);
    SkiaCtx::_main->swapBuffers();

    return true;
}

void setupRenderAndRunLoop() {

}

int UIApplicationMain(std::shared_ptr<UIApplicationDelegate> appDelegate) {
    UIApplication::shared = new_shared<UIApplication>();
    SkiaCtx::_main = MakeSkiaCtx();

    UIApplication::shared->delegate = appDelegate;

//    appDelegate->applicationNeedsXIBRegistration(UIApplication::shared.get());
    if (!appDelegate->applicationDidFinishLaunchingWithOptions(UIApplication::shared.get(), {})) {
        return 1;
    }

    while(platformRunLoop([]() { return applicationRunLoop(); }));

    UIApplication::shared = nullptr;
    SkiaCtx::_main = nullptr;
    
    return 0;
};

}
