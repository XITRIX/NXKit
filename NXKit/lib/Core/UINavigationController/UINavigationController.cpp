//
//  UINavigationController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include <Core/UINavigationController/UINavigationController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIBlurView/UIBlurView.hpp>
#include <Core/UILabel/UILabel.hpp>
#include <Core/UIImageView/UIImageView.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UINavigationBar::UINavigationBar() {
    setAxis(Axis::HORIZONTAL);
    setBorderBottom(1);
    borderColor = UIColor::label;
    setPaddingLeft(35);
    setPaddingRight(35);
    setMarginLeft(30);
    setMarginRight(30);
    setJustifyContent(JustifyContent::FLEX_START);
    setAlignItems(AlignItems::CENTER);

    titleLabel = NXKit::make_shared<UILabel>("Demo app");
    imageView = NXKit::make_shared<UIImageView>();
    imageView->setSize(Size(48, 48));
    imageView->setMarginRight(18);

    titleLabel->getFont()->fontSize = 28;
    titleLabel->setMarginTop(7);

    addSubview(imageView);
    addSubview(titleLabel);
}

UINavigationBar::~UINavigationBar() {
    imageView->setImage(nullptr, false);
}

void UINavigationBar::pushNavigationItem(UINavigationItem navigationItem) {
    titleLabel->setText(navigationItem.title);
    imageView->setHidden(!navigationItem.image);

    if (navigationItem.image) { imageView->setImage(navigationItem.image, false); }
}

UINavigationController::UINavigationController(std::shared_ptr<UIViewController> rootController) {
    pushViewController(rootController, false);

    addAction(BUTTON_B, UIAction([this]() {
        if (viewControllers.size() > 1)
            popViewController(true, true);
    }, "Back", true, true, [this]() {
        return viewControllers.size() > 1;
    }));
}

UINavigationController::~UINavigationController() {
    for (auto vc: viewControllers) {
//        if (vc->getParent().get() != this)
//            delete vc;
    }
}

void UINavigationController::loadView() {
    auto root = NXKit::make_shared<UIView>();
    setView(root);

    root->backgroundColor = UIColor::systemBackground;

    overlay = NXKit::make_shared<UIStackView>(Axis::VERTICAL);
    overlay->passthroughTouches = true;
    overlay->setJustifyContent(JustifyContent::SPACE_BETWEEN);

    setAdditionalSafeAreaInsets(UIEdgeInsets(headerHeight, 0, footerHeight, 0));

    auto blurHeader = NXKit::make_shared<UIBlurView>();
    navigationBar = NXKit::make_shared<UINavigationBar>();
    navigationBar->setSize(Size(UIView::AUTO, headerHeight));
    blurHeader->addSubview(navigationBar);

    overlay->addSubview(blurHeader);
    overlay->addSubview(buildFooter());

    root->addSubview(overlay);
}

std::shared_ptr<UIView> UINavigationController::buildFooter() {
    auto blurFooter = NXKit::make_shared<UIBlurView>();
    auto footer = NXKit::make_shared<UIStackView>();
    footer->setAxis(Axis::HORIZONTAL);
    footer->setAlignItems(AlignItems::STRETCH);
    footer->setJustifyContent(JustifyContent::FLEX_END);
    footer->setBorderTop(1);
    footer->borderColor = UIColor::label;
    footer->setMargins(0, 30, 4, 30);
    footer->setPadding(4, 8, 0, 8);

    acctionsBar = NXKit::make_shared<UIActionsView>();
    acctionsBar->setSize(Size(UIView::AUTO, footerHeight));
    acctionsBar->inController = weak_from_this();
    footer->addSubview(acctionsBar);

    blurFooter->addSubview(footer);
    return blurFooter;
}

void UINavigationController::viewDidLoad() {
    getView()->clipToBounds = true;

}

void UINavigationController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    overlay->setSize(getView()->getFrame().size);
    printf("Width: %f, Height: %f\n", getView()->getFrame().size.width, getView()->getFrame().size.height);

    if (isTranslucent)
        viewControllers.back()->getView()->setSize(getView()->getFrame().size);
    else {
        Size size = getView()->getFrame().size;
        size.height -= headerHeight + footerHeight;
        viewControllers.back()->getView()->setSize(size);
        viewControllers.back()->getView()->setPosition(Point(0, headerHeight));
    }

    auto insets = getView()->safeAreaInsets();
    navigationBar->setMarginTop(insets.top);
    acctionsBar->setMarginBottom(insets.bottom);
}

