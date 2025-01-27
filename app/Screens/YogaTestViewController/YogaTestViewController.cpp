#include <Screens/YogaTestViewController/YogaTestViewController.hpp>
#include "romfs/romfs.hpp"

using namespace NXKit;

YogaTestViewController::YogaTestViewController() {
    
}

void YogaTestViewController::loadView() {
    auto rootView = new_shared<UIView>();
//    rootView->setBackgroundColor(UIColor::secondarySystemBackground);

    //MARK: - HEADER!
    auto header = new_shared<UIView>();
    header->setBackgroundColor(UIColor::systemRed);
//    header->setFrame({ 20, 20, 300, 40 });
    header->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setHeight(66_pt);
    });
    rootView->addSubview(header);



    //MARK: - Content!
    auto contentBox = new_shared<UIView>();
    auto contentContent1 = new_shared<UIView>();
    contentContent1->setBackgroundColor(UIColor::systemTeal);
    auto contentContent2 = new_shared<UIView>();
    contentContent2->setBackgroundColor(UIColor::systemMint);

    contentContent1->configureLayout([](auto layout) {
        layout->setSize({ 200, 100 });
    });
    contentContent2->configureLayout([](auto layout) {
        layout->setSize({ 200, 100 });
    });
    contentBox->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setSize({ 200, 200 });
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setJustifyContent(YGJustifySpaceBetween);
    });

    contentBox->addSubview(contentContent1);
    contentBox->addSubview(contentContent2);
    rootView->addSubview(contentBox);

    static bool toggle = false;
    auto tap = new_shared<UITapGestureRecognizer>();
    tap->onStateChanged = [](auto tap) {
        if (tap->state() != UIGestureRecognizerState::ended) return;
        std::shared_ptr<UIView> view = tap->view().lock();

        toggle = !toggle;

        UIView::animate(0.3, 0, UIViewAnimationOptions::beginFromCurrentState, [view]() {
            view->configureLayout([](std::shared_ptr<YGLayout> layout) {
                layout->setFlexDirection(toggle ? YGFlexDirection::YGFlexDirectionColumnReverse : YGFlexDirection::YGFlexDirectionColumn);
            });
            view->setNeedsLayout();
            view->layoutIfNeeded();
        });
    };
    contentBox->addGestureRecognizer(tap);
    contentBox->tag = "Content box";

    auto buttonsBox = new_shared<UIView>();

    auto res = romfs::get("img/icons8-star-90.png");
    auto data = new_shared<NXData>(res.data(), res.size());
    auto image = UIImage::fromData(data, 3);
    image->setRenderModeAsTemplate(true);

    auto control = new_shared<UIButton>(UIButtonStyle::filled);
    control->layer()->setCornerRadius(10);
    control->titleLabel()->setText("Buttogn!");
    control->setImage(image);
    control->setTintColor(UIColor::systemOrange);
    control->yoga()->setWidth(10_pt);
    control->yoga()->setFlexGrow(1);

    buttonsBox->addSubview(control);

    auto control2 = new_shared<UIButton>(UIButtonStyle::filled);
    control2->layer()->setCornerRadius(10);
    control2->titleLabel()->setText("Button!");
    control2->setTintColor(UIColor::systemRed);
    control2->yoga()->setWidth(10_pt);
    control2->yoga()->setFlexGrow(1);

    buttonsBox->addSubview(control2);

    buttonsBox->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setJustifyContent(YGJustify::YGJustifyCenter);
//        layout->setAlignContent(YGAlign::YGAlignCenter);
//        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
//        layout->setPosition({30, 80});
        layout->setAllGap(8);
    });

    auto control3 = new_shared<UIButton>(UIButtonStyle::filled);
    control3->layer()->setCornerRadius(10);
    control3->titleLabel()->setText("Swap boxes!");
    control3->setTintColor(UIColor::systemGreen);
    control3->primaryAction = UIAction("", [&, contentBox]() {

        toggle = !toggle;

        UIView::animate(0.3, 0, UIViewAnimationOptions::beginFromCurrentState, [contentBox]() {
            contentBox->configureLayout([](std::shared_ptr<YGLayout> layout) {
                layout->setFlexDirection(toggle ? YGFlexDirection::YGFlexDirectionColumnReverse : YGFlexDirection::YGFlexDirectionColumn);
            });
            contentBox->setNeedsLayout();
            contentBox->layoutIfNeeded();
        });
    });


