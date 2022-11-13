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
    auto view = NXKit::make_shared<UIStackView>();
    view->setAxis(Axis::VERTICAL);

    view->setJustifyContent(JustifyContent::FLEX_END);
    view->setAlignItems(AlignItems::STRETCH);

    setView(view);

    view->addSubview(makeContentView());
    view->addSubview(makeFooter());
}

std::shared_ptr<UIView> UISelectorViewController::makeContentView() {
    auto navigationBar = NXKit::make_shared<UINavigationBar>();
    navigationBar->setSize(Size(UIView::AUTO, headerHeight));
    navigationBar->titleLabel->getFont()->fontSize = 24;
    navigationBar->pushNavigationItem({ .title = title, .image = nullptr });

    auto container = NXKit::make_shared<UIStackView>(Axis::VERTICAL);
    container->backgroundColor = UIColor::systemBackground;
    container->showShadow = true;

    auto scroll = NXKit::make_shared<UIScrollView>();
    scroll->setFixWidth(true);

    auto view = NXKit::make_shared<UIStackView>(Axis::VERTICAL);
    view->setAlignItems(AlignItems::CENTER);
    view->setPadding(32, 80, 32, 40);

    for (int i = 0; i < data.size(); i++) {
        auto cell = NXKit::make_shared<UITableViewRadioCell>();
        if (i == selectedIndex) selectedView = cell;
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

std::shared_ptr<UIView> UISelectorViewController::makeFooter() {
    auto container = NXKit::make_shared<UIStackView>();
    container->backgroundColor = UIColor::systemBackground;

    auto footer = NXKit::make_shared<UIStackView>();
    footer->setAxis(Axis::HORIZONTAL);
    footer->setAlignItems(AlignItems::STRETCH);
    footer->setJustifyContent(JustifyContent::FLEX_END);
    footer->setBorderTop(1);
    footer->borderColor = UIColor::label;
    footer->setMarginLeft(30);
    footer->setMarginRight(30);
    footer->setPadding(4, 8, 4, 8);

    auto actions = NXKit::make_shared<UIActionsView>();
    actions->setSize(Size(UIView::AUTO, footerHeight));
    actions->inController = shared_from_this();
    footer->addSubview(actions);
    acctionsBar = actions;

    container->addSubview(footer);

    return container;
}

void UISelectorViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();

    float heightLimit = getView()->getBounds().size.height * 0.6f;

    contentView->layoutIfNeeded();
    float contHeight = contentView->getBounds().size.height;
    
    float height = std::min(heightLimit, contHeight);
    if (!floatSame(scrollView->getBounds().size.height, height)) {
        scrollView->setHeight(height);

        if (selectedView) {
            Application::shared()->setFocus(selectedView);

            Rect focusedViewFrame = selectedView->getFrame();
            Point origin = scrollView->convert(focusedViewFrame.origin, selectedView->getSuperview().lock());
            origin.y -= height / 2 - focusedViewFrame.height() / 2;

            scrollView->setContentOffset(origin, true);
        }
    }

    auto insets = getView()->safeAreaInsets();
    acctionsBar->setMarginBottom(insets.bottom);
}

void UISelectorViewController::makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion) {
    getView()->setNeedsLayout();
    containerView->transform = NXAffineTransform::translationBy(0, 20);
    containerView->alpha = 0;
    UIView::animate({ getView(), containerView }, 0.15f, [this]() {
        getView()->backgroundColor = UIColor::black.withAlphaComponent(0.6f);
        containerView->transform = NXAffineTransform::identity;
        containerView->alpha = 1;
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion();
    });
}

void UISelectorViewController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    UIView::animate({ getView(), containerView }, 0.15f, [this]() {
        getView()->backgroundColor = UIColor::clear;
        containerView->transform = NXAffineTransform::translationBy(0, 20);
        containerView->alpha = 0;
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion(true);
    });
}


}
