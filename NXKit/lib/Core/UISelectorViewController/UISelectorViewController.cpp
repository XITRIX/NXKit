//
//  UISelectorViewController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 14.09.2022.
//

#include <Core/UISelectorViewController/UISelectorViewController.hpp>
#include <Core/UINavigationController/UINavigationController.hpp>
#include <Core/UITableViewRadioCell/UITableViewRadioCell.hpp>
#include <Core/UIScrollView/UIScrollView.hpp>
#include <Core/UIActionsView/UIActionsView.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UISelectorViewController::UISelectorViewController(std::string title, std::vector<std::string> data, std::function<void(int)> onComplete, int selectedIndex)
    : title(title), data(data), onComplete(onComplete), selectedIndex(selectedIndex)
{ }

void UISelectorViewController::loadView() {
    UIStackView* view = new UIStackView();
    view->setAxis(Axis::VERTICAL);

    view->setJustifyContent(JustifyContent::FLEX_END);
    view->setAlignItems(AlignItems::STRETCH);

    setView(view);

    view->addSubview(makeContentView());
    view->addSubview(makeFooter());
}

void UISelectorViewController::viewWillAppear(bool animated) {
    if (selectedView) {
        getView()->layoutIfNeeded();
        Application::shared()->setFocus(selectedView);
    }
}

UIView* UISelectorViewController::makeContentView() {
    UINavigationBar* navigationBar = new UINavigationBar();
    navigationBar->setSize(Size(UIView::AUTO, headerHeight));
    navigationBar->titleLabel->getFont()->fontSize = 24;
    navigationBar->pushNavigationItem({ .title = title, .image = nullptr });

    UIStackView* container = new UIStackView(Axis::VERTICAL);
    container->backgroundColor = UIColor::systemBackground;
    container->showShadow = true;

    UIScrollView* scroll = new UIScrollView();
    scroll->setFixWidth(true);

    UIStackView* view = new UIStackView(Axis::VERTICAL);
    view->setAlignItems(AlignItems::CENTER);
    view->setPadding(32, 80, 32, 40);

    for (int i = 0; i < data.size(); i++) {
        UITableViewRadioCell* cell = new UITableViewRadioCell();
        if (i == selectedIndex) selectedView = cell
            ;
        cell->setHeight(60);
        cell->setPercentWidth(60);
        cell->label->getFont()->fontSize = 20;
        if (i == 0) cell->setBorderTop(1);
        cell->setBorderBottom(1);
        cell->borderColor = UIColor::separator;
        cell->setText(data[i]);
        cell->setOn(i == selectedIndex);
        cell->setImage(nullptr);

        cell->addAction(BUTTON_A, UIAction([this, i]() {
            dismiss(true, [this, i]() {
                onComplete(i);
            });
        }, "OK"));

        view->addSubview(cell);
    }
    scroll->addSubview(view);

    container->addSubview(navigationBar);
    container->addSubview(scroll);

    container->setMarginBottom(-footerHeight);
    container->setPaddingBottom(footerHeight);

    contentView = view;
    scrollView = scroll;
    containerView = container;

    return container;
}

UIView* UISelectorViewController::makeFooter() {
    UIStackView *container = new UIStackView();
    container->backgroundColor = UIColor::systemBackground;

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

    container->addSubview(footer);

    return container;
}

void UISelectorViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    float heightLimit = getView()->getBounds().size.height * 0.6f;

    contentView->layoutIfNeeded();
    float contHeight = contentView->getBounds().size.height;
    
    float height = std::min(heightLimit, contHeight);
    if (!floatSame(scrollView->getBounds().size.height, height))
        scrollView->setHeight(height);
}

void UISelectorViewController::makeViewAppear(bool animated, UIViewController* presentingViewController, std::function<void()> completion) {
    getView()->setNeedsLayout();
    containerView->transformOrigin = { 0, 20 };
    containerView->alpha = 0;
    UIView::animate({ getView(), containerView }, 0.15f, [this]() {
        getView()->backgroundColor = UIColor::black.withAlphaComponent(0.6f);
        containerView->transformOrigin = { 0, 0 };
        containerView->alpha = 1;
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion();
    });
}

void UISelectorViewController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    UIView::animate({ getView(), containerView }, 0.15f, [this]() {
        getView()->backgroundColor = UIColor::clear;
        containerView->transformOrigin = { 0, 20 };
        containerView->alpha = 0;
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion(true);
    });
}


}