#include <SDL.h>

class Application {
public:
    Application();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    bool runLoop();
    void render();
};