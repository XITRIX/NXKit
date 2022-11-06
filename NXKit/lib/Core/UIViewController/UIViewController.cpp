//
//  UIViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include <Core/UIViewController/UIViewController.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UIViewController::UIViewController() {
    addAction(BUTTON_B, UIAction([this]() {
        if (getPresentingViewController())
            dismiss(true);
    }, "Back", true, true, [this]() {
        return parent.expired();
    }));
}

UIViewController::~UIViewController() {
    if (navigationItem.image)
//        delete navigationItem.image;

    for (auto child: children) {
//        delete child;
    }

    if (view) {
        view->removeFromSuperview();
//        delete view;
    }
}

std::shared_ptr<UIView> UIViewController::getView() {
    loadViewIfNeeded();
    return view;
}

void UIViewController::setView(std::shared_ptr<UIView> view) {
    this->view = view;
    this->view->controller = weak_from_this();
    viewDidLoad();
}

void UIViewController::loadView() {
    setView(NXKit::make_shared<UIView>());
}

void UIViewController::loadViewIfNeeded() {
    if (!view) {
        loadView();
    }
}

bool UIViewController::isViewLoaded() {
    return view != nullptr;
}

std::shared_ptr<UIResponder> UIViewController::getNext() {
    auto window = std::dynamic_pointer_cast<UIWindow>(view->getSuperview().lock());
    if (window) return window;
    if (auto spt = parent.lock()) return spt;
    return nullptr;
}

void UIViewController::setTitle(std::string title) {
    navigationItem.title = title;

    if (auto spt = parent.lock()) {
        spt->childNavigationItemDidChange(shared_from_this());
    }
}

void UIViewController::setImage(std::shared_ptr<UIImage> image) {
//    if (navigationItem.image)
//        delete navigationItem.image;
    navigationItem.image = image;

    if (auto spt = parent.lock()) {
        spt->childNavigationItemDidChange(shared_from_this());
    }
}

void UIViewController::addChild(std::shared_ptr<UIViewController> child) {
    children.push_back(child);
    child->willMoveToParent(weak_from_this().lock());
    child->viewWillAppear(true);
}

void UIViewController::willMoveToParent(std::shared_ptr<UIViewController> parent) {
    if (parent)
        this->parent = parent;
}

void UIViewController::didMoveToParent(std::shared_ptr<UIViewController> parent) {
    viewDidAppear(true);
}

void UIViewController::removeFromParent() {
    if (auto spt = parent.lock()) {
        spt->children.erase(std::remove(spt->children.begin(), spt->children.end(), shared_from_this()));
        this->parent.reset();
        viewDidDisappear(true);
    }
}

void UIViewController::setAdditionalSafeAreaInsets(UIEdgeInsets insets) {
    additionalSafeAreaInsets = insets;
}

// MARK: - Presenting
std::shared_ptr<UIViewController> UIViewController::getPresentedViewController() {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent().get();
    }
    return parent->presentedViewController;
}

std::shared_ptr<UIViewController> UIViewController::getPresentingViewController() {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent().get();
    }
    return parent->presentingViewController;
}

void UIViewController::setPresentedViewController(std::shared_ptr<UIViewController> presentedViewController) {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent().get();
    }
    parent->presentedViewController = presentedViewController;
}

void UIViewController::setPresentingViewController(std::shared_ptr<UIViewController> presentingViewController) {
    auto parent = this;
    while (parent->getParent()) {
        parent = parent->getParent().get();
    }
    parent->presentingViewController = presentingViewController;
}

void UIViewController::present(std::shared_ptr<UIViewController> controller, bool animated, std::function<void()> completion) {
    if (!parent.expired()) {
        parent.lock()->present(controller, animated, completion);
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
    controller->setPresentingViewController(shared_from_this());

    auto window = getView()->getWindow();
    window.lock()->addSubview(controller->getView());
    window.lock()->addPresentedViewController(controller);

    controller->viewWillAppear(animated);
    
    Application::shared()->setFocus(controller->getView()->getDefaultFocus());

    controller->makeViewAppear(animated, shared_from_this(), [controller, animated, completion]() {
        controller->viewDidAppear(animated);
        completion();
    });
}

void UIViewController::dismiss(bool animated, std::function<void()> completion) {
    if (!parent.expired()) {
        parent.lock()->dismiss(animated, completion);
        return;
    }

    if (dismissing) return;
    dismissing = true;

    viewWillDisappear(animated);
    presentingViewController->setPresentedViewController(nullptr);
    Application::shared()->setFocus(nullptr);

    makeViewDisappear(animated, [this, animated, completion](bool res) {
        auto controller = shared_from_this();
        getView()->getWindow().lock()->removePresentedViewController(controller);
        getView()->removeFromSuperview();
        viewDidDisappear(animated);
        Application::shared()->setFocus(presentingViewController->getView()->getDefaultFocus());
        completion();
        setPresentingViewController(nullptr);
//        delete this;
    });
}

void UIViewController::show(std::shared_ptr<UIViewController> controller, void* sender) {
    if (!parent.expired())
        return parent.lock()->show(controller, sender);
    present(controller, true);
}

void UIViewController::makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion) {
    // Animation could be added
    getView()->transform = NXAffineTransform::translationBy(0, 720);
//    getView()->alpha = 0;
    getView()->animate(0.2f, [this]() {
        getView()->transform = NXAffineTransform::identity;
//        getView()->alpha = 1.0f;
    }, EasingFunction::quadraticInOut, [presentingViewController, completion](bool res) {
        presentingViewController->getView()->setHidden(true);
        completion();
    });
}

void UIViewController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    // Animation could be added
//    getView()->transformOrigin = { 0, 0 };
    getView()->animate(0.2f, [this]() {
        getPresentingViewController()->getView()->setHidden(false);
        getView()->transform = NXAffineTransform::translationBy(0, 720);
//        getView()->alpha = 0;
    }, EasingFunction::quadraticInOut, [this, completion](bool res) {
        completion(true);
    });
}

UITraitCollection UIViewController::getTraitCollection() {
    UITraitCollection superCollection;
    if (!parent.expired())
        superCollection = parent.lock()->getTraitCollection();

    auto superView = getView()->getSuperview();
    if (!superView.expired()) {
        superCollection = superView.lock()->getTraitCollection();
    }

    if (overrideUserInterfaceStyle != UIUserInterfaceStyle::unspecified)
        superCollection.userInterfaceStyle = overrideUserInterfaceStyle;

    return superCollection;
}

}
