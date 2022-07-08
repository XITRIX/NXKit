//
//  UIViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIViewController.hpp"

UIView* UIViewController::getView() {
    loadViewIfNeeded();
    return view;
}

void UIViewController::setView(UIView* view) {
    this->view = view;
    view->next = this;
    viewDidLoad();
}

void UIViewController::loadView() {
    setView(new UIView());
}

void UIViewController::loadViewIfNeeded() {
    if (view == nullptr) {
        loadView();
    }
}
