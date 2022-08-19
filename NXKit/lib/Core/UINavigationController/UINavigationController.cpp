//
//  UINavigationController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include "UINavigationController.hpp"
#include "UIStackView.hpp"
#include "UIBlurView.hpp"
#include "UILabel.hpp"

namespace NXKit {

UINavigationController::UINavigationController(UIViewController* rootController):
    rootController(rootController)
{ }

void UINavigationController::loadView() {
    UIView* root = new UIView();
    root->backgroundColor = UIColor(235, 235, 235);

    overlay = new UIStackView(Axis::VERTICAL);
    overlay->setJustifyContent(JustifyContent::SPACE_BETWEEN);
    //    root->backgroundColor = RED;
    //    root->cornerRadius = 40;

    float headerHeight = 88;
    float footerHeight = 73;
    setAdditionalSafeAreaInsets(UIEdgeInsets(headerHeight, 0, 0, 0));

    UIBlurView *header = new UIBlurView();
    header->setAxis(Axis::HORIZONTAL);
    header->setSize(Size(UIView::AUTO, headerHeight));
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

    UIBlurView *footer = new UIBlurView();
    footer->setAxis(Axis::HORIZONTAL);
    //        footer->backgroundColor = UIColor(255, 255, 0);
    footer->setSize(Size(UIView::AUTO, footerHeight));
    footer->setBorderTop(1);
    footer->setPaddingLeft(35);
    footer->setPaddingRight(35);
    footer->setMarginLeft(30);
    footer->setMarginRight(30);
    footer->setShrink(0);

    overlay->addSubview(header);
    
    addChild(rootController);
    root->addSubview(rootController->getView());
    rootController->didMoveToParent(this);

    rootController->getView()->setGrow(1);
    rootController->getView()->setShrink(1);
    overlay->addSubview(footer);

    //    UIView* view = new UIView();
    //    view->addSubview(root);

    root->addSubview(overlay);
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
    overlay->setSize(getView()->frame.size());
    printf("Width: %f, Height: %f", getView()->frame.size().width, getView()->frame.size().height);
    rootController->getView()->setSize(getView()->frame.size());
    //    getView()->setSize(getView()->frame.size());
}

}
