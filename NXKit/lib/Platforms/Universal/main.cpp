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

using namespace NXKit;

int main(int argc, const char * argv[]) {
    std::shared_ptr<Application> application = NXKit::make_shared<Application>();

#ifdef __SWITCH__
    auto videoContext = new SwitchVideoContext("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
    application->setVideoContext(videoContext);

    InputManager::initWith(new SwitchInputManager());
    application->setResourcesPath("romfs:/");
#else
    auto videoContext = NXKit::make_shared<GLFWVideoContext>("Title", ORIGINAL_WINDOW_WIDTH, ORIGINAL_WINDOW_HEIGHT);
    application->setVideoContext(videoContext);

    InputManager::initWith(new GLFWInputManager());
#endif

    application->getDelegate()->applicationDidFinishLaunching();

    while (application->mainLoop());

//    application->free();
    application = nullptr;
    videoContext = nullptr;
    
    return 0;
}
