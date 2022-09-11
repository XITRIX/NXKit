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
#include <Core/UIActionsView/UIActionsView.hpp>
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

    titleLabel = new UILabel("Demo app");
    imageView = new UIImageView();
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

UINavigationController::UINavigationController(UIViewController* rootController) {
    pushViewController(rootController, false);
}

UINavigationController::~UINavigationController() {
    for (auto vc: viewControllers) {
        if (vc->getParent() != this)
            delete vc;
    }
}

void UINavigationController::loadView() {
    UIView* root = new UIView();
    setView(root);

    root->backgroundColor = UIColor::systemBackground;

    overlay = new UIStackView(Axis::VERTICAL);
    overlay->passthroughTouches = true;
    overlay->setJustifyContent(JustifyContent::SPACE_BETWEEN);

    setAdditionalSafeAreaInsets(UIEdgeInsets(headerHeight, 0, footerHeight, 0));

    UIBlurView* blurHeader = new UIBlurView();
    navigationBar = new UINavigationBar();
    navigationBar->setSize(Size(UIView::AUTO, headerHeight));
    blurHeader->addSubview(navigationBar);

    overlay->addSubview(blurHeader);
    overlay->addSubview(buildFooter());

    root->addSubview(overlay);
}

UIView* UINavigationController::buildFooter() {
    UIBlurView* blurFooter = new UIBlurView();
    UIStackView *footer = new UIStackView();
    footer->setAxis(Axis::HORIZONTAL);
    footer->setAlignItems(AlignItems::STRETCH);
    footer->setJustifyContent(JustifyContent::FLEX_END);
    footer->setSize(Size(UIView::AUTO, footerHeight));
    footer->setBorderTop(1);
    footer->borderColor = UIColor::label;
    footer->setMarginLeft(30);
    footer->setMarginRight(30);
    footer->setPadding(4, 8, 4, 8);

    auto actions = new UIActionsView();
    actions->inController = this;
    footer->addSubview(actions);

    blurFooter->addSubview(footer);
    return blurFooter;
}

void UINavigationController::viewDidLoad() {
    getView()->clipToBounds = true;

    addAction(BUTTON_B, UIAction([this]() {
        popViewController(true, true);
    }, "Back"));
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
}

void UINavigationController::show(UIViewController* controller, void* sender) {
    pushViewController(controller, true);
}

void UINavigationController::pushViewController(UIViewController* otherViewController, bool animated) {
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
        otherViewController->didMoveToParent(this);
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
        otherViewController->getView()->transformOrigin = Point(20, 0);

        // Remove focus from old View
        Application::shared()->setFocus(nullptr);

        UIView::animate({ otherViewController->getView(), oldView->getView() }, 0.1f, [oldView, otherViewController]() {
            // Animate old View
            oldView->getView()->transformOrigin = Point(-20, 0);
            oldView->getView()->alpha = 0;

            // Animate new View
            otherViewController->getView()->alpha = 1;
            otherViewController->getView()->transformOrigin = Point();
        }, EasingFunction::linear, [this, oldView, otherViewController](bool res) {
            // Reset old View state and remove
            oldView->getView()->transformOrigin = Point();
            oldView->getView()->alpha = 1;
            oldView->getView()->removeFromSuperview();
            oldView->removeFromParent();

            // Finish adding new View
            otherViewController->didMoveToParent(this);

            otherViewController->getView()->setGrow(1);
            otherViewController->getView()->setShrink(1);

            // Move focus to new View
            Application::shared()->setFocus(otherViewController->getView()->getDefaultFocus());
        });
    }
}

UIViewController* UINavigationController::popViewController(bool animated, bool free) {
    if (viewControllers.size() <= 1) return nullptr;

    UIViewController* oldTop = viewControllers.back();
    viewControllers.pop_back();
    oldTop->willMoveToParent(nullptr);

    UIViewController* newTop = viewControllers.back();

    navigationBar->pushNavigationItem(newTop->getNavigationItem());

    if (!animated) {
        oldTop->getView()->removeFromSuperview();
        oldTop->removeFromParent();

        addChild(viewControllers.back());
        getView()->insertSubview(viewControllers.back()->getView(), 0);
        viewControllers.back()->didMoveToParent(this);
        Application::shared()->setFocus(viewControllers.back()->getView()->getDefaultFocus());

        if (free) delete oldTop;
    } else {
        addChild(viewControllers.back());
        getView()->insertSubview(viewControllers.back()->getView(), 0);


        newTop->getView()->alpha = 0;
        newTop->getView()->transformOrigin = Point(-20, 0);

        UIView::animate({ oldTop->getView(), viewControllers.back()->getView() }, 0.1f, [this, oldTop, newTop]() {
            newTop->getView()->alpha = 1;
            newTop->getView()->transformOrigin = Point(0, 0);

            oldTop->getView()->alpha = 0;
            oldTop->getView()->transformOrigin = Point(20, 0);
        }, EasingFunction::linear, [this, oldTop, newTop, free](bool res) {
            oldTop->getView()->removeFromSuperview();
            oldTop->removeFromParent();

            viewControllers.back()->didMoveToParent(this);
            Application::shared()->setFocus(viewControllers.back()->getView()->getDefaultFocus());

            if (free) delete oldTop;
        });
    }

    return oldTop;
}

void UINavigationController::childNavigationItemDidChange(UIViewController* controller) {
    navigationBar->pushNavigationItem(viewControllers.back()->getNavigationItem());
}

}
