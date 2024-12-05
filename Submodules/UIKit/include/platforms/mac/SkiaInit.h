#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "tools/window/WindowContext.h"

#include <functional>
#include <SDL.h>

//std::unique_ptr<skwindow::WindowContext> skiaMakeMetalWindow(SDL_Window* window);
std::unique_ptr<skwindow::WindowContext> skiaMakeWindow(SDL_Window* window);

bool platformRunLoop(const std::function<bool()>& runLoop);