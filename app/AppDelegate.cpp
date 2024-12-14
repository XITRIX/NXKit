#include <UIApplicationDelegate.h>
#include "romfs/romfs.hpp"
// #include <TestViewController/TestViewController.hpp>
// #include <YogaTestViewController/YogaTestViewController.hpp>
// #include <NavigationViewController/NavigationViewController.hpp>
// #include <TextViewController/TextViewController.hpp>

namespace NXKit {

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication* application, std::map<std::string, std::any> launchOptions) {
    window = new_shared<UIWindow>();

//    auto vc = new_shared<TestViewController>();
//    auto vc = new_shared<YogaTestViewController>();
    // auto vc = new_shared<NavigationViewController>();
//    auto vc = new_shared<TextViewController>();
    // window->setRootViewController(vc);
    window->makeKeyAndVisible();
    window->layer()->setBackgroundColor(UIColor::systemBackground);

    auto sublayer = new_shared<CALayer>();
    sublayer->setPosition({ 100, 100 });
    sublayer->setBounds({ 0, 0, 200, 44 });
    sublayer->setBackgroundColor(UIColor::tint);
    sublayer->setCornerRadius(10);
    sublayer->setAnchorPoint(NXPoint::zero);
    sublayer->setOpacity(0.5);
//    sublayer->setTransform(NXTransform3D::identity.rotationBy(45, 0, 0, 1));

    window->layer()->addSublayer(sublayer);

    auto res = romfs::get("img/star.png");
    auto data = new_shared<NXData>(res.data(), (int) res.size());
    auto image = new_shared<CGImage>(data);

    auto imageLayer = new_shared<CALayer>();
    imageLayer->setContents(image);
    imageLayer->setFrame({ 0, 0, 120, 120 });
    imageLayer->setBackgroundColor(UIColor::green);
    imageLayer->setContentsGravity(CALayerContentsGravity::topLeft);

    sublayer->addSublayer(imageLayer);

    return true;
}

}
