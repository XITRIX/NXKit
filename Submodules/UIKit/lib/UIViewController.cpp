#include <UIViewController.h>
#include <UIWindow.h>
#include <DispatchQueue.h>

using namespace NXKit;

//std::shared_ptr<UIResponder> UIViewController::next() {
//    if (!_view->_superview.expired()) {
//        return _view->_superview.lock();
//    }
//    return nullptr;
//}

std::shared_ptr<UIView> UIViewController::view() {
    loadViewIfNeeded();
    return _view;
}

void UIViewController::setView(std::shared_ptr<UIView> view) {
    if (_view) _view->_parentController.reset();
    _view = std::move(view);
    _view->_parentController = weak_from_this();
    viewDidLoad();
}

bool UIViewController::viewIsLoaded() {
    return _view != nullptr;
}

void UIViewController::loadViewIfNeeded() {
    if (!viewIsLoaded()) {
        loadView();
    }
}

void UIViewController::loadView() {
    setView(new_shared<UIView>());
}

void UIViewController::viewWillAppear(bool animated) {
    for (auto& child: _children) {
        DispatchQueue::main()->async([child, animated]() { child->viewWillAppear(animated); });
    }
}

void UIViewController::viewDidAppear(bool animated) {
    for (auto& child: _children) {
        DispatchQueue::main()->async([child, animated]() { child->viewDidAppear(animated); });
    }
}

void UIViewController::viewWillDisappear(bool animated) {
    for (auto& child: _children) {
        DispatchQueue::main()->async([child, animated]() { child->viewWillDisappear(animated); });
    }
}

void UIViewController::viewDidDisappear(bool animated) {
    for (auto& child: _children) {
        DispatchQueue::main()->async([child, animated]() { child->viewDidDisappear(animated); });
    }
}

void UIViewController::addChild(const std::shared_ptr<UIViewController>& child) {
    _children.push_back(child);
    child->willMoveToParent(weak_from_this().lock());
}

void UIViewController::willMoveToParent(const std::shared_ptr<UIViewController>& parent) {
    if (parent)
        this->_parent = parent;
}

void UIViewController::didMoveToParent(std::shared_ptr<UIViewController> parent) {
    if (parent->view()->window())
        viewDidAppear(true);
}

void UIViewController::removeFromParent() {
    if (auto spt = _parent.lock()) {
        spt->_children.erase(std::remove(spt->_children.begin(), spt->_children.end(), shared_from_this()));
        this->_parent.reset();
        viewDidDisappear(true);
    }
}

//void UIViewController::setAdditionalSafeAreaInsets(UIEdgeInsets additionalSafeAreaInsets) {
//    if (_additionalSafeAreaInsets == additionalSafeAreaInsets) return;
//    _additionalSafeAreaInsets = additionalSafeAreaInsets;
//    view()->setNeedsUpdateSafeAreaInsets();
//}

void UIViewController::setViewRespectsSystemMinimumLayoutMargins(bool viewRespectsSystemMinimumLayoutMargins) {
    if (_viewRespectsSystemMinimumLayoutMargins == viewRespectsSystemMinimumLayoutMargins) return;
    _viewRespectsSystemMinimumLayoutMargins = viewRespectsSystemMinimumLayoutMargins;
//    view()->setNeedsUpdateLayoutMargins();
}

void UIViewController::present(std::shared_ptr<UIViewController> otherViewController, bool animated, std::function<void()> completion) {
    if (!parent().expired()) {
        return parent().lock()->present(otherViewController, animated, completion);
    }

    if (_presentedViewController != nullptr) {
        printf("Warning: attempted to present \(otherViewController), but \(self) is already presenting another view controller. Ignoring request.");
        return;
    }

    if (!otherViewController->_presentingViewController.expired()) {
        printf("Tried to present \(otherViewController) but it is already being presented by \(otherViewController.presentingViewController!)");
        return;
    }

    _presentedViewController = otherViewController;
    otherViewController->_presentingViewController = shared_from_this();

    otherViewController->view()->setFrame(view()->window()->bounds());
    otherViewController->viewWillAppear(animated);
    otherViewController->makeViewAppear(animated, shared_from_this(), [otherViewController]() {
//        otherViewController->view()->window()->updateFocus();
    });
    otherViewController->viewDidAppear(animated);

    otherViewController->view()->layoutSubviews();

    completion();
}

void UIViewController::dismiss(bool animated, std::function<void()> completion) {
    if (!parent().expired()) {
        return parent().lock()->dismiss(animated, completion);
    }

    if (_presentingViewController.expired()) return;

//    if let navigationController = navigationController {
//        navigationController.dismiss(animated: animated, completion: completion)
//        return
//    }

    viewWillDisappear(animated);

    // This comes before completion because we may want to check if any presentedViewControllers are present before the animation is completed. In that case we're not really "presenting" anything anymore so we should return `nil`. This is particularly important for a `UIAlertController` that presents another UIViewController after dismissing itself.
    _presentingViewController.lock()->_presentedViewController = nullptr;

    makeViewDisappear(animated, [this, animated, completion](auto) {
//        _presentingViewController.lock()->_presentedViewController = nullptr;
        auto window = view()->window();
        view()->removeFromSuperview();
        viewDidDisappear(animated);
        completion();
//        window->updateFocus();
        _presentingViewController.reset();
        window->removePresentedViewController(shared_from_this());
    });
}

//std::shared_ptr<UIFocusEnvironment> UIViewController::parentFocusEnvironment() {
//    return std::dynamic_pointer_cast<UIFocusEnvironment>(next());
//}

void UIViewController::makeViewAppear(bool animated, std::shared_ptr<UIViewController> presentingViewController, std::function<void()> completion) {
    auto window = presentingViewController->view()->window();
    window->addSubview(view());
    window->addPresentedViewController(shared_from_this());

//    view()->yoga->setIncludedInLayout(false);
    view()->setTransform(NXAffineTransform::translationBy(0, window->bounds().maxY()));

    UIView::animate(animated ? _animationTime : 0, 0, curveEaseOut, [this]() {
//        _presentingViewController.lock()->view()->setTransform(NXAffineTransform::scaleBy(0.9f, 1));
        view()->setTransform(NXAffineTransform::identity);
    }, [this, completion](auto) {
        _presentingViewController.lock()->view()->removeFromSuperview();
        completion();
    });
}

void UIViewController::makeViewDisappear(bool animated, std::function<void(bool)> completion) {
    view()->window()->insertSubviewBelow(_presentingViewController.lock()->view(), view());
    UIView::animate(
        animated ? _animationTime : 0.0,
        0,
        curveEaseOut,
        [this]() {
//            _presentingViewController.lock()->view()->setTransform(NXAffineTransform::identity);

            float yOffset = 0;
            if (!view()->superview().expired()) {
                yOffset = view()->superview().lock()->bounds().height();
            } else {
                yOffset = view()->frame().height();
            }
            view()->setTransform(NXAffineTransform::translationBy(0, yOffset));
        }, [this, completion](bool res) {
            completion(res);
        });
}

std::shared_ptr<UIViewController> UIViewController::rootVC() {
    auto root = shared_from_this();
    while (!root->parent().expired()) {
        root = root->parent().lock();
    }
    return root;
}
