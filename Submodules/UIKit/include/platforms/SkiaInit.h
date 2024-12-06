#include "tools/window/WindowContext.h"

#include <functional>
#include <SDL.h>

std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window);

bool platformRunLoop(const std::function<bool()>& runLoop);