//    buttonsBox->setBackgroundColor(UIColor::systemPink);
    auto buttonsBox2 = new_shared<UIView>();
    buttonsBox2->addSubview(buttonsBox);
    buttonsBox2->addSubview(control3);

    buttonsBox2->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionColumn);
//        layout->setJustifyContent(YGJustify::YGJustifyCenter);
        layout->setAlignContent(YGAlign::YGAlignStretch);
        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
        layout->setPosition({30, 80});
        layout->setWidth(400_pt);
//        layout->setSize({400, 80});
        layout->setAllGap(8);
    });

    rootView->addSubview(buttonsBox2);

    //MARK: - Footer!
    auto footer = new_shared<UIView>();
    footer->setBackgroundColor(UIColor::systemBlue);
//    footer->setFrame({ 20, 420, 300, 40 });
    footer->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setHeight(66_pt);
    });
    rootView->addSubview(footer);

    rootView->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setJustifyContent(YGJustifySpaceBetween);
        layout->setAlignContent(YGAlign::YGAlignCenter);
    });


    auto dragMeViewLabel = new_shared<UILabel>();
    dragMeViewLabel->setText("Drag me!");
    dragMeViewLabel->setFontWeight(600);
    dragMeViewLabel->setTextAlignment(NSTextAlignment::center);
    dragMeViewLabel->setAutolayoutEnabled(true);
    dragMeViewLabel->setBackgroundColor(UIColor::systemGreen);
    // dragMeViewLabel->setHidden(true);
    // dragMeViewLabel->setFrame({ 80, 110, 44, 100 });

    auto blur = new_shared<UIBlurView>();
    blur->setFrame({ 80, 200, 240, 240 });
    rootView->addSubview(blur);
    blur->layer()->setCornerRadius(12);
    blur->setBackgroundTintColor(UIColorThemed(UIColor(0x60DDDDDD), UIColor(0x60404040)));

    blur->layer()->setShadowOffset({4, 4});
    blur->layer()->setShadowColor(UIColor::black.withAlphaComponent(0.05f));
//    blur->layer()->setShadowOpacity(0.2);
    blur->layer()->setShadowRadius(16);

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

                _view->yoga()->setPosition({ initial.x + translation.x, initial.y + translation.y });
                break;
            }
            default:
                break;
        }
    };
    blur->addGestureRecognizer(panGesture);

    blur->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setJustifyContent(YGJustify::YGJustifyCenter);
        layout->setAlignContent(YGAlign::YGAlignCenter);
        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
        layout->setPosition({100, 200});
        layout->setSize({ 200, 200 });
    });

    auto scroll = new_shared<UIScrollView>();
    rootView->addSubview(scroll);

    auto res1 = romfs::get("img/star.png");
    auto data1 = new_shared<NXData>(res1.data(), res1.size());
    auto image1 = UIImage::fromData(data1, 3);

    auto imageView = new_shared<UIImageView>();
    imageView->setContentMode(UIViewContentMode::center);
    imageView->setAutolayoutEnabled(true);
    imageView->setImage(image1);
//    imageView->setBackgroundColor(UIColor::systemOrange);
    imageView->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setPositionType(YGPositionTypeAbsolute);
        layout->setPosition({ 0, 0 });
        layout->setSize({ 200, 200 });
    });

    scroll->addSubview(imageView);
    scroll->setBackgroundColor(UIColor::tertiarySystemFill);
    scroll->setBounceHorizontally(true);
    scroll->setBounceVertically(true);

    scroll->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
        layout->setPosition({200, 200});
        layout->setSize({ 100, 100 });
    });

    setView(rootView);
}

void YogaTestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
}
