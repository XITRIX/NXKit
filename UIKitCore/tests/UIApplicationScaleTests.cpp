#include <UIApplicationMain.h>
#include <SkiaCtx.h>
#include <UIViewController.h>
#include <UIEvent.h>
#include <UITouch.h>
#include <CADisplayLink.h>
#include <SDL3/SDL.h>
#include "include/core/SkBitmap.h"
#include <cmath>
#include <iostream>

using namespace NXKit;

namespace {
int failures = 0;
NXSize nativeSize{430, 932};
float nativeScale = 3;
UIEdgeInsets nativeSafe{59, 0, 34, 0};
void expect(bool result, const char* message) {
    if (!result) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

// Exercise real UIApplicationMain, including its iOS startup branch, without
// requiring a GPU window. Match the iOS backend's optional coordinate scaling.
class Context final : public SkiaCtx {
public:
    int requestedRate=0,presents=0,waits=0,preparations=0;
    bool exposeDuringAcquire=false;
    void expose() { renderImmediatelyForResize(); }
    void prepareFrame() override { ++preparations; }
    void setTargetFrameRate(int rate) override {requestedRate=rate;}
    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
        std::lround(nativeSize.width * nativeScale), std::lround(nativeSize.height * nativeScale)));
    NXSize getSize() override { return nativeSize * _extraScaleFactor; }
    float getScaleFactor() override { return nativeScale / _extraScaleFactor; }
    UIEdgeInsets deviceSafeAreaInsets() override { return nativeSafe; }
    sk_sp<SkSurface> getBackbufferSurface() override {
        if (surface->width()!=std::lround(nativeSize.width*nativeScale) || surface->height()!=std::lround(nativeSize.height*nativeScale))
            surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(std::lround(nativeSize.width*nativeScale),std::lround(nativeSize.height*nativeScale)));
        if (exposeDuringAcquire) { exposeDuringAcquire=false; expose(); }
        return surface;
    }
    void flushAndSubmit(sk_sp<SkSurface>) override {}
    void swapBuffers() override {++presents;}
    void sleepForNextFrame() override {++waits;}
    bool platformRunLoop(std::function<bool()> loop) override;
protected:
    skgpu::graphite::Context* graphiteContext() override { return nullptr; }
    skgpu::graphite::Recorder* graphiteRecorder() override { return nullptr; }
};

class TouchView final : public UIView {
public:
    NXPoint lastTouch{-1, -1};
    void touchesBegan(std::set<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override {
        lastTouch = (*touches.begin())->locationIn(shared_from_this());
    }
};

bool Context::platformRunLoop(std::function<bool()> loop) {
    expect(loop(), "first application frame runs");
    const auto painted=presents;
    {
        CADisplayLink fast([] {CALayer::requestFramerate(120);});
        expect(loop()&&requestedRate==120,"current display-link requests control frame pacing even with a clean tree");
        fast.invalidate();
    }
    expect(loop()&&requestedRate==30,"idle pacing returns to 30 Hz after interaction stops");
    expect(presents==painted+2,"clean frames continue acquiring/presenting framebuffers");
    const auto window = UIApplication::shared->keyWindow.lock();
    expect(window->bounds().size == nativeSize, "startup preserves native window points");
    expect(window->traitCollection()->displayScale() == nativeScale, "window retains native display scale");
    expect(window->safeAreaInsets() == nativeSafe, "safe areas share the window point space");

    // Check actual run-loop output: a 44-point view must occupy 132 pixels at
    // 3x and 88 pixels at 2x, without changing its point-sized hit geometry.
    SkBitmap bitmap;
    bitmap.allocN32Pixels(surface->width(), surface->height());
    expect(surface->readPixels(bitmap, 0, 0), "read rendered application frame");
    const int edge = std::lround(44 * nativeScale);
    expect(bitmap.getColor(edge - 2, edge / 2) == SK_ColorRED,
           "44-point control fills its native pixel extent");
    expect(bitmap.getColor(edge + 2, edge / 2) == SK_ColorBLACK,
           "display scale is applied exactly once");

    SDL_Event event{};
    event.type = SDL_EVENT_FINGER_DOWN;
    event.tfinger.fingerID = 912;
    event.tfinger.x = .5f;
    event.tfinger.y = .5f;
    SDL_PushEvent(&event);
    expect(loop(), "touch frame runs");
    auto view = std::static_pointer_cast<TouchView>(window->rootViewController()->view());
    expect(view->lastTouch == NXPoint(nativeSize.width / 2, nativeSize.height / 2),
           "normalized SDL touch resolves in native window points");
    event.type = SDL_EVENT_FINGER_UP;
    SDL_PushEvent(&event);
    loop();

    const auto oldSize = nativeSize;
    nativeSize = {oldSize.width+37,oldSize.height+41};
    const auto beforeResize = presents, beforeWaits = waits, beforePreparations = preparations;
    exposeDuringAcquire = true;
    expose();
    expect(window->bounds().size==nativeSize && view->bounds().size==nativeSize,
           "resize callback updates window and root geometry immediately");
    expect(presents==beforeResize+1 && waits==beforeWaits,
           "resize callback presents without pacing sleep or recursive acquisition");
    expect(preparations==beforePreparations+1,"one geometry snapshot prepares the resize frame");
    nativeSize=oldSize; expose();
    return false;
}
}

namespace NXKit {
// Renamed only for this test target so the startup path uses our raster context.
std::unique_ptr<SkiaCtx> MakeStartupScaleTestContext() { return std::make_unique<Context>(); }
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*, std::map<std::string, std::any>) {
    window = new_shared<UIWindow>();
    window->setBackgroundColor(UIColor::black);
    auto controller = new_shared<UIViewController>();
    auto view = new_shared<TouchView>();
    auto control = new_shared<UIView>(NXRect(0, 0, 44, 44));
    control->setBackgroundColor(UIColor::red);
    view->addSubview(control);
    controller->setView(view);
    window->setRootViewController(controller);
    window->makeKeyAndVisible();
    return true;
}
}

int main() {
    SDL_InitSubSystem(SDL_INIT_EVENTS);
    for (const auto scale : {2.0f, 3.0f}) {
        nativeScale = scale;
        for (const bool landscape : {false, true}) {
            nativeSize = landscape ? NXSize{932, 430} : NXSize{430, 932};
            nativeSafe = landscape ? UIEdgeInsets{0, 59, 21, 59} : UIEdgeInsets{59, 0, 34, 0};
            expect(UIApplicationMain(std::make_shared<UIApplicationDelegate>()) == 0, "application exits cleanly");
        }
    }
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    if (failures) return 1;
    std::cout << "Application display-scale tests passed\n";
}
