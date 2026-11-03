#include <Application.h>
#include <SDL3/SDL_main.h>
#include <UIApplicationMain.h>
#include "SwitchShaderBake.hpp"
#include <string_view>
#include <cstdlib>

int main(int argc, char *argv[]) {
#ifdef __SWITCH__
    ::unsetenv("NXKIT_SWITCH_BUNDLED_CACHE_ONLY");
    for (int i = 1; i < argc; ++i)
        if (std::string_view(argv[i]) == "--nxkit-shader-bake") NXKit::requestSwitchShaderBake();
        else if (std::string_view(argv[i]) == "--nxkit-exit-test") NXKit::requestSwitchExitTest();
        else if (std::string_view(argv[i]) == "--nxkit-bundled-cache-only")
            ::setenv("NXKIT_SWITCH_BUNDLED_CACHE_ONLY", "1", 1);
#endif
    return NXKit::UIApplicationMain();
}
