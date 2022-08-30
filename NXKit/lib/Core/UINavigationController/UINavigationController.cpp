//
//  UINavigationController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 15.07.2022.
//

#include <Core/UINavigationController/UINavigationController.hpp>
#include <Core/UIStackView/UIStackView.hpp>
#include <Core/UIBlurView/UIBlurView.hpp>
#include <Core/UILabel/UILabel.hpp>
#include <Core/UIImageView/UIImageView.hpp>

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
    setAdditionalSafeAreaInsets(UIEdgeInsets(headerHeight, 0, footerHeight, 0));

    UIBlurView* blurHeader = new UIBlurView();
    UIStackView *header = new UIStackView();
    header->setSize(Size(UIView::AUTO, headerHeight));
    header->setAxis(Axis::HORIZONTAL);
    header->setBorderBottom(1);
    header->setPaddingLeft(35);
    header->setPaddingRight(35);
    header->setMarginLeft(30);
    header->setMarginRight(30);
    header->setJustifyContent(JustifyContent::FLEX_START);
    header->setAlignItems(AlignItems::CENTER);

    UILabel *headerLabel = new UILabel("Demo app");
    UIImageView* imageView = new UIImageView();
    imageView->setSize(Size(48, 48));
    imageView->setMarginRight(18);
    imageView->setImage(new UIImage("Images/logo.png"_res, 2));
//    imageView->setScalingType(ImageScalingType::CENTER);

    headerLabel->getFont()->fontSize = 28;
    headerLabel->setMarginTop(7);
//    headerLabel->backgroundColor = UIColor(255, 0, 0);

    header->addSubview(imageView);
    header->addSubview(headerLabel);
    blurHeader->addSubview(header);

    UIBlurView* blurFooter = new UIBlurView();
    UIStackView *footer = new UIStackView();
    footer->setAxis(Axis::HORIZONTAL);
    //        footer->backgroundColor = UIColor(255, 255, 0);
    footer->setSize(Size(UIView::AUTO, footerHeight));
    footer->setBorderTop(1);
    footer->setPaddingLeft(35);
    footer->setPaddingRight(35);
    footer->setMarginLeft(30);
    footer->setMarginRight(30);

    overlay->addSubview(blurHeader);
    
    addChild(rootController);
    root->addSubview(rootController->getView());
    rootController->didMoveToParent(this);

    rootController->getView()->setGrow(1);
    rootController->getView()->setShrink(1);

    blurFooter->addSubview(footer);
    overlay->addSubview(blurFooter);

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
    overlay->setSize(getView()->getFrame().size);
    printf("Width: %f, Height: %f\n", getView()->getFrame().size.width, getView()->getFrame().size.height);
    rootController->getView()->setSize(getView()->getFrame().size);
    //    getView()->setSize(getView()->getFrame().size);
}

}
