#include <UIApplicationMain.h>
#include <UIApplication.h>
#include <Timer.h>
// #include <UIRenderer.h>
#include <tools/SharedBase.hpp>
#include <DispatchQueue.h>
#include <SkiaCtx.h>

namespace NXKit {

bool applicationRunLoop() {
    const auto currentTime = Timer();
    // Requests belong to this iteration, including display-link callbacks
    // and presentation animations advanced below.
    CALayer::maxFramerateRequired = 0;
    UIApplication::shared->handleEventsIfNeeded();
    if (UIApplication::shared->isQuitRequested()) {
        return false;
    }
    SkiaCtx::_main->pollBackdropReads();
    DispatchQueue::main()->performAll(); // TODO: May be need to be after rendering loop
//        UIRenderer::main()->render(UIApplication::shared->keyWindow.lock(), currentTime);


    // Move to UIRenderer
    const auto keyWindow = UIApplication::shared->keyWindow.lock();
    NXSize size = SkiaCtx::_main->getSize();
    keyWindow->setFrame({ NXPoint::zero, size });

    const auto scale = SkiaCtx::_main->getScaleFactor();
    // Poll even when geometry is unchanged: UIKit can change size classes at
    // the same bounds after rotation, multitasking, or an environment change.
    keyWindow->updateTraitsFromPlatform();

    UIView::animateIfNeeded(currentTime);
    keyWindow->drawAndLayoutTreeIfNeeded();

    // Update pacing even on a clean frame. Otherwise the last 120 Hz touch
    // frame leaves an idle screen polling its entire tree at 120 Hz forever.
    SkiaCtx::_main->setTargetFrameRate(UIView::maximumAnimationFrameRate());

    static NXSize lastSize;

    if (!CALayer::layerTreeIsDirty && lastSize == size) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        SkiaCtx::_main->sleepForNextFrame();
        return true;
    }

    const auto surface = SkiaCtx::_main->getBackbufferSurface();
    if (!surface) {
        CALayer::layerTreeIsDirty = true;
        SkiaCtx::_main->sleepForNextFrame();
        return true;
    }

    CALayer::layerTreeIsDirty = false;
    lastSize = size;

    const auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    canvas->save();
    canvas->scale(scale, scale);

    UITraitCollection::setCurrent(keyWindow->traitCollection());
    keyWindow->layer()->presentationOrSelf()->skiaRender(canvas);
    canvas->restore();

    SkiaCtx::_main->flushAndSubmit(surface);
    SkiaCtx::_main->swapBuffers();
    SkiaCtx::_main->sleepForNextFrame();

    return true;
}

void setupRenderAndRunLoop() {

}

int UIApplicationMain(const std::shared_ptr<UIApplicationDelegate>& appDelegate) {
    UIApplication::shared = new_shared<UIApplication>();
    SkiaCtx::_main = MakeSkiaCtx();

    UIApplication::shared->delegate = appDelegate;
    UIApplication::shared->startHandlingLifecycleEvents();

    appDelegate->applicationNeedsXIBRegistration(UIApplication::shared.get());
    if (!appDelegate->applicationDidFinishLaunchingWithOptions(UIApplication::shared.get(), {})) {
        return -1;
    }

    while(SkiaCtx::_main->platformRunLoop([]() { return applicationRunLoop(); })) {}

    appDelegate->applicationWillTerminate(UIApplication::shared.get());
    auto animatedLayers = UIView::layersWithAnimations;
    for (const auto& layer : animatedLayers) {
        layer->removeAllAnimations();
    }

    if (auto window = appDelegate->window) {
        window->setRootViewController(nullptr);
    }

    appDelegate->window = nullptr;
    UIApplication::shared->delegate = nullptr;
    UIApplication::shared->keyWindow.reset();
    UIApplication::shared = nullptr;
    SkiaCtx::_main = nullptr;
    
    return 0;
}

}
