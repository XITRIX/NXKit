#include <Screens/YogaTestViewController/YogaTestViewController.hpp>

using namespace NXKit;

YogaTestViewController::YogaTestViewController() {
    
}

void YogaTestViewController::loadView() {
    auto rootView = new_shared<UIView>();

    auto header = new_shared<UIView>();
    header->setBackgroundColor(UIColor::systemRed);
//    header->setFrame({ 20, 20, 300, 40 });
    header->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setFlexDirection(YGFlexDirectionRow);
        layout->setHeight(66_pt);
    });
    rootView->addSubview(header);

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
    });

    auto dragMeViewLabel = new_shared<UILabel>();
    dragMeViewLabel->setText("Drag me!");
    dragMeViewLabel->setFontWeight(600);
    dragMeViewLabel->setTextAlignment(NSTextAlignment::center);
    dragMeViewLabel->setAutolayoutEnabled(true);
//    dragMeViewLabel->setFrame({ 80, 110, 44, 100 });

    auto blur = new_shared<UIBlurView>();
    blur->setFrame({ 80, 200, 240, 240 });
    rootView->addSubview(blur);
    blur->layer()->setCornerRadius(12);
    blur->setBackgroundTintColor(UIColorThemed(UIColor(0x60DDDDDD), UIColor(0x60404040)));

    blur->addSubview(dragMeViewLabel);

    auto panGesture = new_shared<UIPanGestureRecognizer>();
    panGesture->onStateChanged = [this, panGesture](UIGestureRecognizerState status) {
        static NXPoint initial;
        auto _view = panGesture->view().lock();
        if (!_view) return;

        switch (status) {
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
