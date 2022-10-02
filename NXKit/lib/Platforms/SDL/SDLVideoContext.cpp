/*
    Copyright 2021 natinusala

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>

//#include <borealis/core/application.hpp>
//#include <borealis/core/logger.hpp>
#include <Platforms/SDL/SDLVideoContext.hpp>

//#include <SDL2/SDL.h>

// nanovg implementation
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg-gl/nanovg_gl.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include <Core/Application/Application.hpp>

namespace NXKit {

static void glfwWindowFramebufferSizeCallback(SDL_Window* window, int width, int height)
{
    if (!width || !height)
        return;

//    glViewport(0, 0, width, height);


    int wWidth, wHeight;
    int fWidth, fHeight;

    SDL_GetWindowSize(window, &wWidth, &wHeight);
    SDL_GL_GetDrawableSize(window, &fWidth, &fHeight);
    //    glfwGetWindowSize(window, &wWidth, &wHeight);
//    glfwGetFramebufferSize(window, &fWidth, &fHeight);

    Application::shared()->onWindowResized(static_cast<unsigned>(wWidth), static_cast<unsigned>(wHeight), (float)fWidth / (float)wWidth);
}

static int resizingEventWatcher(void* data, SDL_Event* event) {
  if (event->type == SDL_WINDOWEVENT &&
      event->window.event == SDL_WINDOWEVENT_RESIZED) {
    SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
    if (win == (SDL_Window*)data) {
        int width, height;
        SDL_GetWindowSize(win, &width, &height);
        printf("resizing..... %d, %d\n", width, height);
        glfwWindowFramebufferSizeCallback(win, width, height);
    }
  }
  return 0;
}

SDLVideoContext::SDLVideoContext(std::string windowTitle, uint32_t windowWidth, uint32_t windowHeight)
{
//    if (!glfwInit()) { return; }

    // Create window
//#if defined(__APPLE__) || defined(_WIN32)
    // Explicitly ask for a 3.2 context on OS X
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//    glutInitContextVersion(3,2);
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { return; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,         8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,     SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,         1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,         1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,         8);
//#else
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//#endif

    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

//    this->window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);

//    if (!window)
//    {
//        //        Logger::error("glfw: failed to create window");
//        glfwTerminate();
//        return;
//    }

    // Configure window
//    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

//    glfwMakeContextCurrent(window);
//    glfwSetFramebufferSizeCallback(window, glfwWindowFramebufferSizeCallback);
    SDL_AddEventWatch(resizingEventWatcher, window);

    // Load OpenGL routines using glad
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    SDL_GL_SetSwapInterval(1);

    //    Logger::info("glfw: GL Vendor: {}", glGetString(GL_VENDOR));
    //    Logger::info("glfw: GL Renderer: {}", glGetString(GL_RENDERER));
    //    Logger::info("glfw: GL Version: {}", glGetString(GL_VERSION));

    // Initialize nanovg
    this->nvgContext = nvgCreateGL3(NVG_STENCIL_STROKES);
    if (!this->nvgContext)
    {
        //        Logger::error("glfw: unable to init nanovg");
//        glfwTerminate();
        return;
    }

    // Setup scaling
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
//    glfwGetFramebufferSize(window, &width, &height);
    glfwWindowFramebufferSizeCallback(window, width, height);

#ifdef __SWITCH__
    monitor = glfwGetPrimaryMonitor();
#endif
}

float SDLVideoContext::getScaleFactor() {
    int wWidth, wHeight;
    int fWidth, fHeight;

//    glfwGetWindowSize(window, &wWidth, &wHeight);
//    glfwGetFramebufferSize(window, &fWidth, &fHeight);
    SDL_GetWindowSize(window, &wWidth, &wHeight);
    SDL_GL_GetDrawableSize(window, &fWidth, &fHeight);

    return fWidth / wWidth;
}

void SDLVideoContext::beginFrame()
{
#ifdef __SWITCH__
    const GLFWvidmode* return_struct = glfwGetVideoMode(monitor);
    int width = return_struct->width;
    int height = return_struct->height;

    if (oldWidth != width || oldHeight != height) {
        oldWidth = width;
        oldHeight = height;

        glfwSetWindowSize(window, width, height);
    }
#endif
}

void SDLVideoContext::endFrame()
{
//    glfwSwapBuffers(this->window);
    SDL_GL_SwapWindow(this->window);
}

void SDLVideoContext::clear(NVGcolor color)
{
    glClearColor(
                 color.r,
                 color.g,
                 color.b,
                 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SDLVideoContext::resetState()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
}

bool SDLVideoContext::mainLoopInteraction() {
//    bool isActive;
//    do {
//        isActive = !glfwGetWindowAttrib(getGLFWWindow(), GLFW_ICONIFIED);
////        SDL_Event
//
//        if (isActive)
//            glfwPollEvents();
//        else
//            glfwWaitEvents();
//    } while (!isActive);

    bool platform = true;

#ifdef __SWITCH__
    platform = appletMainLoop();
#endif

    SDL_Event event;
    bool running = true;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT){
            running = false;
        }
    }

    return running || !platform;
}

void SDLVideoContext::disableScreenDimming(bool disable)
{
#ifdef __SWITCH__
    appletSetMediaPlaybackState(disable);
#endif
}

static void flipHorizontal(unsigned char* image, int w, int h, int stride) {
    int i = 0, j = h-1, k;
    while (i < j) {
        unsigned char* ri = &image[i * stride];
        unsigned char* rj = &image[j * stride];
        for (k = 0; k < w*4; k++) {
            unsigned char t = ri[k];
            ri[k] = rj[k];
            rj[k] = t;
        }
        i++;
        j--;
    }
}

void SDLVideoContext::getContextPixels(int x, int y, int w, int h, unsigned char** pixels)
{
    Application::shared()->flushContext();

    *pixels = (unsigned char*) malloc(static_cast<size_t>(w * h * 4));

    glReadPixels(x, y - h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
    flipHorizontal(*pixels, w, h, w*4);
}

SDLVideoContext::~SDLVideoContext()
{
    if (this->nvgContext)
        nvgDeleteGL3(this->nvgContext);

//    glfwDestroyWindow(this->window);
//    SDL_GL
//    glfwTerminate();
}

NVGcontext* SDLVideoContext::getNVGContext()
{
    return this->nvgContext;
}

SDL_Window* SDLVideoContext::getGLFWWindow()
{
    return this->window;
}

UIUserInterfaceStyle SDLVideoContext::getUserInterfaceStyle() {
    return UIUserInterfaceStyle::dark;
}

}
