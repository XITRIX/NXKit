#pragma once

#include <SDL.h>

#include <UIApplicationDelegate.h>
#include <UIWindow.h>

namespace NXKit {

class UIApplication {
public:
    static std::shared_ptr<UIApplication> shared;

    UIApplication();

    std::shared_ptr<UIApplicationDelegate> delegate;
    std::weak_ptr<UIWindow> keyWindow;

    void handleEventsIfNeeded();
    void handleSDLQuit();

    // void sendEvent(std::shared_ptr<UIEvent> event);

// TODO: Need to remove
//    static GPU_Target* currentRenderer;
    
private:
    static void onWillEnterForeground();
    static void onDidEnterForeground();
    static void onWillEnterBackground();
    static void onDidEnterBackground();

    void handleSDLEvent(SDL_Event e);
};

}