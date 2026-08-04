#pragma once

#include <SDL3/SDL_events.h>

#include <Timer.h>
#include <UIApplicationDelegate.h>
#include <UIWindow.h>

#include <map>
#include <utility>

namespace NXKit {

struct UIGamepadKey;
class UIPressesEvent;
enum class UIPressPhase;
enum class UIPressType;
enum class UIGamepadInputType;

class UIApplication {
public:
    static std::shared_ptr<UIApplication> shared;

    UIApplication();
    ~UIApplication();

    std::shared_ptr<UIApplicationDelegate> delegate;
    std::weak_ptr<UIWindow> keyWindow;

    void startHandlingLifecycleEvents();
    void handleEventsIfNeeded();
    void handleSDLQuit();
    bool isQuitRequested() const { return quitRequested; }

    void sendEvent(const std::shared_ptr<UIEvent>& event) const;

// TODO: Need to remove
//    static GPU_Target* currentRenderer;
    
private:
    static void onWillEnterForeground();
    static void onDidEnterForeground();
    static void onWillEnterBackground();
    static void onDidEnterBackground();

    static bool SDLCALL handleSDLLifecycleEvent(void* userdata, SDL_Event* event);
    void handleSDLEvent(SDL_Event e);

    static UIGamepadKey mapGamepadButtonEventToUIGamepadKey(SDL_GamepadButtonEvent event);
    static std::optional<UIGamepadKey> mapGamepadAxisEventToUIGamepadKey(SDL_GamepadAxisEvent event);
    static UIPressType mapGamepadInputToUIPressType(UIGamepadInputType key);
    void sendPressRepeatsIfNeeded(const Timer& timestamp);
    void finishActivePressEvent(
        const std::shared_ptr<UIPressesEvent>& event,
        UIPressPhase phase
    );
    void cancelGamepadPresses(SDL_JoystickID gamepadID);
    void cancelAllActivePresses();

    using GamepadControlKey = std::pair<SDL_JoystickID, Uint8>;
    std::map<GamepadControlKey, std::shared_ptr<UIPressesEvent>>
        _gamepadButtonPresses;
    std::map<GamepadControlKey, std::shared_ptr<UIPressesEvent>>
        _gamepadAxisPresses;

    bool lifecycleEventWatchInstalled = false;
    bool quitRequested = false;

    friend class UIApplicationPressRepeatTestHarness;
};

}
