#include <NXKit.h>
#include <Screens/TestViewController/TestViewController.hpp>
#include <Screens/IBTestController/IBTestController.h>
#include <Screens/YogaTestViewController/YogaTestViewController.hpp>
// #include <NavigationViewController/NavigationViewController.hpp>
// #include <TextViewController/TextViewController.hpp>
#include <romfs/romfs.hpp>

namespace NXKit {

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication* application, std::map<std::string, std::any> launchOptions) {
    DEFAULT_ROMFS_REGISTRATION
    window = new_shared<UIWindow>();

    auto vc = new_shared<IBTestController>();
    auto vc1 = new_shared<TestViewController>();
    auto vc2 = new_shared<YogaTestViewController>();
//    auto vc3 = new_shared<NavigationViewController>();
//    auto vc4 = new_shared<TextViewController>();
    std::vector<std::shared_ptr<UIViewController>> vcs1 = { vc, vc1 };
    std::vector<std::shared_ptr<UIViewController>> vcs2 = { vc2 };
    std::vector<std::vector<std::shared_ptr<UIViewController>>> vcs;

    vcs.push_back(vcs1);
    vcs.push_back(vcs2);

    auto tbvc = new_shared<NXTabBarController>(vcs);
    auto nvc = new_shared<NXNavigationController>(tbvc);
    window->setRootViewController(nvc);
    window->makeKeyAndVisible();
//    window->setBackgroundColor(UIColor::systemBackground);
    window->setBackgroundColor(UIColor::secondarySystemBackground);
    window->setTintColor(UIColorThemed(UIColor::systemBlue, UIColor::systemTeal, "_tint"));
//    window->setTintColor(UIColorThemed(UIColor::systemTeal, UIColor::systemTeal));

    return true;
}

}
