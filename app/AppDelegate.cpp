#include <UIApplicationDelegate.h>
#include <UIImageView.h>
#include <UILabel.h>
#include "romfs/romfs.hpp"
#include <DispatchQueue.h>
// #include <TestViewController/TestViewController.hpp>
// #include <YogaTestViewController/YogaTestViewController.hpp>
// #include <NavigationViewController/NavigationViewController.hpp>
// #include <TextViewController/TextViewController.hpp>

namespace NXKit {

void animateLabel(std::shared_ptr<UILabel> label) {
    UIView::animate(5, [label]() {
        label->setFrame({ 200, 100, 140, 88 });
        label->setTextColor(UIColor::cyan);
    }, [label](bool res) {
        UIView::animate(5, [label]() {
            label->setFrame({ 200, 100, 240, 88 });
            label->setTextColor(UIColor::black);
        }, [label](bool res) {
            animateLabel(label);
        });
    });
}

bool UIApplicationDelegate::applicationDidFinishLaunchingWithOptions(UIApplication* application, std::map<std::string, std::any> launchOptions) {
    window = new_shared<UIWindow>();

//    auto vc = new_shared<TestViewController>();
//    auto vc = new_shared<YogaTestViewController>();
    // auto vc = new_shared<NavigationViewController>();
//    auto vc = new_shared<TextViewController>();
    // window->setRootViewController(vc);
    window->makeKeyAndVisible();
    window->setBackgroundColor(UIColor::systemBackground);

    auto subview = new_shared<UIView>();
    subview->setFrame({ 100, 100, 200, 44 });
    subview->setBackgroundColor(UIColor::tint);
    subview->layer()->setCornerRadius(10);
    subview->setAlpha(0.5);
//    subview->setClipsToBounds(true);

    window->addSubview(subview);

    auto res = romfs::get("img/star.png");
    auto data = new_shared<NXData>(res.data(), res.size());
    auto image = UIImage::fromData(data, 3);

    auto imageView = new_shared<UIImageView>();
    imageView->setImage(image);
    imageView->setFrame({ 0, 0, 120, 120 });
    imageView->setBackgroundColor(UIColor::green);
    imageView->setContentMode(UIViewContentMode::topLeft);
    
    subview->addSubview(imageView);

    UIView::animate(5, [imageView]() {
        imageView->setTransform(NXAffineTransform::identity.rotationBy(45));
        imageView->setBackgroundColor(UIColor::orange);
    });

    auto label = new_shared<UILabel>();
    label->setFrame({ 200, 100, 240, 88 });
    label->setBackgroundColor(UIColor::red);
    window->addSubview(label);

    animateLabel(label);

    return true;
}

}
