//
// Created by Даниил Виноградов on 17.11.2024.
//

#include "Application.h"

#include <glad/glad.h>

#define WIDTH 1280
#define HEIGHT 720

Application* Application::shared = nullptr;

int Application::resizingEventWatcher(void* data, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
        if (win == (SDL_Window*)data) {
            printf("resizing.....\n");
            Application::shared->resize();
            Application::shared->render();
        }
    }
    return 0;
}

Application::Application() {
    if (!shared)
        shared = this;

    initVideo();
}

void Application::initVideo() {
    tvg::Initializer::init(0);
    SDL_Init(SDL_INIT_EVERYTHING);

//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
//    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
//    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
//    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    Uint32 windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;// | SDL_WINDOW_OPENGL;

    window = SDL_CreateWindow("SDL2 Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT,
                                          windowFlags);

    SDL_AddEventWatch(resizingEventWatcher, window);

//    SDL_GLContext context = SDL_GL_CreateContext(window);
//    SDL_GL_MakeCurrent(window, context);
//
//    gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
//    SDL_GL_SetSwapInterval(1);
//
//    canvas = tvg::GlCanvas::gen();
    canvas = tvg::SwCanvas::gen();
    resize();

    bool quit = false;
//
    const int frameDelay = 1000 / FPS;

    while (!quit) {
        auto frameStart = SDL_GetTicks();
//
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }
        }

        render();

        auto frameTime = SDL_GetTicks() - frameStart;

        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    tvg::Initializer::term();
}

void Application::render() {
//    resize();
    draw();
//    SDL_GL_SwapWindow(window);
    SDL_UpdateWindowSurface(window);
}

void Application::draw() {
    canvas->clear();

    auto background = tvg::Shape::gen();               //generate a shape
    background->appendRect(0, 0, width, height, 0, 0);  //define it as a rounded rectangle (x, y, w, h, rx, ry)
    background->fill(255, 255, 255);                   //set its color (r, g, b)
    canvas->push(std::move(background));                    //push the rectangle into the canvas

    auto rect = tvg::Shape::gen();               //generate a shape
    rect->appendRect(50, 50, 200, 200, 20, 20);  //define it as a rounded rectangle (x, y, w, h, rx, ry)
    rect->fill(100, 100, 100);                   //set its color (r, g, b)
    canvas->push(std::move(rect));                    //push the rectangle into the canvas

    auto circle = tvg::Shape::gen();             //generate a shape
    circle->appendCircle(400, 400, 100, 100);    //define it as a circle (cx, cy, rx, ry)

    auto fill = tvg::RadialGradient::gen();      //generate a radial gradient
    fill->radial(400, 400, 150, 400, 400, 0);                 //set the radial gradient geometry info (cx, cy, radius)

    tvg::Fill::ColorStop colorStops[2];          //gradient colors
    colorStops[0] = {0.0, 255, 255, 255, 255};   //1st color values (offset, r, g, b, a)
    colorStops[1] = {1.0, 0, 0, 0, 255};         //2nd color values (offset, r, g, b, a)
    fill->colorStops(colorStops, 2);             //set the gradient colors info

    circle->fill(std::move(fill));                    //set the circle fill
    canvas->push(std::move(circle));                  //push the circle into the canvas

    canvas->draw();
    canvas->sync();
}

void Application::resize() {
//    SDL_Get
//    SDL_GL_GetDrawableSize(window, &_width, &_height);

    auto surface = SDL_GetWindowSurface(window);
    if (!surface) return;

    if (width == surface->w && height == surface->h)
        return;

    width = surface->w;
    height = surface->h;

//    width = _width;
//    height = _height;
//
//    canvas->target(0, width, height, tvg::ColorSpace::ABGR8888S);
//    canvas->viewport(0, 0, width, height);
    //Set the canvas target and draw on it.

    canvas->target((uint32_t*)surface->pixels, surface->w, surface->pitch / 4, surface->h, tvg::ColorSpace::ARGB8888);
}
