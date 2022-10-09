//
//  UIWindow.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include <Core/UIWindow/UIWindow.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UIWindow::UIWindow() {
    backgroundColor = UIColor::systemBackground;
}

UIWindow::~UIWindow() {
//    delete rootViewController;
}

std::shared_ptr<UIViewController> UIWindow::getRootViewController() {
    return rootViewController;
}

void UIWindow::setRootViewController(std::shared_ptr<UIViewController> viewController) {
    rootViewController = viewController;
}

void UIWindow::makeKeyAndVisible() {
    Application::shared()->setKeyWindow(shared_from_base<UIWindow>());

    if (rootViewController) {
        rootViewController->loadViewIfNeeded();
        rootViewController->viewWillAppear(false);
        addSubview(rootViewController->getView());
        layoutSubviews();
        rootViewController->viewDidAppear(false);
    }
}

void UIWindow::sendEvent(std::shared_ptr<UIEvent> event) {
    auto allTouches = event->allTouches;

    for (auto touch: allTouches) {
        switch (touch->phase) {
            case UITouchPhase::BEGIN:
                touch->runTouchActionOnRecognizerHierachy([touch, event](std::shared_ptr<UIGestureRecognizer> recognizer) { recognizer->_touchesBegan({ touch }, event); });

                if (!touch->hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesBegan({ touch }, event);
                }
                break;
            case UITouchPhase::MOVED:
                if (touch->previousAbsoluteLocation == touch->absoluteLocation) break;
                
                touch->runTouchActionOnRecognizerHierachy([touch, event](std::shared_ptr<UIGestureRecognizer> recognizer) { recognizer->_touchesMoved({ touch }, event); });

                if (!touch->hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesMoved({ touch }, event);
                }
                break;
            case UITouchPhase::ENDED:
                bool hasBeenCancelledByAGestureRecognizer = touch->hasBeenCancelledByAGestureRecognizer;
                touch->runTouchActionOnRecognizerHierachy([touch, event](std::shared_ptr<UIGestureRecognizer> recognizer) { recognizer->_touchesEnded({ touch }, event); });

                if (!hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesEnded({ touch }, event);
                }
                break;
        }
    }
}

void UIWindow::addPresentedViewController(std::shared_ptr<UIViewController> controller) {
    presentedViewControllers.push_back(controller);
}

void UIWindow::removePresentedViewController(std::shared_ptr<UIViewController> controller) {
    presentedViewControllers.erase(std::remove(presentedViewControllers.begin(), presentedViewControllers.end(), controller), presentedViewControllers.end());
}

void UIWindow::layoutSubviews() {
    UIView::layoutSubviews();
    rootViewController->getView()->setSize(getBounds().size);
    for (auto controller: presentedViewControllers) {
        controller->getView()->setSize(getBounds().size);
    }
}

std::shared_ptr<UIView> UIWindow::getDefaultFocus() {
    if (presentedViewControllers.size() > 0) {
        return presentedViewControllers.back()->getView()->getDefaultFocus();
    }
    return UIView::getDefaultFocus();
}

UITraitCollection UIWindow::getTraitCollection() {
    UITraitCollection traits;
    traits.userInterfaceStyle = Application::shared()->getUserInterfaceStyle();
    return traits;
}

}
