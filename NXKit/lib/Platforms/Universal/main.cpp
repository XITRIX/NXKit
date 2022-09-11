//
//  main2..cpp
//  NXKit
//
//  Created by Даниил Виноградов on 06.07.2022.
//

#include <NXKit.hpp>
#include <Platforms/Universal/GLFWVideoContext.hpp>
#include <Platforms/Universal/GLFWInputManager.hpp>

#ifdef __SWITCH__
#include <Platforms/Switch/SwitchVideoContext.hpp>
#include <Platforms/Switch/SwitchInputManager.hpp>
#endif

constexpr uint32_t ORIGINAL_WINDOW_WIDTH  = 1280;
constexpr uint32_t ORIGINAL_WINDOW_HEIGHT = 720;

#ifdef __SWITCH__
#include <switch.h>
#endif

int main(int argc, const char * argv[]) {
    Application* application = new Application();

#ifdef __SWITCH__
    auto videoContext = new SwitchVideoContext("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
    application->setVideoContext(videoContext);

    InputManager::initWith(new SwitchInputManager());
    application->setResourcesPath("romfs:/");
#else
    auto videoContext = new GLFWVideoContext("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
    application->setVideoContext(videoContext);

    InputManager::initWith(new GLFWInputManager());
#endif

    application->getDelegate()->applicationDidFinishLaunching();

    while (application->mainLoop());

    delete application;
    return 0;
}
