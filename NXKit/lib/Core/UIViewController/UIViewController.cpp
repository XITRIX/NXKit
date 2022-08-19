//
//  UIViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIViewController.hpp"
#include "UIWindow.hpp"

#include <typeinfo>

namespace NXKit {

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

UIResponder* UIViewController::getNext() {
    auto window = dynamic_cast<UIWindow*>(view->getSuperview());
    if (window) return window;
    if (parent) return parent;
    return nullptr;
}

void UIViewController::addChild(UIViewController* child) {
    children.push_back(child);
    child->willMoveToParent(this);
    child->viewWillAppear(true);
}

void UIViewController::willMoveToParent(UIViewController* parent) {
    this->parent = parent;
}

void UIViewController::didMoveToParent(UIViewController* parent) {
    viewDidAppear(true);
}

void UIViewController::removeFromParent() {
    std::remove(parent->children.begin(), parent->children.end(), this);
    viewDidDisappear(true);
}

void UIViewController::setAdditionalSafeAreaInsets(UIEdgeInsets insets) {
    additionalSafeAreaInsets = insets;
}

}
