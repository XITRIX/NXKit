#include <SDL.h>
#include "include/gpu/ganesh/GrDirectContext.h"
#import "tools/window/WindowContext.h"

class Application {
public:
    Application();
    ~Application();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    sk_sp<GrDirectContext> ctx;
    std::unique_ptr<skwindow::WindowContext> skiaWindow;

    static Application* shared;
    static int resizingEventWatcher(void* data, SDL_Event* event);

    bool runLoop();
    void render();
};
