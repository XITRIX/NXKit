//
// Created by Даниил Виноградов on 17.11.2024.
//

#pragma once

#include <SDL.h>
#include <thorvg.h>

class Application {
public:
    Application();

private:
    const int FPS = 120;
    static Application* shared;

    int width, height = 0;

    SDL_Window *window = nullptr;
    tvg::GlCanvas* canvas = nullptr;

    void initVideo();
    void render();
    void resize();
    void draw();

    static int resizingEventWatcher(void* data, SDL_Event* event);
};
