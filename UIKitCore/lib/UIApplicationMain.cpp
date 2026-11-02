#include <UIApplicationMain.h>
#include <UIApplication.h>
#include <Timer.h>
// #include <UIRenderer.h>
#include <tools/SharedBase.hpp>
#include <DispatchQueue.h>
#include <SkiaCtx.h>
#include "RenderPerformance.hpp"

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
    {
        Internal::RenderPerformanceScope timer(Internal::RenderPerformance::current ? &Internal::RenderPerformance::current->layoutMs : nullptr);
        keyWindow->drawAndLayoutTreeIfNeeded();
    }

    // Resolve pacing after callbacks and animations. Rendering is continuous,
    // including when layout and the retained layer tree have not changed.
    SkiaCtx::_main->setTargetFrameRate(UIView::maximumAnimationFrameRate());

    const auto surface = SkiaCtx::_main->getBackbufferSurface();
    if (!surface) {
        CALayer::layerTreeIsDirty = true;
        SkiaCtx::_main->sleepForNextFrame();
        return true;
    }

    CALayer::layerTreeIsDirty = false;

    const auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    canvas->save();
    canvas->scale(scale, scale);

    UITraitCollection::setCurrent(keyWindow->traitCollection());
    {
        Internal::RenderPerformanceScope timer(Internal::RenderPerformance::current ? &Internal::RenderPerformance::current->renderMs : nullptr);
        keyWindow->layer()->presentationOrSelf()->skiaRender(canvas);
    }
    canvas->restore();

    {
        Internal::RenderPerformanceScope timer(Internal::RenderPerformance::current ? &Internal::RenderPerformance::current->submitMs : nullptr);
        SkiaCtx::_main->flushAndSubmit(surface);
        SkiaCtx::_main->swapBuffers();
    }
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
