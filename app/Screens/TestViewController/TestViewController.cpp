#include <Screens/TestViewController/TestViewController.hpp>
#include <romfs/romfs.hpp>

using namespace NXKit;

void animateCube(std::shared_ptr<UIView> view) {
    UIView::animate(2.5, 0, 0.5, 2, UIViewAnimationOptions::none, [view]() {
        view->setTransform(NXAffineTransform::identity.rotationBy(55));
        view->setBackgroundColor(UIColor::systemOrange);
    }, [view](bool res) {
        DispatchQueue::main()->asyncAfter(1, [view]() {
            UIView::animate(2.5, [view]() {
                view->setTransform(NXAffineTransform::identity.rotationBy(10));
                view->setBackgroundColor(UIColor::systemBlue);
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
        view->setBlurRadius(0);
    }, [view](bool res) {
        UIView::animate(2.5, [view]() {
            view->setBlurRadius(20);
        }, [view](bool res) {
            animateBlur(view);
        });
    });
}

void animateLabel(std::shared_ptr<UILabel> label) {
    UIView::animate(5, [label]() {
        label->setFrame({ 200, 100, 140, 88 });
        label->setTextColor(UIColor::systemCyan);
        label->setBackgroundColor(UIColor::systemGreen);
        label->setFontWeight(100);
    }, [label](bool res) {
        UIView::animate(5, [label]() {
            label->setFrame({ 200, 100, 240, 88 });
            label->setTextColor(UIColor::black);
            label->setBackgroundColor(UIColor::systemRed);
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
    setTitle("Test VC");
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
    imageView->setBackgroundColor(UIColor::systemGreen);
    imageView->setContentMode(UIViewContentMode::topLeft);

    subview->addSubview(imageView);
    animateCube(imageView);

    label = new_shared<UILabel>();
    label->setFrame({ 200, 100, 240, 88 });
    label->setText("Привет\nебать,\nэто\nкириллица,\nнахуй!!!!");
    label->setTextAlignment(NSTextAlignment::center);
    label->setBackgroundColor(UIColor::systemRed);
    label->setContentMode(NXKit::UIViewContentMode::center);
    rootView->addSubview(label);

//    animateBlur(blur);

    label2 = new_shared<UILabel>();
    label2->setText("Test text\nTry to fit me!!!");
    label2->setBackgroundColor(UIColor::systemCyan);
    label2->setFrame({ 150, 200, 0, 0 });
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
    tapGesture->onStateChanged = [](auto gesture) {
        if (gesture->state() == NXKit::UIGestureRecognizerState::ended) {
            static bool toggle = false;
            toggle = !toggle;
            auto button = std::static_pointer_cast<UILabel>(gesture->view().lock());
            button->setText(toggle ? "You did it!!!!" : "Another text, keep trying!!!!!");
        }
    };
    button->addGestureRecognizer(tapGesture);

    dragMeViewLabel = new_shared<UILabel>();
    dragMeViewLabel->setText("Drag me!");
    dragMeViewLabel->setFontWeight(600);
    dragMeViewLabel->setTextAlignment(NSTextAlignment::center);
//    dragMeViewLabel->setFrame({ 80, 110, 44, 100 });

    auto blur = new_shared<UIBlurView>();
    blur->setFrame({ 80, 200, 240, 240 });
    rootView->addSubview(blur);
    blur->layer()->setCornerRadius(12);
    blur->setBackgroundTintColor(UIColorThemed(UIColor(0x60DDDDDD), UIColor(0x60404040)));

    blur->addSubview(dragMeViewLabel);

    auto panGesture = new_shared<UIPanGestureRecognizer>();
    panGesture->onStateChanged = [this](auto gesture) {
        auto panGesture = std::static_pointer_cast<UIPanGestureRecognizer>(gesture);
        static NXPoint initial;
        auto _view = panGesture->view().lock();
        if (!_view) return;

        switch (panGesture->state()) {
            case UIGestureRecognizerState::began: {
                initial = _view->frame().origin;
                break;
            }
            case UIGestureRecognizerState::changed: {
                auto translation = panGesture->translationInView(view());

                auto frame = _view->frame();
                frame.origin.x = initial.x + translation.x;
                frame.origin.y = initial.y + translation.y;
                _view->setFrame(frame);
                break;
            }
            default:
                break;
        }
    };
    blur->addGestureRecognizer(panGesture);

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

    auto bounds = dragMeViewLabel->superview().lock()->bounds();
    dragMeViewLabel->sizeToFit();
    dragMeViewLabel->setCenter({ bounds.midX(), bounds.midY() });

    label2->sizeToFit();
}
