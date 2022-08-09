//
//  UIWindow.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 09.07.2022.
//

#include "UIWindow.hpp"
#include "Application.hpp"

namespace NXKit {

UIViewController* UIWindow::getRootViewController() {
    return rootViewController;
}

void UIWindow::setRootViewController(UIViewController *viewController) {
    rootViewController = viewController;
}

void UIWindow::makeKeyAndVisible() {
    Application::shared()->setKeyWindow(this);

    if (rootViewController) {
        rootViewController->loadViewIfNeeded();
        rootViewController->viewWillAppear(false);
        addSubview(rootViewController->getView());
        layoutSubviews();
        rootViewController->viewDidAppear(false);
    }
}

void UIWindow::sendEvent(UIEvent* event) {
    auto allTouches = event->allTouches;

    for (auto touch: allTouches) {
        switch (touch->phase) {
            case UITouchPhase::BEGIN:
                touch->runTouchActionOnRecognizerHierachy([touch, event](UIGestureRecognizer* recognizer) { recognizer->touchesBegan({ touch }, event); });

                if (!touch->hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesBegan({ touch }, event);
                }
                break;
            case UITouchPhase::MOVED:
                if (touch->previousAbsoluteLocation == touch->absoluteLocation) break;
                
                touch->runTouchActionOnRecognizerHierachy([touch, event](UIGestureRecognizer* recognizer) { recognizer->touchesMoved({ touch }, event); });

                if (!touch->hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesMoved({ touch }, event);
                }
                break;
            case UITouchPhase::ENDED:
                bool hasBeenCancelledByAGestureRecognizer = touch->hasBeenCancelledByAGestureRecognizer;
                touch->runTouchActionOnRecognizerHierachy([touch, event](UIGestureRecognizer* recognizer) { recognizer->touchesEnded({ touch }, event); });

                if (!hasBeenCancelledByAGestureRecognizer) {
                    touch->view->touchesEnded({ touch }, event);
                }
                break;
        }
    }
}

void UIWindow::layoutSubviews() {
    UIView::layoutSubviews();
    rootViewController->getView()->setSize(((Rect) this->frame).size);
}

}
