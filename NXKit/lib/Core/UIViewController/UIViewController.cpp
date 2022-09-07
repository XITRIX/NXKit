//
//  UIViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIWindow/UIWindow.hpp>

#include <typeinfo>

namespace NXKit {

UIViewController::~UIViewController() {
    for (auto child: children) {
        delete child;
    }

    if (view) {
        view->removeFromSuperview();
        delete view;
    }
}

UIView* UIViewController::getView() {
    loadViewIfNeeded();
    return view;
}

void UIViewController::setView(UIView* view) {
    this->view = view;
    this->view->controller = this;
    viewDidLoad();
}

void UIViewController::loadView() {
    setView(new UIView());
}

void UIViewController::loadViewIfNeeded() {
    if (!view) {
        loadView();
    }
}

bool UIViewController::isViewLoaded() {
    return view != nullptr;
}

UIResponder* UIViewController::getNext() {
    auto window = dynamic_cast<UIWindow*>(view->getSuperview());
    if (window) return window;
    if (parent) return parent;
    return nullptr;
}

void UIViewController::setTitle(std::string title) {
    this->title = title;
}

void UIViewController::addChild(UIViewController* child) {
    children.push_back(child);
    child->willMoveToParent(this);
    child->viewWillAppear(true);
}

void UIViewController::willMoveToParent(UIViewController* parent) {
    if (parent)
        this->parent = parent;
}

void UIViewController::didMoveToParent(UIViewController* parent) {
    viewDidAppear(true);
}

void UIViewController::removeFromParent() {
    parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this));
    viewDidDisappear(true);
}

void UIViewController::setAdditionalSafeAreaInsets(UIEdgeInsets insets) {
    additionalSafeAreaInsets = insets;
}

// MARK: - Presenting
UIViewController* UIViewController::getPresentedViewController() {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent();
    }
    return parent->presentedViewController;
}

UIViewController* UIViewController::getPresentingViewController() {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent();
    }
    return parent->presentingViewController;
}

void UIViewController::setPresentedViewController(UIViewController* presentedViewController) {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent();
    }
    parent->presentedViewController = presentedViewController;
}

void UIViewController::setPresentingViewController(UIViewController* presentingViewController) {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent();
    }
    parent->presentingViewController = presentingViewController;
}

void UIViewController::present(UIViewController* controller, bool animated, std::function<void()> completion) {
    if (parent) {
        parent->present(controller, animated, completion);
        return;
    }

    if (getPresentedViewController()) {
        printf("Warning: attempted to present \(controller), but \(this) is already presenting another view controller. Ignoring request.");
        return;
    }

    if (controller->getPresentingViewController()) {
        printf("Tried to present \(controller) but it is already being presented by \(controller->getPresentingViewController!)");
        return;
    }

    setPresentedViewController(controller);
    controller->setPresentingViewController(this);

    controller->viewWillAppear(animated);

    auto window = getView()->getWindow();
    window->addSubview(controller->getView());
    window->addPresentedViewController(controller);
    Application::shared()->setFocus(controller->getView()->getDefaultFocus());

    controller->makeViewAppear(animated, this, [controller, animated, completion]() {
        controller->viewDidAppear(animated);
        completion();
    });
}

void UIViewController::dismiss(bool animated, std::function<void()> completion) {
    if (parent) {
        parent->dismiss(animated, completion);
        return;
    }

    if (dismissing) return;
    dismissing = true;

    viewWillDisappear(animated);
    presentingViewController->setPresentedViewController(nullptr);
    Application::shared()->setFocus(presentingViewController->getView()->getDefaultFocus());

    makeViewDisappear(animated, [this, animated, completion](bool res) {
        getView()->getWindow()->removePresentedViewController(this);
        getView()->removeFromSuperview();
        viewDidDisappear(animated);
        completion();
        setPresentingViewController(nullptr);
        delete this;
    });
}

void UIViewController::makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion) {
    // Animation could be added
    getView()->transformOrigin = { 0, 720 };
//    getView()->alpha = 0;
    getView()->animate(0.2f, [this]() {
        getView()->transformOrigin = { 0, 0 };
//        getView()->alpha = 1.0f;
    }, EasingFunction::quadraticInOut, [presentingViewController, completion](bool res) {
        presentingViewController->getView()->setHidden(true);
        completion();
    });
}

void UIViewController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    // Animation could be added
//    getView()->transformOrigin = { 0, 0 };
    this->getPresentingViewController()->getView()->setHidden(false);
    getView()->animate(0.2f, [this]() {
        getView()->transformOrigin = { 0, 720 };
//        getView()->alpha = 0;
    }, EasingFunction::quadraticInOut, [this, completion](bool res) {
        completion(true);
    });
}

}
