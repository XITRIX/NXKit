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

UITabBarSeparatorView::UITabBarSeparatorView() {
    setHeight(30);
    setAlignItems(AlignItems::STRETCH);
    setJustifyContent(JustifyContent::CENTER);

    auto line = NXKit::make_shared<UIView>();
    line->setHeight(1);
    line->backgroundColor = UIColor::separator;

    addSubview(line);
}

UITabBarItemView::UITabBarItemView(std::shared_ptr<UITabBarController> parent, std::shared_ptr<UIViewController> controller):
    parent(parent.get()), controller(controller)
{
    setAxis(Axis::HORIZONTAL);
    setHeight(70);

    selectionBar = NXKit::make_shared<UIView>();
    selectionBar->backgroundColor = selectionBar->getTintColor();
    selectionBar->setWidth(4);
    selectionBar->setMargins(9, 8, 9, 8);

    label = NXKit::make_shared<UILabel>("Test Item");
    label->setGrow(1);
    label->verticalAlign = VerticalAlign::CENTER;
    label->getFont()->fontSize = 22;
    label->setMargins(9, 0, 9, 8);

    addSubview(selectionBar);
    addSubview(label);

    setSelected(false);

    onEvent = [this](UIControlTouchEvent event) {
        if (event == UIControlTouchEvent::touchUpInside) {
            Application::shared()->setFocus(shared_from_base<UITabBarItemView>());
        }
    };
//    auto tap = new UITapGestureRecognizer();
//    tap->onStateChanged = [this](UIGestureRecognizerState state) {
//        switch (state) {
//            case UIGestureRecognizerState::ENDED:
//                Application::shared()->setFocus(this);
//                break;
//
//            default: break;
//        }
//    };
//    addGestureRecognizer(tap);
}

UITabBarItemView::~UITabBarItemView() {
//    delete controller;
}

bool UITabBarItemView::isEnabled() {
    return !isSelected();
}

void UITabBarItemView::setTitle(std::string text) {
    label->setText(text);
}

void UITabBarItemView::becomeFocused() {
//    if (!parent.expired())
        parent->setSelected(shared_from_base<UITabBarItemView>());
}

void UITabBarItemView::setSelected(bool selected) {
    UIControl::setSelected(selected);
    if (selected) {
        label->textColor = label->getTintColor();
        selectionBar->backgroundColor = selectionBar->getTintColor();
    } else {
        label->textColor = UIColor::label;
        selectionBar->backgroundColor = UIColor::clear;
    }
}

UITabBarController::UITabBarController() {
    addAction(BUTTON_B, UIAction([this]() {
        if (Application::shared()->getFocus().lock() != tabs->getDefaultFocus())
            Application::shared()->setFocus(tabs->getDefaultFocus());
    }, "Back", true, true, [this]() {
        return Application::shared()->getFocus().lock() != tabs->getDefaultFocus();
    }));
}

UITabBarController::UITabBarController(std::vector<std::shared_ptr<UIViewController>> controllers):
    UITabBarController()
{
    viewControllers = controllers;
}

UITabBarController::~UITabBarController() {
    for (auto vc: viewControllers) {
//        if (vc && vc->getParent().get() != this)
//            delete vc;
    }
}

void UITabBarController::loadView() {
    auto view = NXKit::make_shared<UIStackView>(Axis::HORIZONTAL);
    view->tag = "TabBar stack 2";

    tabs = NXKit::make_shared<UIStackView>(Axis::VERTICAL);
    tabs->tag = "TabBarItems stack";
    tabs->setPadding(32, 40, 47, 80);

    contentView = NXKit::make_shared<UIView>();
    contentView->setGrow(1);

    auto scrollView = NXKit::make_shared<UIScrollView>();
    scrollView->tag = "TabBar scroll";
    scrollView->setFixWidth(true);
    scrollView->setPercentWidth(32);
    scrollView->backgroundColor = UIColor::secondarySystemBackground;
    scrollView->addSubview(tabs);

    view->addSubview(scrollView);
    view->addSubview(contentView);

    setView(view);
}

void UITabBarController::viewDidLoad() {
    reloadViewForViewControllers();
    tabs->clipToBounds = true;
}

void UITabBarController::setSelected(std::shared_ptr<UITabBarItemView> item) {
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
    item->controller->didMoveToParent(shared_from_this());

    content = item->controller;
}

void UITabBarController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    content->getView()->setSize(contentView->getFrame().size);
}

void UITabBarController::setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers) {
    viewControllers = controllers;
    if (isViewLoaded()) reloadViewForViewControllers();
}

void UITabBarController::reloadViewForViewControllers() {
//    for (auto tab: tabViews) delete tab;
    tabViews.clear();
    for (auto tab: tabs->getSubviews()) { tab->removeFromSuperview(); }
    selectedIndex = -1;

    for (int i = 0; i < viewControllers.size(); i++) {
        auto controller = viewControllers[i];
        if (controller) {
            auto item = NXKit::make_shared<UITabBarItemView>(shared_from_base<UITabBarController>(), viewControllers[i]);
            item->setTitle(controller->getTitle());
            item->tag = "Num" + std::to_string(i);
            item->setSelected(i == selectedIndex);
            item->addAction(BUTTON_A, UIAction([this]() {
                auto newFocus = getView()->getNextFocus(NavigationDirection::RIGHT);
                if (newFocus)
                    Application::shared()->setFocus(newFocus);
            }, "OK"));
            tabViews.push_back(item);
            tabs->addSubview(item);
        } else {
            tabs->addSubview(NXKit::make_shared<UITabBarSeparatorView>());
        }
    }

    setSelected(tabViews.front());
}

void UITabBarController::childNavigationItemDidChange(std::shared_ptr<UIViewController> controller) {
    for (auto tab: tabViews) {
        if (tab->controller == controller) {
            tab->label->setText(controller->getTitle());
        }
    }
}

}
