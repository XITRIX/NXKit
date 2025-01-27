#pragma once

#include <UIWindow.h>

#include <map>
#include <any>

#define REGISTER_NIB(n) UINib::registerXIB(#n, n::init);

namespace NXKit {

class UIApplication;
class UIApplicationDelegate {
public:
    std::shared_ptr<UIWindow> window;

    virtual bool applicationDidFinishLaunchingWithOptions(UIApplication* application, std::map<std::string, std::any> launchOptions);
    virtual void applicationWillTerminate(UIApplication* application) {}

    virtual void applicationWillEnterForeground(UIApplication* application) {}
    virtual void applicationDidBecomeActive(UIApplication* application) {}

    virtual void applicationWillResignActive(UIApplication* application) {}
    virtual void applicationDidEnterBackground(UIApplication* application) {}

     virtual void applicationNeedsXIBRegistration(UIApplication* application);
};

}
