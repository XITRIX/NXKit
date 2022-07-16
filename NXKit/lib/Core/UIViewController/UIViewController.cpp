//
//  UIViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIViewController.hpp"
#include "UIWindow.hpp"

#include <typeinfo>

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
//    if (typeid(view->getSuperview()) == typeid(UIWindow)) {
//        return view->getSuperview();
//    }
    return nullptr;
}
