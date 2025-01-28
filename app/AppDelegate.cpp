#include <UIApplicationDelegate.h>
#include <UIImageView.h>
#include <UILabel.h>
#include <DispatchQueue.h>
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

//    auto vc = new_shared<TestViewController>();
    auto vc = new_shared<IBTestController>();
//    auto vc = new_shared<YogaTestViewController>();
    // auto vc = new_shared<NavigationViewController>();
//    auto vc = new_shared<TextViewController>();
    window->setRootViewController(vc);
    window->makeKeyAndVisible();
//    window->setBackgroundColor(UIColor::systemBackground);
    window->setBackgroundColor(UIColor::secondarySystemBackground);

    return true;
}

}
