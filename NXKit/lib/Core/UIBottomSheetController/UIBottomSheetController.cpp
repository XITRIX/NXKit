//
//  UIBottomSheetController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 14.09.2022.
//

#include <Core/UIBottomSheetController/UIBottomSheetController.hpp>

namespace NXKit {

UIBottomSheetController::UIBottomSheetController(std::shared_ptr<UIViewController> rootViewController):
    rootViewController(rootViewController)
{}

void UIBottomSheetController::loadView() {
    auto view = std::make_shared<UIStackView>(Axis::VERTICAL);

    view->backgroundColor = UIColor::black.withAlphaComponent(0.6f);
    view->setJustifyContent(JustifyContent::FLEX_END);
    view->setAlignItems(AlignItems::STRETCH);

//    rootViewController->
    addChild(rootViewController);
    view->addSubview(rootViewController->getView());
    rootViewController->didMoveToParent(shared_from_this());

    rootViewController->getView()->setPercentHeight(80);
    
    setView(view);
}

void UIBottomSheetController::makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion) {
    getView()->alpha = 0;
//    alert->transformSize = { 0.98f, 0.98f };

    UIView::animate({ getView() }, 0.15f, [this]() {
        getView()->alpha = 1;
//        alert->transformSize = { 1, 1 };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion();
    });
}

void UIBottomSheetController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    UIView::animate({ getView() }, 0.15f, [this]() {
        getView()->alpha = 0;
//        alert->transformSize = { 0.98f, 0.98f };
    }, EasingFunction::quadraticOut, [completion](bool res) {
        completion(true);
    });
}

}
