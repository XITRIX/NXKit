#pragma once
#include <memory>
#include <UIApplication.h>

namespace NXKit {

class UIApplicationDelegate;
int UIApplicationMain(const std::shared_ptr<UIApplicationDelegate>& appDelegate = new_shared<UIApplicationDelegate>());

}
