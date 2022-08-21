//
//  UITabBarController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include <Core/UITabBarController/UITabBarController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/Application/Application.hpp>
#include <Core/UIScrollView/UIScrollView.hpp>
#include <Core/UITapGestureRecognizer/UITapGestureRecognizer.hpp>
#include <Core/UILabel/UILabel.hpp>

namespace NXKit {

UITabBarItemView::UITabBarItemView(UITabBarController* parent, UIViewController* controller):
    parent(parent), controller(controller)
{
    setAxis(Axis::HORIZONTAL);
    setHeight(70);

    selectionBar = new UIView();
    selectionBar->backgroundColor = UIColor(49, 79, 235);
    selectionBar->setWidth(4);
    selectionBar->setMargins(9, 8, 9, 8);

    label = new UILabel("Test Item");
    label->setGrow(1);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;
    label->setMargins(9, 8, 9, 0);

    addSubview(selectionBar);
    addSubview(label);

    setSelected(false);
    canBecomeFocused = true;

    auto tap = new UITapGestureRecognizer();
    tap->onStateChanged = [this](UIGestureRecognizerState state) {
        switch (state) {
            case UIGestureRecognizerState::ENDED:
                Application::shared()->setFocus(this);
                break;

            default: break;
        }
    };
    addGestureRecognizer(tap);
}

void UITabBarItemView::setTitle(std::string text) {
    label->setText(text);
}

bool UITabBarItemView::isSelected() {
    return selected;
}

void UITabBarItemView::becomeFocused() {
    parent->setSelected(this);
}

void UITabBarItemView::setSelected(bool selected) {
    this->selected = selected;
    if (selected) {
        label->textColor = UIColor(49, 79, 235);
        selectionBar->backgroundColor = UIColor(49, 79, 235);
    } else {
        label->textColor = UIColor(0, 0, 0);
        selectionBar->backgroundColor = UIColor(0, 0, 0, 0);
    }
}

UITabBarController::UITabBarController() {}
UITabBarController::UITabBarController(std::vector<UIViewController*> controllers):
    viewControllers(controllers)
{}

void UITabBarController::loadView() {
    UIStackView* view = new UIStackView(Axis::HORIZONTAL);

    tabs = new UIStackView(Axis::VERTICAL);
    tabs->setPadding(32, 40, 47, 80);

    contentView = new UIView();
    contentView->setGrow(1);

    UIScrollView* scrollView = new UIScrollView();
    scrollView->setFixWidth(true);
    scrollView->setPercentWidth(32);
    scrollView->backgroundColor = UIColor(240, 240, 240);
    scrollView->addSubview(tabs);

    view->addSubview(scrollView);
    view->addSubview(contentView);

    setView(view);
}

void UITabBarController::viewDidLoad() {
    reloadViewForViewControllers();
    tabs->clipToBounds = true;
}

void UITabBarController::setSelected(UITabBarItemView* item) {
    size_t newIndex = std::find(tabViews.begin(), tabViews.end(), item) - tabViews.begin();
    if (selectedIndex == newIndex) return;

    if (selectedIndex >= 0 && selectedIndex < tabViews.size()) {
        tabViews[selectedIndex]->setSelected(false);

        tabViews[selectedIndex]->controller->willMoveToParent(nullptr);
        tabViews[selectedIndex]->controller->getView()->removeFromSuperview();
        tabViews[selectedIndex]->controller->removeFromParent();
    }

    item->setSelected(true);
    selectedIndex = (int) newIndex;

    addChild(item->controller);
    contentView->addSubview(item->controller->getView());
    item->controller->didMoveToParent(this);

    content = item->controller;
}

void UITabBarController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    content->getView()->setSize(contentView->getFrame().size);
}

void UITabBarController::setViewControllers(std::vector<UIViewController*> controllers) {
    viewControllers = controllers;
    if (isViewLoaded()) reloadViewForViewControllers();
}

void UITabBarController::reloadViewForViewControllers() {
    tabViews.clear();
    for (UIView* tab: tabs->getSubviews()) { tab->removeFromSuperview(); }
    selectedIndex = -1;

    for (int i = 0; i < viewControllers.size(); i++) {
        auto controller = viewControllers[i];
        UITabBarItemView* item = new UITabBarItemView(this, viewControllers[i]);
        item->setTitle(controller->getTitle());
        item->tag = "Num" + std::to_string(i);
        item->setSelected(i == selectedIndex);
        tabViews.push_back(item);
        tabs->addSubview(item);
    }
}

}
