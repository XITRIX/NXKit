#pragma once

#ifdef __SWITCH__
#include <UIViewController.h>
#include <functional>

namespace NXKit {
bool switchShaderBakeRequested();
void requestSwitchShaderBake();
std::shared_ptr<UIViewController> makeSwitchShaderBakeController();
void requestSwitchExitTest();
void installSwitchExitTest(std::function<void(size_t)> selectScreen);
}
#endif
