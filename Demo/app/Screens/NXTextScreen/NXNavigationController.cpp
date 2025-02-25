//
// Created by Даниил Виноградов on 22.02.2025.
//

#include "NXNavigationController.h"

#include <utility>

using namespace NXKit;

class NXNavigationBar: public UIBlurView {
public:
    NXNavigationBar() {
        auto headerContent = new_shared<UIView>();
        headerContent->setAutolayoutEnabled(true);

        _imageView = new_shared<UIImageView>();
        _titleLabel = new_shared<UILabel>();

        _imageView->setContentMode(UIViewContentMode::scaleAspectFill);
        _imageView->setImage(UIImage::fromRes("img/moonlight_96.png", 3));

        _titleLabel->setFontSize(28);

        _imageView->setAutolayoutEnabled(true);
        _titleLabel->setAutolayoutEnabled(true);

        headerContent->addSubview(_imageView);
        headerContent->addSubview(_titleLabel);

        _imageView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setSize({ 58, 58 });
        });

        headerContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setAllGap(12);
            layout->setPaddingTop(14_pt);
            layout->setMarginHorizontal(30_pt);
            layout->setHeight(88_pt);
            layout->setFlexGrow(1);
//        layout->setSize({ 250, 50 });
        });

        addSubview(headerContent);

        auto separator = new_shared<UIView>();
        separator->setAutolayoutEnabled(true);
        separator->setBackgroundColor(UIColor::label);
        separator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setHeight(1_pt);
        });
        addSubview(separator);

        configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionColumn);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignStretch);
//            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setPaddingHorizontal(30_pt);
        });
    }

    void setTitle(const std::string& text) {
        _titleLabel->setText(text);
    }
private:
    std::shared_ptr<UIImageView> _imageView;
    std::shared_ptr<UILabel> _titleLabel;
};

class NXFooterBar: public UIBlurView {
public:
    NXFooterBar() {
        auto headerContent = new_shared<UIView>();
        headerContent->setAutolayoutEnabled(true);

        _imageView = new_shared<UIImageView>();
        _titleLabel = new_shared<UILabel>();

        _imageView->setContentMode(UIViewContentMode::center);
        _imageView->setImage(UIImage::fromRes("img/sys/battery_back_dark.png", 1));

        _titleLabel->setFontSize(21.5);

        _imageView->setAutolayoutEnabled(true);
        _titleLabel->setAutolayoutEnabled(true);

        headerContent->addSubview(_imageView);
        headerContent->addSubview(_titleLabel);

        _imageView->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setSize({ 40, 40 });
        });

        headerContent->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionRow);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignCenter);
            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setAllGap(30);
            layout->setPaddingHorizontal(30_pt);
            layout->setFlexGrow(1);
            layout->setHeight(73_pt);
//        layout->setSize({ 250, 50 });
        });


        auto separator = new_shared<UIView>();
        separator->setAutolayoutEnabled(true);
        separator->setBackgroundColor(UIColor::label);
        separator->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setHeight(1_pt);
        });
        addSubview(separator);
        addSubview(headerContent);

        configureLayout([](const std::shared_ptr<YGLayout>& layout) {
            layout->setFlexDirection(YGFlexDirectionColumn);
//        layout->setAlignContent(YGAlignFlexEnd);
            layout->setAlignItems(YGAlignStretch);
//            layout->setJustifyContent(YGJustifyFlexStart);
            layout->setPaddingHorizontal(30_pt);
        });
    }

    void setTitle(const std::string& text) {
        _titleLabel->setText(text);
    }
private:
    std::shared_ptr<UIImageView> _imageView;
    std::shared_ptr<UILabel> _titleLabel;
};

NXNavigationController::NXNavigationController(const std::shared_ptr<UIViewController>& rootViewController) {
    _viewControllers.push_back(rootViewController);
}

void NXNavigationController::loadView() {
    auto view = new_shared<UIView>();
    view->setBackgroundColor(UIColor::systemBackground);

    auto header = new_shared<NXNavigationBar>();
    header->setAutolayoutEnabled(true);
    header->setTitle("Системные настройки");
    _navigationBar = header;

    auto footer = new_shared<NXFooterBar>();
    footer->setAutolayoutEnabled(true);
    footer->setTitle("16:21:53");
    _toolBar = footer;

    view->addSubview(_navigationBar);
    view->addSubview(_toolBar);

    view->configureLayout([](const std::shared_ptr<YGLayout>& layout) {
        layout->setFlexDirection(YGFlexDirectionColumn);
        layout->setJustifyContent(YGJustifySpaceBetween);
        layout->setAlignItems(YGAlignStretch);
    });

    setView(view);
}

void NXNavigationController::viewDidLoad() {
    UIViewController::viewDidLoad();
    updatePresentedViewController(false);
}

void NXNavigationController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    _navigationBar->yoga()->setPaddingTop(YGValue { view()->superview().lock()->safeAreaInsets().top, YGUnitPoint });
    _toolBar->yoga()->setPaddingBottom(YGValue { view()->superview().lock()->safeAreaInsets().bottom, YGUnitPoint });
    setAdditionalSafeAreaInsets({88, 0, 73, 0});
}

void NXNavigationController::setViewControllers(std::vector<std::shared_ptr<UIViewController>> viewControllers, bool animated) {
    _viewControllers = std::move(viewControllers);
    updatePresentedViewController(animated);
}

std::shared_ptr<UIViewController> NXNavigationController::topViewController() {
    if (_viewControllers.empty()) return nullptr;
    return _viewControllers.back();
}

void NXNavigationController::updatePresentedViewController(bool animated) {
    if (presentedViewController != nullptr) {
        presentedViewController->willMoveToParent(nullptr);
        presentedViewController->view()->removeFromSuperview();
        presentedViewController->removeFromParent();
    }

    presentedViewController = topViewController();
    if (presentedViewController == nullptr) return;

    addChild(presentedViewController);
    view()->insertSubviewAt(presentedViewController->view(), 0);
    presentedViewController->didMoveToParent(shared_from_this());

    presentedViewController->view()->configureLayout([](std::shared_ptr<YGLayout> layout) {
        layout->setWidth(100_percent);
        layout->setHeight(100_percent);
        layout->setPositionType(YGPositionTypeAbsolute);
    });
}
