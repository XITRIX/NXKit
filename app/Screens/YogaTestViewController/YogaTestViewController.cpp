#include <Screens/YogaTestViewController/YogaTestViewController.hpp>

using namespace NXKit;

YogaTestViewController::YogaTestViewController() {
    
}

void YogaTestViewController::loadView() {
    auto rootView = new_shared<UIView>();

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

    auto tap = new_shared<UITapGestureRecognizer>();
    tap->onStateChanged = [](auto tap) {
        if (tap->state() != UIGestureRecognizerState::ended) return;
        std::shared_ptr<UIView> view = tap->view().lock();

        static bool toggle = false;
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

    auto buttonLabel = new_shared<UILabel>();
    auto control = new_shared<UIControl>();

    control->addSubview(buttonLabel);
    control->layer()->setCornerRadius(10);
    buttonLabel->setText("Button!");
    buttonLabel->setFontWeight(800);
    buttonLabel->setTextAlignment(NSTextAlignment::center);
    buttonLabel->setAutolayoutEnabled(true);

    control->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setJustifyContent(YGJustify::YGJustifyCenter);
        layout->setAlignContent(YGAlign::YGAlignCenter);
//        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
//        layout->setPosition({230, 80});
        layout->setSize({ 200, 44 });
    });

    buttonsBox->addSubview(control);

    auto buttonLabel2 = new_shared<UILabel>();
    auto control2 = new_shared<UIControl>();

    control2->addSubview(buttonLabel2);
    control2->layer()->setCornerRadius(10);
    buttonLabel2->setText("Button!");
    buttonLabel2->setFontWeight(800);
    buttonLabel2->setTextAlignment(NSTextAlignment::center);
    buttonLabel2->setAutolayoutEnabled(true);

    control2->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setJustifyContent(YGJustify::YGJustifyCenter);
        layout->setAlignContent(YGAlign::YGAlignCenter);
//        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
//        layout->setPosition({530, 80});
        layout->setSize({ 200, 44 });
    });

    buttonsBox->addSubview(control2);

    buttonsBox->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setJustifyContent(YGJustify::YGJustifyCenter);
//        layout->setAlignContent(YGAlign::YGAlignCenter);
        layout->setPositionType(YGPositionType::YGPositionTypeAbsolute);
        layout->setPosition({130, 80});
        layout->setAllGap(8);
    });
    rootView->addSubview(buttonsBox);

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

    setView(rootView);
}

void YogaTestViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
}
