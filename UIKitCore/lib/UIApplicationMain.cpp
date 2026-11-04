#include <UIApplicationMain.h>
#include <UIApplication.h>
#include <Timer.h>
// #include <UIRenderer.h>
#include <tools/SharedBase.hpp>
#include <DispatchQueue.h>
#include <SkiaCtx.h>
#include "RenderPerformance.hpp"
#include <cstdlib>
#include <string_view>

namespace NXKit {

// Shared by the outer loop and native live-resize exposes. This path never
// polls events, runs dispatch callbacks or sleeps, avoiding recursive pumps.
void renderApplicationFrame() {
    static thread_local bool rendering = false;
    if (rendering || !UIApplication::shared || UIApplication::shared->isQuitRequested() || !SkiaCtx::_main) return;
    const auto keyWindow = UIApplication::shared->keyWindow.lock();
    if (!keyWindow) return;
    rendering = true;
    struct Restore { bool& value; ~Restore() { value = false; } } restore{rendering};
    SkiaCtx::_main->prepareFrame();
    // Move to UIRenderer
    NXSize size = SkiaCtx::_main->getSize();
    keyWindow->setFrame({ NXPoint::zero, size });

    const auto scale = SkiaCtx::_main->getScaleFactor();
    // Poll even when geometry is unchanged: UIKit can change size classes at
    // the same bounds after rotation, multitasking, or an environment change.
    keyWindow->updateTraitsFromPlatform();

    const Timer frameTime;
    UIView::animateIfNeeded(frameTime);
    {
        Internal::RenderPerformanceScope timer(Internal::RenderPerformance::current ? &Internal::RenderPerformance::current->layoutMs : nullptr);
        keyWindow->layoutTreeIfNeeded(false);
        // Layout can resize/reposition an animating layer after its first
        // presentation snapshot. Refresh with the same timestamp so properties
        // not animated by that layer do not retain the previous frame's size.
        UIView::animateIfNeeded(frameTime);
        keyWindow->displayTreeIfNeeded();
    }

    // Polling, layout and animation delivery still run while idle. Avoid
    // acquiring a drawable or submitting GPU work when the retained tree is
    // unchanged; the platform keeps displaying its last presented frame.
    SkiaCtx::_main->setTargetFrameRate(UIView::maximumAnimationFrameRate());
    static const bool continuous = [] {
        const auto enabled = [](const char* name) {
            const auto value = std::getenv(name);
            return value && std::string_view(value) == "1";
        };
        return enabled("NXKIT_CONTINUOUS_RENDERING") || enabled("NXKIT_UNCAPPED");
    }();
    if (!continuous && !CALayer::layerTreeIsDirty) return;

    const auto surface = SkiaCtx::_main->getBackbufferSurface();
    if (!surface) {
        CALayer::layerTreeIsDirty = true;
        return;
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
}

bool applicationRunLoop() {
    CALayer::maxFramerateRequired = 0;
    if (!SkiaCtx::_main->_immediateResizeFrame) {
        SkiaCtx::_main->_immediateResizeFrame = [] {
            CALayer::layerTreeIsDirty = true;
            renderApplicationFrame();
        };
    }
    UIApplication::shared->handleEventsIfNeeded();
    if (UIApplication::shared->isQuitRequested()) return false;
    SkiaCtx::_main->pollBackdropReads();
    DispatchQueue::main()->performAll();
    renderApplicationFrame();
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
