#include <Screens/TestViewController/TestViewController.hpp>
#include <romfs/romfs.hpp>

using namespace NXKit;

void animateCube(std::shared_ptr<UIView> view) {
    UIView::animate(2.5, 0, 0.5, 2, UIViewAnimationOptions::none, [view]() {
        view->setTransform(NXAffineTransform::identity.rotationBy(55));
        view->setBackgroundColor(UIColor::orange);
    }, [view](bool res) {
        DispatchQueue::main()->asyncAfter(1, [view]() {
            UIView::animate(2.5, [view]() {
                view->setTransform(NXAffineTransform::identity.rotationBy(10));
                view->setBackgroundColor(UIColor::blue);
            }, [view](bool res) {
                DispatchQueue::main()->asyncAfter(1, [view]() {
                    animateCube(view);
                });
            });
        });
    });
}

void animateBlur(std::shared_ptr<UIBlurView> view) {
    UIView::animate(2.5, [view]() {
        view->setBlurValue(0);
    }, [view](bool res) {
        UIView::animate(2.5, [view]() {
            view->setBlurValue(20);
        }, [view](bool res) {
            animateBlur(view);
        });
    });
}

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

    // Looks like transform interpolation has issues
//    UIView::animate(5, [label]() {
//        label->layer()->setTransform(NXTransform3D::identity.rotationBy(180, 0, 0, 1));
//    }, [label](bool res) {
//        UIView::animate(5, [label]() {
//            label->layer()->setTransform(NXTransform3D::identity.rotationBy(360, 0, 0, 1));
//        }, [label](bool res) {
//            label->layer()->setTransform(NXTransform3D::identity);
//            animateLabel(label);
//        });
//    });
}

void switchLabelText(std::shared_ptr<UILabel> label) {
    label->setText("Text number 1");
    label->sizeToFit();
    DispatchQueue::main()->asyncAfter(2, [label]() {
        label->setText("Text number 2\nHere is more text to test sizeToFit");
        label->sizeToFit();
        DispatchQueue::main()->asyncAfter(2, [label]() {
            switchLabelText(label);
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
    animateCube(imageView);

    label = new_shared<UILabel>();
    label->setFrame({ 200, 100, 240, 88 });
    label->setText("Привет\nебать,\nэто\nкириллица,\nнахуй!!!!");
    label->setTextAlignment(NSTextAlignment::center);
    label->setBackgroundColor(UIColor::red);
    label->setContentMode(NXKit::UIViewContentMode::center);
    rootView->addSubview(label);

    auto blur = new_shared<UIBlurView>();
    blur->setFrame({ 80, 80, 240, 240 });
    rootView->addSubview(blur);
    animateBlur(blur);

    bottomBar = new_shared<UIView>();
    bottomBar->setBackgroundColor(UIColor::gray);
    rootView->addSubview(bottomBar);

    label2 = new_shared<UILabel>();
    label2->setText("Test text\nTry to fit me!!!");
    label2->setBackgroundColor(UIColor::cyan);
    label2->setFrame({ 100, 200, 0, 0 });
    rootView->addSubview(label2);

    switchLabelText(label2);

    auto button = new_shared<UILabel>();
    button->setText("Press me!");
    button->setFontWeight(600);
    button->setTextAlignment(NSTextAlignment::center);
    button->setFrame({ 100, 300, 200, 60 });
    button->layer()->setCornerRadius(12);
    button->setBackgroundColor(UIColor::tint);
    rootView->addSubview(button);

    auto tapGesture = new_shared<UITapGestureRecognizer>();
    tapGesture->onStateChanged = [button](UIGestureRecognizerState status) {
        if (status == NXKit::UIGestureRecognizerState::ended) {
            static bool toggle = false;
            toggle = !toggle;
            button->setText(toggle ? "You did it!!!!" : "Another text, keep trying!!!!!");
        }
    };
    button->addGestureRecognizer(tapGesture);

    setView(rootView);
}

void TestViewController::viewDidLoad() {
    UIViewController::viewDidLoad();
    animateLabel(label);

    auto testMatrix = CATransform3DMakeRotation(45, 0, 0, 1);
    printf("");
}

void TestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    label2->sizeToFit();

    auto frame = view()->frame();
    NXFloat bottomBarHeight = 48 + 35; //83;
    bottomBar->setFrame({ 0, frame.size.height - bottomBarHeight, frame.size.width, bottomBarHeight });
}
