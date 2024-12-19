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


    // Move to UIRenderer
    auto keyWindow = UIApplication::shared->keyWindow.lock();

    auto scale = SkiaCtx::_main->getScaleFactor();

    if (keyWindow->_traitCollection == nullptr
        || keyWindow->_traitCollection->_userInterfaceStyle != SkiaCtx::main()->getThemeMode()
        || keyWindow->_traitCollection->_displayScale != scale)
    {
        auto oldCollection = keyWindow->_traitCollection;
        keyWindow->_traitCollection = new_shared<UITraitCollection>();
        keyWindow->_traitCollection->_displayScale = scale;
        keyWindow->_traitCollection->_userInterfaceStyle = SkiaCtx::main()->getThemeMode();
        keyWindow->traitCollectionDidChange(oldCollection);
    }

    UIView::animateIfNeeded(currentTime);
    keyWindow->drawAndLayoutTreeIfNeeded();

    auto surface = SkiaCtx::_main->getBackbufferSurface();
    if (!surface) return true;

    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    canvas->save();
    canvas->scale(scale, scale);

    UIView::animate(0.3, [keyWindow]() {
        keyWindow->setFrame({ NXPoint::zero, SkiaCtx::_main->getSize() } );
    });

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
