#include <UIWindow.h>
#include <UIApplication.h>
#include <UIViewController.h>
#include <UITouch.h>
#include <UIPress.h>
#include <UIPressesEvent.h>

using namespace NXKit;

std::vector<std::weak_ptr<UIGestureRecognizer>> getRecognizerHierachyFromView(std::shared_ptr<UIView> view) {
    std::vector<std::weak_ptr<UIGestureRecognizer>> recognizers;
    while (view) {
        for (const auto& recognizer : *view->gestureRecognizers())
            recognizers.push_back(recognizer);
        view = view->superview().lock();
    }

    return recognizers;
}

UIWindow::UIWindow() {
    _focusSystem = new_shared<UIFocusSystem>();
    _focusSystem->_rootWindow = weak_from_base<UIWindow>();

    setBackgroundColor(UIColor::systemBackground);
    setTintColor(UIColor::systemBlue);

//    yoga->setEnabled(false);
}

UIWindow::~UIWindow() {
    _presentedViewControllers.clear();
}

std::shared_ptr<UIWindow> UIWindow::window() {
    return shared_from_base<UIWindow>();
}

void UIWindow::setRootViewController(std::shared_ptr<UIViewController> rootViewController) {
    if (_rootViewController) {
        _rootViewController->view()->removeFromSuperview();
    }

    _rootViewController = std::move(rootViewController);
}

void UIWindow::makeKeyAndVisible() {
//    self.safeAreaInsets = UIWindow.getSafeAreaInsets()
    auto window = std::static_pointer_cast<UIWindow>(shared_from_this());
//    window->setBounds(UIRenderer::main()->bounds());
    UIApplication::shared->keyWindow = window;

    auto viewController = _rootViewController;
    if (viewController) {
        viewController->loadViewIfNeeded();
        viewController->view()->setFrame(this->bounds());
        viewController->viewWillAppear(false);
        addSubview(viewController->view());
        viewController->viewDidAppear(false);
        updateFocus();
    }
}

void UIWindow::updateFocus() {
    _focusSystem->updateFocus();
}

void UIWindow::sendEvent(const std::shared_ptr<UIEvent>& event) {
    if (auto pevent = std::dynamic_pointer_cast<UIPressesEvent>(event)) {
        sendPressEvent(pevent);
    } else {
        sendTouchEvent(event);
    }
}

void UIWindow::sendTouchEvent(std::shared_ptr<UIEvent> event) {
    for (auto& touch: event->allTouches()) {
        _inputType = UIWindowInputType::touch;
        _focusSystem->setActive(false);

        auto wHitView = touch->view();
        if (wHitView.expired()) wHitView = hitTest(touch->locationIn(nullptr), nullptr);
        if (wHitView.expired()) continue;
        auto hitView = wHitView.lock();

        switch (touch->phase()) {
            case UITouchPhase::began: {
                touch->_view = hitView;
                touch->_gestureRecognizers = getRecognizerHierachyFromView(hitView);

                touch->runTouchActionOnRecognizerHierachy([touch, event](auto gestureRecognizer) {
                    gestureRecognizer->touchesBegan({ touch }, event);
                });

                if (!touch->hasBeenCancelledByAGestureRecognizer()) {
                    hitView->touchesBegan(event->allTouches(), event);
                }
                break;
            }
            case UITouchPhase::moved: {
                touch->runTouchActionOnRecognizerHierachy([touch, event](auto gestureRecognizer) {
                    gestureRecognizer->touchesMoved({ touch }, event);
                });
                if (!touch->hasBeenCancelledByAGestureRecognizer()) {
                    hitView->touchesMoved(event->allTouches(), event);
                }
                break;
            }
            case UITouchPhase::ended: {
                // compute the value before ending the touch on the recognizer hierachy
                // otherwise `hasBeenCancelledByAGestureRecognizer` will be false because the state was reset already
                auto hasBeenCancelledByAGestureRecognizer = touch->hasBeenCancelledByAGestureRecognizer();

                touch->runTouchActionOnRecognizerHierachy([touch, event](auto gestureRecognizer) {
                    gestureRecognizer->touchesEnded({ touch }, event);
                });

                if (!hasBeenCancelledByAGestureRecognizer) {
                    hitView->touchesEnded(event->allTouches(), event);
                }
                break;
            }
        }
    }
}

void UIWindow::sendPressEvent(const std::shared_ptr<UIPressesEvent>& event) {
    _inputType = UIWindowInputType::focus;
    _focusSystem->setActive(true);

    for (auto& press: event->allPresses()) {
        if (press->responder().expired()) continue;

        switch (press->phase()) {
            case UIPressPhase::began: {
                press->responder().lock()->pressesBegan({ press }, event);
                focusSystem()->sendEvent(event);
                break;
            }
            case UIPressPhase::ended: {
                press->responder().lock()->pressesEnded({ press }, event);
                focusSystem()->sendEvent(event);
                break;
            }
            default:
                break;
        }
    }
}

void UIWindow::layoutSubviews() {
//    setSafeAreaInsets(Platform::getPlatfromSafeArea());

    UIView::layoutSubviews();

    if (_rootViewController) {
        _rootViewController->view()->setFrame(this->bounds());
    }

    for(auto& vc: _presentedViewControllers) {
        vc->view()->setFrame(this->bounds());
    }
}

void UIWindow::addPresentedViewController(const std::shared_ptr<UIViewController>& controller) {
    _presentedViewControllers.push_back(controller);
}

void UIWindow::removePresentedViewController(const std::shared_ptr<UIViewController>& controller) {
    _presentedViewControllers.erase(std::remove(_presentedViewControllers.begin(), _presentedViewControllers.end(), controller), _presentedViewControllers.end());
}

void UIWindow::pressesBegan(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesBegan(pressees, event);
}

void UIWindow::pressesChanged(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesChanged(pressees, event);
//    focusSystem()->sendEvent(event);
}

void UIWindow::pressesEnded(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesEnded(pressees, event);
}

void UIWindow::pressesCancelled(std::set<std::shared_ptr<UIPress>> pressees, std::shared_ptr<UIPressesEvent> event) {
    UIView::pressesCancelled(pressees, event);
//    focusSystem()->sendEvent(event);
}

void UIWindow::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UITraitEnvironment::traitCollectionDidChange(previousTraitCollection);
    if (!rootViewController()) return;

    rootViewController()->_traitCollection = _traitCollection;
    rootViewController()->traitCollectionDidChange(previousTraitCollection);
}
