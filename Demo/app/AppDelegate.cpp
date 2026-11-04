#include <NXKit.h>
#include <UIKit.h>
#include <UIApplication.h>
#include <Screens/TestViewController/TestViewController.hpp>
#include <Screens/IBTestController/IBTestController.h>
#include <Screens/YogaTestViewController/YogaTestViewController.hpp>
#include <Screens/TestScrollController/TestScrollController.hpp>
#include <Screens/NavigationTestViewController/NavigationTestViewController.hpp>
#include <Screens/GlassDemoViewController/GlassDemoViewController.hpp>
#include <Screens/GlassDemoViewController/GlassStressView.hpp>
#include <Screens/ControllerTestViewController/ControllerTestViewController.hpp>
#include <Screens/TabBarDemoController/TabBarDemoController.hpp>
// #include <TextViewController/TextViewController.hpp>
#include <romfs/romfs.hpp>
#include <cstdlib>
#include "SwitchShaderBake.hpp"

namespace NXKit {

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication* application, std::map<std::string, std::any> launchOptions) {
    registerUIKitViews();
    DEFAULT_ROMFS_REGISTRATION
    window = new_shared<UIWindow>();
#ifdef __SWITCH__
    if (switchShaderBakeRequested()) {
        window->setRootViewController(makeSwitchShaderBakeController());
        window->makeKeyAndVisible();
        return true;
    }
#endif
    NXResponderAction {
        .button = NXActionButton::b,
        .isEnabled = true,
        .action = UIAction("Exit", [
            weakApplication = std::weak_ptr<UIApplication>(UIApplication::shared)
        ]() {
            if (const auto application = weakApplication.lock()) {
                application->handleSDLQuit();
            }
        }),
        .identifier = "NXKit.application.exit",
        .priority = -100,
    }.registerOn(window);

    // Direct launch for Instruments/Metal captures without navigating the Demo.
    const auto stress = std::getenv("NXKIT_GLASS_STRESS");
    const auto stressReplay = std::getenv("NXKIT_GLASS_STRESS_REPLAY");
    if ((stress && std::string(stress) == "1") || (stressReplay && std::string(stressReplay) == "1")) {
        window->setRootViewController(new_shared<GlassStressViewController>());
        window->makeKeyAndVisible();
        return true;
    }

    auto vc = new_shared<IBTestController>();
    auto vc1 = new_shared<TestViewController>();
    auto vc2 = new_shared<YogaTestViewController>();
    auto vc3 = new_shared<TestScrollController>();
    auto navigationTestController = new_shared<NavigationTestViewController>();
    auto glassDemoController = new_shared<GlassDemoViewController>();
    auto controllerTestController = new_shared<ControllerTestViewController>();
//    auto vc4 = new_shared<TextViewController>();
    std::vector<std::shared_ptr<UIViewController>> vcs1 = {
        vc,
        vc1,
        glassDemoController,
        new_shared<TabBarDemoController>()
    };
    std::vector<std::shared_ptr<UIViewController>> vcs2 = { vc2, vc3 };
    std::vector<std::shared_ptr<UIViewController>> navigationTests = {
        navigationTestController
    };
    std::vector<std::shared_ptr<UIViewController>> controllerTests = {
        controllerTestController
    };
    std::vector<std::vector<std::shared_ptr<UIViewController>>> vcs;

    vcs.push_back(vcs1);
    vcs.push_back(vcs2);
    vcs.push_back(navigationTests);
    vcs.push_back(controllerTests);

    auto tbvc = new_shared<NXTabBarController>(vcs);
    tbvc->setTitle("NXKit Demo");
    auto nvc = new_shared<NXNavigationController>(tbvc);
    nvc->navigationItemFor(tbvc)->setIcon(UIImage::fromRes("img/moonlight_96.png", 3));
    window->setRootViewController(nvc);
    window->makeKeyAndVisible();
//    window->setBackgroundColor(UIColor::systemBackground);
    window->setBackgroundColor(UIColor::secondarySystemBackground);
    window->setTintColor(UIColorThemed(UIColor::systemBlue, UIColor::systemTeal, "_tint"));
#ifdef __SWITCH__
    installSwitchExitTest([weakTabs = std::weak_ptr<NXTabBarController>(tbvc)](size_t index) {
        if (const auto tabs = weakTabs.lock()) tabs->setSelectedIndex(index);
    });
#endif
//    window->setTintColor(UIColorThemed(UIColor::systemTeal, UIColor::systemTeal));

    return true;
}

}
