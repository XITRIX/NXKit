#include <UIApplication.h>
#include <UIPress.h>
#include <UIPressesEvent.h>
#include <UIViewController.h>
#include <SDL3/SDL.h>
#include <iostream>

using namespace NXKit;

namespace NXKit {
bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(
    UIApplication*, std::map<std::string, std::any>) { return true; }

class UIApplicationPressRepeatTestHarness {
public:
    static void send(const std::shared_ptr<UIApplication>& app, SDL_Event event) {
        app->handleSDLEvent(event);
    }
    static bool empty(const std::shared_ptr<UIApplication>& app) {
        return app->_gamepadButtonPresses.empty() && app->_gamepadAxisPresses.empty()
            && UIPressesEvent::activePressesEvents.empty();
    }
};
}

namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
struct Probe final : UIView {
    std::function<void()> ended, cancelled;
    int endCount = 0, cancelCount = 0;
    void pressesEnded(std::set<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) override {
        ++endCount;
        if (ended) ended();
    }
    void pressesCancelled(std::set<std::shared_ptr<UIPress>>, std::shared_ptr<UIPressesEvent>) override {
        ++cancelCount;
        if (cancelled) cancelled();
    }
};
struct Fixture {
    std::shared_ptr<UIApplication> app = std::make_shared<UIApplication>();
    std::shared_ptr<UIApplicationDelegate> delegate = std::make_shared<UIApplicationDelegate>();
    std::shared_ptr<Probe> probe = new_shared<Probe>();
    Fixture() {
        UIApplication::shared = app;
        app->delegate = delegate;
        delegate->window = new_shared<UIWindow>();
        auto controller = new_shared<UIViewController>();
        controller->setView(probe);
        delegate->window->setRootViewController(controller);
        delegate->window->addSubview(probe);
        app->keyWindow = delegate->window;
    }
    ~Fixture() {
        probe->ended = {}; probe->cancelled = {};
        app->handleSDLQuit();
        app->delegate.reset();
        UIApplication::shared.reset();
    }
    void send(SDL_Event event) { UIApplicationPressRepeatTestHarness::send(app, event); }
    void button(bool down, Uint8 button = SDL_GAMEPAD_BUTTON_EAST) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
        event.gbutton.which = 42; event.gbutton.button = button; event.gbutton.down = down;
        send(event);
    }
    void axis(Sint16 value) {
        SDL_Event event{}; event.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
        event.gaxis.which = 42; event.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTX; event.gaxis.value = value;
        send(event);
    }
};
}

int main() {
    SDL_InitSubSystem(SDL_INIT_EVENTS);
    {
        Fixture f;
        f.probe->ended = [&] { f.app->handleSDLQuit(); };
        f.button(true); f.button(false);
        expect(f.app->isQuitRequested(), "button release invokes the quit callback");
        expect(f.probe->endCount == 1 && f.probe->cancelCount == 0,
               "a completed release is never redelivered during quit");
        expect(UIApplicationPressRepeatTestHarness::empty(f.app), "quit drains all press sources");
    }
    for (const Sint16 value : {Sint16(0), Sint16(-32000)}) {
        Fixture f;
        f.probe->ended = [&] { f.app->handleSDLQuit(); };
        f.axis(32000); f.axis(value);
        expect(f.probe->endCount == 1 && f.probe->cancelCount == 0,
               "axis release/reversal can quit without duplicate delivery");
        expect(UIApplicationPressRepeatTestHarness::empty(f.app),
               "axis reversal cannot begin a new press after quit");
    }
    {
        Fixture f;
        f.probe->ended = [&] {
            if (f.probe->endCount == 1) f.button(true);
        };
        f.button(true); f.button(false);
        expect(!UIApplicationPressRepeatTestHarness::empty(f.app),
               "a release callback can replace the same physical press source");
        f.button(false);
        expect(f.probe->endCount == 2 && UIApplicationPressRepeatTestHarness::empty(f.app),
               "the replacement remains registered until its own release");
    }
    {
        Fixture f;
        f.probe->ended = [&] { f.app->handleSDLQuit(); };
        SDL_Event event{}; event.type = SDL_EVENT_KEY_DOWN;
        event.key.scancode = SDL_SCANCODE_ESCAPE; event.key.key = SDLK_ESCAPE;
        f.send(event);
        event.type = SDL_EVENT_KEY_UP; f.send(event);
        expect(f.probe->endCount == 1 && f.probe->cancelCount == 0,
               "a keyboard release can quit without duplicate delivery");
        expect(UIApplicationPressRepeatTestHarness::empty(f.app), "keyboard quit drains press sources");
    }
    {
        Fixture f;
        f.probe->cancelled = [&] { f.app->handleSDLQuit(); };
        f.button(true); f.axis(32000);
        SDL_Event event{}; event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
        f.send(event);
        expect(f.probe->cancelCount == 2 && f.probe->endCount == 0,
               "reentrant quit from cancellation delivers each held press once");
        expect(UIApplicationPressRepeatTestHarness::empty(f.app), "cancellation drains press sources");
    }
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    if (!failures) std::cout << "Press release and reentrant shutdown tests passed\n";
    return failures ? 1 : 0;
}
