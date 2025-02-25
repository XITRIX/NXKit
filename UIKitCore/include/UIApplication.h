#pragma once

#include <SDL_events.h>

#include <UIApplicationDelegate.h>
#include <UIWindow.h>

namespace NXKit {

struct UIGamepadKey;
enum class UIPressType;
enum class UIGamepadInputType;

class UIApplication {
public:
    static std::shared_ptr<UIApplication> shared;

    UIApplication();

    std::shared_ptr<UIApplicationDelegate> delegate;
    std::weak_ptr<UIWindow> keyWindow;

    void handleEventsIfNeeded();
    void handleSDLQuit();

    void sendEvent(const std::shared_ptr<UIEvent>& event) const;

// TODO: Need to remove
//    static GPU_Target* currentRenderer;
    
private:
    static void onWillEnterForeground();
    static void onDidEnterForeground();
    static void onWillEnterBackground();
    static void onDidEnterBackground();

    void handleSDLEvent(SDL_Event e);

    static UIGamepadKey mapControllerButtonEventToUIGamepadKey(SDL_ControllerButtonEvent event);
    static std::optional<UIGamepadKey> mapControllerAxisEventToUIGamepadKey(SDL_ControllerAxisEvent event);
    static UIPressType mapGamepadInputToUIPressType(UIGamepadInputType key);
};

}
