#include "Application.h"



Application::Application() {
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Window", 100, 100, 1280, 720, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SkGraphics::Init();

    while(runLoop());
}

bool Application::runLoop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    render();
    return true;
}

void Application::render() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}