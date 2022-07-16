//
//  UINavigationController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include "UINavigationController.hpp"
#include "UIStackView.hpp"
#include "UILabel.hpp"

UINavigationController::UINavigationController(UIViewController* rootController):
    rootController(rootController)
{ }

void UINavigationController::loadView() {
    UIStackView *root = new UIStackView(Axis::VERTICAL);
    root->backgroundColor = UIColor(235, 235, 235);
//    root->backgroundColor = RED;
//    root->cornerRadius = 40;

    UIStackView *header = new UIStackView(Axis::HORIZONTAL);
//        header->backgroundColor = UIColor(255, 0, 0);
    header->setSize(Size(UIView::AUTO, 88));
    header->setBorderBottom(1);
    header->setPaddingLeft(35);
    header->setPaddingRight(35);
    header->setMarginLeft(30);
    header->setMarginRight(30);
    header->setJustifyContent(JustifyContent::FLEX_START);
    header->setAlignItems(AlignItems::CENTER);
    header->setShrink(0);

    UILabel *headerLabel = new UILabel("Demo app");
    headerLabel->getFont()->fontSize = 28;
    headerLabel->setMarginTop(7);
    headerLabel->backgroundColor = UIColor(255, 0, 0);
//
    header->addSubview(headerLabel);

    UIStackView *footer = new UIStackView(Axis::HORIZONTAL);
//        footer->backgroundColor = UIColor(255, 255, 0);
    footer->setSize(Size(UIView::AUTO, 73));
    footer->setBorderTop(1);
    footer->setPaddingLeft(35);
    footer->setPaddingRight(35);
    footer->setMarginLeft(30);
    footer->setMarginRight(30);
    footer->setShrink(0);

    root->addSubview(header);
    root->addSubview(rootController->getView());
    rootController->getView()->setGrow(1);
    rootController->getView()->setShrink(1);
    root->addSubview(footer);

//    UIView* view = new UIView();
//    view->addSubview(root);

    setView(root);
}

void UINavigationController::viewDidLoad() {
    getView()->clipToBounds = true;

//        test->transformOrigin.reset();
//        test->transformOrigin.addStep({100, 100}, 1200);
//        test->transformOrigin.start();

//        getView()->transformSize = Size(0.8f, 0.8f);
}

void UINavigationController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
//    getView()->setSize(getView()->frame.size());
}