void UINavigationController::show(std::shared_ptr<UIViewController> controller, void* sender) {
    pushViewController(controller, true);
}

void UINavigationController::pushViewController(std::shared_ptr<UIViewController> otherViewController, bool animated) {
    if (viewControllers.size() == 0) animated = false;

    getView();
    navigationBar->pushNavigationItem(otherViewController->getNavigationItem());

    if (!animated) {
        if (viewControllers.size() > 0) {
            viewControllers.back()->willMoveToParent(nullptr);
            viewControllers.back()->getView()->removeFromSuperview();
            viewControllers.back()->removeFromParent();
        }

        viewControllers.push_back(otherViewController);

        addChild(otherViewController);
        getView()->insertSubview(otherViewController->getView(), 0);
        otherViewController->didMoveToParent(weak_from_this().lock());
        Application::shared()->setFocus(otherViewController->getView()->getDefaultFocus());

        otherViewController->getView()->setGrow(1);
        otherViewController->getView()->setShrink(1);
    } else {
        auto oldView = viewControllers.back();
        viewControllers.push_back(otherViewController);

        // Prepare to remove old view
        oldView->willMoveToParent(nullptr);

        // Prepare to add new view
        addChild(otherViewController);
        getView()->insertSubview(otherViewController->getView(), 1);

        // Move new view to initial state
        otherViewController->getView()->alpha = 0;
        otherViewController->getView()->transform = NXAffineTransform::translationBy(20, 0);

        // Remove focus from old View
        Application::shared()->setFocus(nullptr);

        UIView::animate({ otherViewController->getView(), oldView->getView() }, 0.1f, [oldView, otherViewController]() {
            // Animate old View
            otherViewController->getView()->transform = NXAffineTransform::translationBy(-20, 0);
            oldView->getView()->alpha = 0;

            // Animate new View
            otherViewController->getView()->alpha = 1;
            otherViewController->getView()->transform = NXAffineTransform::identity;
        }, EasingFunction::linear, [this, oldView, otherViewController](bool res) {
            // Reset old View state and remove
            otherViewController->getView()->transform = NXAffineTransform::identity;
            oldView->getView()->alpha = 1;
            oldView->getView()->removeFromSuperview();
            oldView->removeFromParent();

            // Finish adding new View
            otherViewController->didMoveToParent(shared_from_this());

            otherViewController->getView()->setGrow(1);
            otherViewController->getView()->setShrink(1);

            // Move focus to new View
            Application::shared()->setFocus(otherViewController->getView()->getDefaultFocus());
        });
    }
}

std::shared_ptr<UIViewController> UINavigationController::popViewController(bool animated, bool free) {
    if (viewControllers.size() <= 1) return nullptr;

    auto oldTop = viewControllers.back();
    viewControllers.pop_back();
    oldTop->willMoveToParent(nullptr);

    auto newTop = viewControllers.back();

    navigationBar->pushNavigationItem(newTop->getNavigationItem());

    if (!animated) {
        oldTop->getView()->removeFromSuperview();
        oldTop->removeFromParent();

        addChild(viewControllers.back());
        getView()->insertSubview(viewControllers.back()->getView(), 0);
        viewControllers.back()->didMoveToParent(shared_from_this());
        Application::shared()->setFocus(viewControllers.back()->getView()->getDefaultFocus());

//        if (free) delete oldTop;
    } else {
        addChild(viewControllers.back());
        getView()->insertSubview(viewControllers.back()->getView(), 0);


        newTop->getView()->alpha = 0;
        newTop->getView()->transform = NXAffineTransform::translationBy(-20, 0);

        UIView::animate({ oldTop->getView(), viewControllers.back()->getView() }, 0.1f, [this, oldTop, newTop]() {
            newTop->getView()->alpha = 1;
            newTop->getView()->transform = NXAffineTransform::identity;

            oldTop->getView()->alpha = 0;
            oldTop->getView()->transform = NXAffineTransform::translationBy(20, 0);
        }, EasingFunction::linear, [this, oldTop, newTop, free](bool res) {
            oldTop->getView()->removeFromSuperview();
            oldTop->removeFromParent();

            viewControllers.back()->didMoveToParent(shared_from_this());
            Application::shared()->setFocus(viewControllers.back()->getView()->getDefaultFocus());

//            if (free) delete oldTop;
        });
    }

    return oldTop;
}

void UINavigationController::childNavigationItemDidChange(std::shared_ptr<UIViewController> controller) {
    navigationBar->pushNavigationItem(viewControllers.back()->getNavigationItem());
}

}
