#include <platforms/SkiaCtx_sdlBase.h>
#include <UIApplication.h>
#include <UIApplicationMain.h>
#include <UIViewController.h>
#include <CADisplayLink.h>
#include <SDL3/SDL.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace NXKit;
namespace NXKit {
bool applicationRunLoop();
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication*,std::map<std::string,std::any>) { return true; }
}
namespace {
int failures=0;
void expect(bool condition,const char* message) {
    if (!condition) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}
}
class Context final : public SkiaCtx_sdlBase {
public:
    int presents=0;
    sk_sp<SkSurface> getBackbufferSurface() override {
        const auto size=getSize(); const auto scale=getScaleFactor();
        return SkSurfaces::Null(int(size.width*scale),int(size.height*scale));
    }
    void flushAndSubmit(sk_sp<SkSurface>) override {}
    void swapBuffers() override {++presents;}
    void sleepForNextFrame() override {}
    bool platformRunLoop(std::function<bool()>) override {return false;}
    bool usable() const {return window!=nullptr;}
    void hide() {SDL_HideWindow(window);}
    void resize(int w,int h) {SDL_SetWindowSize(window,w,h);SDL_SyncWindow(window);}
    void expose() {
        SDL_Event event{};event.type=SDL_EVENT_WINDOW_EXPOSED;
        event.window.windowID=SDL_GetWindowID(window);event.window.data1=1;
        SDL_PushEvent(&event);
    }
    void nativeWait() {currentRunLoopStartTimer=Timer();setTargetFrameRate(1);SkiaCtx_sdlBase::sleepForNextFrame();}
protected:
    skgpu::graphite::Context* graphiteContext() override {return nullptr;}
    skgpu::graphite::Recorder* graphiteRecorder() override {return nullptr;}
};
}
int main() {
    auto context=std::make_shared<Context>();
    if (!context->usable()) {std::cerr<<SDL_GetError()<<'\n';return 2;}
    SkiaCtx::_main=context;
    UIApplication::shared=new_shared<UIApplication>();
    UIApplication::shared->delegate=std::make_shared<UIApplicationDelegate>();
    auto window=new_shared<UIWindow>();window->setRootViewController(new_shared<UIViewController>());window->makeKeyAndVisible();
    int displayLinks=0;
    CADisplayLink link([&]{++displayLinks;});
    applicationRunLoop();
    context->resize(503,347);
    const auto before=context->presents, callbacks=displayLinks;
    context->expose();
    expect(context->presents==before+1,"main-thread live expose renders before returning to caller");
    expect(window->bounds().size==NXSize(503,347),"live expose uses the new desktop size immediately");
    expect(displayLinks==callbacks,"live expose does not recursively run dispatch callbacks");
    const auto rendered=context->presents;
    std::thread synthetic([&]{context->expose();});synthetic.join();
    expect(context->presents==rendered,"worker-thread synthetic exposes cannot mutate UI");
    context->hide();SDL_PumpEvents();SDL_Delay(20);SDL_PumpEvents();
    SDL_FlushEvents(SDL_EVENT_FIRST,SDL_EVENT_LAST);
    // Pacing must wake for input, while leaving that input queued for delivery.
    std::thread input([]{SDL_Delay(25);SDL_Event event{};event.type=SDL_EVENT_USER;event.user.code=921;SDL_PushEvent(&event);});
    const auto start=std::chrono::steady_clock::now();context->nativeWait();
    const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    input.join();
    SDL_Event event{};bool retained=false;
    while(SDL_PollEvent(&event)) if(event.type==SDL_EVENT_USER && event.user.code==921) retained=true;
    expect(elapsed>=15 && elapsed<250,"input interrupts a one-second frame-pacing wait");
    expect(retained,"frame-pacing wait preserves the waking event");
    link.invalidate();window->setRootViewController(nullptr);UIApplication::shared.reset();SkiaCtx::_main.reset();context.reset();
    if (!failures) std::cout<<"Desktop resize/expose/wake tests passed\n";
    return failures?1:0;
}
