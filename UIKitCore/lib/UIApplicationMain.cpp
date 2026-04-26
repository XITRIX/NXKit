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
    UIApplication::shared->handleEventsIfNeeded();
    if (UIApplication::shared->isQuitRequested()) {
        return false;
    }
    DispatchQueue::main()->performAll(); // TODO: May be need to be after rendering loop
//        UIRenderer::main()->render(UIApplication::shared->keyWindow.lock(), currentTime);


    // Move to UIRenderer
    const auto keyWindow = UIApplication::shared->keyWindow.lock();
    NXSize size = SkiaCtx::_main->getSize();
    keyWindow->setFrame({ NXPoint::zero, size });

    const auto scale = SkiaCtx::_main->getScaleFactor();

    if (keyWindow->_traitCollection == nullptr
        || keyWindow->_traitCollection->_userInterfaceStyle != SkiaCtx::main()->getThemeMode()
        || keyWindow->_traitCollection->_displayScale != scale)
    {
        const auto oldCollection = keyWindow->_traitCollection;
        keyWindow->_traitCollection = new_shared<UITraitCollection>();
        keyWindow->_traitCollection->_displayScale = scale;
        keyWindow->_traitCollection->_userInterfaceStyle = SkiaCtx::main()->getThemeMode();
        keyWindow->traitCollectionDidChange(oldCollection);
        CALayer::setLayerTreeIsDirty();
    }

    const int maximumAnimationFrameRate = UIView::maximumAnimationFrameRate();
    CALayer::maxFramerateRequired = 0; // reset value

    UIView::animateIfNeeded(currentTime);
    keyWindow->drawAndLayoutTreeIfNeeded();

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

    // TODO: Get target frame rate from UI context
    SkiaCtx::_main->setTargetFrameRate(maximumAnimationFrameRate);
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

#ifdef PLATFORM_IOS
    SkiaCtx::_main->setExtraScaleFactor(1.3f);
#endif

    UIApplication::shared->delegate = appDelegate;

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
