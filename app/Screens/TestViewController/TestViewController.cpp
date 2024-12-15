#include <Screens/TestViewController/TestViewController.hpp>
#include <romfs/romfs.hpp>

using namespace NXKit;

void animateLabel(std::shared_ptr<UILabel> label) {
    UIView::animate(5, [label]() {
        label->setFrame({ 200, 100, 140, 88 });
        label->setTextColor(UIColor::cyan);
        label->setBackgroundColor(UIColor::green);
        label->setFontWeight(100);
    }, [label](bool res) {
        UIView::animate(5, [label]() {
            label->setFrame({ 200, 100, 240, 88 });
            label->setTextColor(UIColor::black);
            label->setBackgroundColor(UIColor::red);
            label->setFontWeight(1000);
        }, [label](bool res) {
            animateLabel(label);
        });
    });
}

TestViewController::TestViewController() {

}

void TestViewController::loadView() {
    auto rootView = new_shared<UIView>();

    auto subview = new_shared<UIView>();
    subview->setFrame({ 100, 100, 200, 44 });
    subview->setBackgroundColor(UIColor::tint);
    subview->layer()->setCornerRadius(10);
    subview->setAlpha(0.5);
//    subview->setClipsToBounds(true);

    rootView->addSubview(subview);

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

    label = new_shared<UILabel>();
    label->setFrame({ 200, 100, 240, 88 });
    label->setText("Привет\nебать,\nэто\nкириллица,\nнахуй!!!!");
    label->setTextAlignment(NSTextAlignment::center);
    label->setBackgroundColor(UIColor::red);
    rootView->addSubview(label);

    setView(rootView);
}

void TestViewController::viewDidLoad() {
    UIViewController::viewDidLoad();
    animateLabel(label);
}
