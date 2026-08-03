#include <UIViewController.h>
#include <UIPresentationController.h>
#include <UIWindow.h>
#include <DispatchQueue.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <vector>

using namespace NXKit;

namespace {

class DefaultPresentationController final : public UIPresentationController {
public:
    DefaultPresentationController(
        const std::shared_ptr<UIViewController>& presentedViewController,
        const std::shared_ptr<UIViewController>& presentingViewController,
        bool removesPresentersView
    ) : UIPresentationController(presentedViewController, presentingViewController),
        _removesPresentersView(removesPresentersView) {}

    [[nodiscard]] bool shouldRemovePresentersView() const override {
        return _removesPresentersView;
    }

private:
    bool _removesPresentersView;
};

class PresentationTransitionContext final
    : public UIViewControllerContextTransitioning,
      public std::enable_shared_from_this<PresentationTransitionContext> {
public:
    using Completion = std::function<void(bool)>;

    PresentationTransitionContext(
        std::shared_ptr<UIView> containerView,
        std::shared_ptr<UIViewController> fromViewController,
        std::shared_ptr<UIViewController> toViewController,
        NXRect fromInitialFrame,
        NXRect fromFinalFrame,
        NXRect toInitialFrame,
        NXRect toFinalFrame,
        bool animated,
        UIModalPresentationStyle presentationStyle,
        Completion completion
    ) : _containerView(std::move(containerView)),
        _fromViewController(std::move(fromViewController)),
        _toViewController(std::move(toViewController)),
        _fromInitialFrame(fromInitialFrame),
        _fromFinalFrame(fromFinalFrame),
        _toInitialFrame(toInitialFrame),
        _toFinalFrame(toFinalFrame),
        _animated(animated),
        _presentationStyle(presentationStyle),
        _completion(std::move(completion)) {}

    [[nodiscard]] std::shared_ptr<UIView> containerView() const override {
        return _containerView;
    }

    [[nodiscard]] std::shared_ptr<UIViewController> viewControllerForKey(
        UITransitionContextViewControllerKey key
    ) const override {
        return key == UITransitionContextViewControllerKey::from
            ? _fromViewController
            : _toViewController;
    }

    [[nodiscard]] std::shared_ptr<UIView> viewForKey(
        UITransitionContextViewKey key
    ) const override {
        const auto controller = key == UITransitionContextViewKey::from
            ? _fromViewController
            : _toViewController;
        return controller ? controller->view() : nullptr;
    }

    [[nodiscard]] NXRect initialFrameForViewController(
        const std::shared_ptr<UIViewController>& viewController
    ) const override {
        if (viewController == _fromViewController) {
            return _fromInitialFrame;
        }
        if (viewController == _toViewController) {
            return _toInitialFrame;
        }
        return NXRect::null;
    }

    [[nodiscard]] NXRect finalFrameForViewController(
        const std::shared_ptr<UIViewController>& viewController
    ) const override {
        if (viewController == _fromViewController) {
            return _fromFinalFrame;
        }
        if (viewController == _toViewController) {
            return _toFinalFrame;
        }
        return NXRect::null;
    }

    [[nodiscard]] bool isAnimated() const override { return _animated; }

    [[nodiscard]] UIModalPresentationStyle presentationStyle() const override {
        return _presentationStyle;
    }

    [[nodiscard]] bool transitionWasCancelled() const override {
        return _completed && !_didComplete;
    }

    void completeTransition(bool didComplete) override {
        if (_completed) {
            return;
        }

        const auto keepAlive = shared_from_this();
        _completed = true;
        _didComplete = didComplete;
        auto completion = std::move(_completion);
        _completion = nullptr;
        if (completion) {
            completion(didComplete);
        }
    }

private:
    std::shared_ptr<UIView> _containerView;
    std::shared_ptr<UIViewController> _fromViewController;
    std::shared_ptr<UIViewController> _toViewController;
    NXRect _fromInitialFrame;
    NXRect _fromFinalFrame;
    NXRect _toInitialFrame;
    NXRect _toFinalFrame;
    bool _animated;
    UIModalPresentationStyle _presentationStyle;
    Completion _completion;
    bool _completed = false;
    bool _didComplete = false;
};

class ModalTransitionAnimator final : public UIViewControllerAnimatedTransitioning {
public:
    ModalTransitionAnimator(UIModalTransitionStyle style, bool presenting)
        : _style(style), _presenting(presenting) {}

    [[nodiscard]] double transitionDuration(
        const std::shared_ptr<UIViewControllerContextTransitioning>&
    ) const override {
        return 0.5;
    }

    void animateTransition(
        const std::shared_ptr<UIViewControllerContextTransitioning>& context
    ) override {
        const auto container = context->containerView();
        const bool animatesToVisible = _presenting;
        const auto transitionKey = _presenting
            ? UITransitionContextViewKey::to
            : UITransitionContextViewKey::from;
        const auto transitionView = context->viewForKey(transitionKey);
        if (!transitionView || !container) {
            context->completeTransition(false);
            return;
        }

        const auto height = container->bounds().height();
        const auto applyVisibleState = [transitionView]() {
            transitionView->setAlpha(1);
            transitionView->setTransform(NXAffineTransform::identity);
        };
        const auto applyHiddenState = [transitionView, height, style = _style]() {
            switch (style) {
                case UIModalTransitionStyle::coverVertical:
                    transitionView->setTransform(
                        NXAffineTransform::translationBy(0, height)
                    );
                    break;
                case UIModalTransitionStyle::flipHorizontal:
                    transitionView->setAlpha(0);
                    transitionView->setTransform(NXAffineTransform::scaleBy(0.01f, 1));
                    break;
                case UIModalTransitionStyle::crossDissolve:
                    transitionView->setAlpha(0);
                    break;
                case UIModalTransitionStyle::partialCurl:
                    // NXKit's portable renderer has no page-curl primitive. This
                    // folds the presented view without reordering or animating
                    // the live presenting hierarchy.
                    transitionView->setAlpha(0);
                    transitionView->setTransform(
                        NXAffineTransform::translationBy(0, height * 0.18f)
                            * NXAffineTransform::rotationBy(4)
                            * NXAffineTransform::scaleBy(0.94f, 0.88f)
                    );
                    break;
            }
        };

        if (animatesToVisible) {
            applyHiddenState();
        } else {
            applyVisibleState();
        }

        const std::function<void()> animations = animatesToVisible
            ? std::function<void()>(applyVisibleState)
            : std::function<void()>(applyHiddenState);
        if (!context->isAnimated()) {
            animations();
            context->completeTransition(true);
            if (!animatesToVisible) {
                applyVisibleState();
            }
            return;
        }

        UIView::animate(
            transitionDuration(context),
            0,
            UIViewAnimationOptions(
                curveEaseOut
                    | preferredFramesPerSecond120
                    | allowUserInteraction
            ),
            animations,
            [context, transitionView, animatesToVisible](bool finished) {
                context->completeTransition(finished);
                if (!animatesToVisible) {
                    transitionView->setAlpha(1);
                    transitionView->setTransform(NXAffineTransform::identity);
                }
            }
        );
    }

private:
    UIModalTransitionStyle _style;
    bool _presenting;
};

} // namespace

std::shared_ptr<UIResponder> UIViewController::next() {
    if (!_view->_superview.expired()) {
        return _view->_superview.lock();
    }
    return nullptr;
}

std::shared_ptr<UIView> UIViewController::view() {
    loadViewIfNeeded();
    return _view;
}

void UIViewController::setView(std::shared_ptr<UIView> view) {
    if (_view) _view->_parentController.reset();
    _view = std::move(view);
    _view->_parentController = weak_from_this();

    if (traitCollection() != nullptr) {
        _view->_traitCollection = traitCollection();
        _view->UITraitEnvironment::traitCollectionDidChange(nullptr);
    }

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
    if (!child) {
        throw std::invalid_argument("UIViewController::addChild requires a non-null child");
    }
    if (child.get() == this) {
        throw std::invalid_argument("A view controller cannot contain itself");
    }
    if (!child->_parent.expired()) {
        throw std::invalid_argument("The child view controller already has a parent");
    }

    for (auto ancestor = shared_from_this(); ancestor; ancestor = ancestor->parent().lock()) {
        if (ancestor == child) {
            throw std::invalid_argument("Adding this child would create a controller hierarchy cycle");
        }
    }

    auto parent = shared_from_this();
    child->willMoveToParent(parent);
    child->_parent = parent;
    _children.push_back(child);

    child->_traitCollection = _traitCollection;
    child->traitCollectionDidChange(nullptr);
}

void UIViewController::willMoveToParent(const std::shared_ptr<UIViewController>& parent) {
}

void UIViewController::didMoveToParent(std::shared_ptr<UIViewController> parent) {
}

void UIViewController::removeFromParent() {
    if (auto spt = _parent.lock()) {
        spt->_children.erase(std::remove(spt->_children.begin(), spt->_children.end(), shared_from_this()), spt->_children.end());
        this->_parent.reset();
        didMoveToParent(nullptr);
    }
}

void UIViewController::setAdditionalSafeAreaInsets(UIEdgeInsets additionalSafeAreaInsets) {
    if (_additionalSafeAreaInsets == additionalSafeAreaInsets) return;
    _additionalSafeAreaInsets = additionalSafeAreaInsets;
    view()->setNeedsUpdateSafeAreaInsets();
}

void UIViewController::setViewRespectsSystemMinimumLayoutMargins(bool viewRespectsSystemMinimumLayoutMargins) {
    if (_viewRespectsSystemMinimumLayoutMargins == viewRespectsSystemMinimumLayoutMargins) return;
    _viewRespectsSystemMinimumLayoutMargins = viewRespectsSystemMinimumLayoutMargins;
//    view()->setNeedsUpdateLayoutMargins();
}

void UIViewController::present(const std::shared_ptr<UIViewController>& otherViewController, bool animated, const std::function<void()>& completion) {
    if (!otherViewController) {
        throw std::invalid_argument("UIViewController::present requires a non-null view controller");
    }
    if (!otherViewController->parent().expired()) {
        throw std::invalid_argument(
            "A presented view controller cannot already belong to a parent container"
        );
    }

    const auto sourceViewController = shared_from_this();
    const auto presentingViewController = rootVC();
    if (presentingViewController == otherViewController) {
        throw std::invalid_argument("A view controller cannot present itself");
    }

    const auto sourceCoordinator = modalTransitionCoordinator();
    const auto targetCoordinator = otherViewController->modalTransitionCoordinator();
    const auto activeCoordinator = sourceCoordinator->_isModalTransitionInFlight
        ? sourceCoordinator
        : (targetCoordinator->_isModalTransitionInFlight
            ? targetCoordinator
            : nullptr);
    if (activeCoordinator) {
        activeCoordinator->enqueueModalOperation({
            PendingModalOperationKind::present,
            sourceViewController,
            otherViewController,
            animated,
            completion,
        });
        return;
    }

    if (presentingViewController->_presentedViewController) {
        // UIKit ignores a presentation request when this controller already
        // owns a presented controller. Treat repeated input the same way
        // instead of terminating an application with an uncaught exception.
        return;
    }
    if (!presentingViewController->_presentingViewController.expired()
        && presentingViewController->modalTransitionStyle()
            == UIModalTransitionStyle::partialCurl) {
        throw std::logic_error(
            "A controller presented with partial curl cannot present another controller"
        );
    }
    if (!otherViewController->_presentingViewController.expired()) {
        throw std::logic_error("The view controller is already being presented");
    }
    if (presentingViewController->_isPerformingModalTransition
        || otherViewController->_isPerformingModalTransition) {
        throw std::logic_error("A modal transition is already in progress");
    }
    if (otherViewController->modalTransitionStyle()
            == UIModalTransitionStyle::partialCurl
        && otherViewController->modalPresentationStyle()
            != UIModalPresentationStyle::fullScreen) {
        throw std::invalid_argument(
            "The partial-curl transition requires full-screen presentation"
        );
    }

    const auto window = presentingViewController->view()->window();
    if (!window) {
        throw std::logic_error(
            "UIViewController::present requires the presenting view to be in a window"
        );
    }
    const auto transitioningDelegate = otherViewController->transitioningDelegate();
    std::shared_ptr<UIPresentationController> presentationController;
    std::shared_ptr<UIViewControllerAnimatedTransitioning> animator;
    if (otherViewController->modalPresentationStyle()
        == UIModalPresentationStyle::custom && transitioningDelegate) {
        presentationController =
            transitioningDelegate->presentationControllerForPresented(
                otherViewController,
                presentingViewController,
                sourceViewController
            );
        animator = transitioningDelegate->animationControllerForPresented(
            otherViewController,
            presentingViewController,
            sourceViewController
        );
    }

    if (!presentationController) {
        presentationController = new_shared<DefaultPresentationController>(
            otherViewController,
            presentingViewController,
            otherViewController->modalPresentationStyle()
                == UIModalPresentationStyle::fullScreen
        );
    }
    if (!animator) {
        animator = new_shared<ModalTransitionAnimator>(
            otherViewController->modalTransitionStyle(),
            true
        );
    }

    sourceCoordinator->_isModalTransitionInFlight = true;
    presentingViewController->_presentedViewController = otherViewController;
    otherViewController->_presentingViewController = presentingViewController;
    otherViewController->_presentationController = presentationController;
    presentingViewController->_isPerformingModalTransition = true;
    otherViewController->_isPerformingModalTransition = true;
    otherViewController->_isBeingPresented = true;

    const auto previousTraitCollection = otherViewController->_traitCollection;
    otherViewController->_traitCollection = presentingViewController->_traitCollection;
    otherViewController->traitCollectionDidChange(previousTraitCollection);

    presentationController->setContainerView(window);
    const bool removesPresentersView =
        presentationController->shouldRemovePresentersView();
    const auto finalPresentedFrame =
        presentationController->frameOfPresentedViewInContainerView();

    otherViewController->loadViewIfNeeded();
    otherViewController->view()->setFrame(finalPresentedFrame);

    if (removesPresentersView) {
        presentingViewController->viewWillDisappear(animated);
    }
    otherViewController->viewWillAppear(animated);
    presentationController->presentationTransitionWillBegin();
    window->addSubview(otherViewController->view());
    window->addPresentedViewController(otherViewController);
    presentationController->containerViewWillLayoutSubviews();
    otherViewController->view()->setFrame(
        presentationController->frameOfPresentedViewInContainerView()
    );
    otherViewController->view()->layoutSubviews();
    presentationController->containerViewDidLayoutSubviews();

    const auto weakPresented = std::weak_ptr<UIViewController>(otherViewController);
    const auto weakPresenting = std::weak_ptr<UIViewController>(presentingViewController);
    const auto weakWindow = std::weak_ptr<UIWindow>(window);
    const auto weakCoordinator = std::weak_ptr<UIViewController>(sourceCoordinator);
    const auto context = std::make_shared<PresentationTransitionContext>(
        window,
        presentingViewController,
        otherViewController,
        presentingViewController->view()->frame(),
        presentingViewController->view()->frame(),
        finalPresentedFrame,
        finalPresentedFrame,
        animated,
        otherViewController->modalPresentationStyle(),
        [
            weakPresented,
            weakPresenting,
            weakWindow,
            weakCoordinator,
            presentationController,
            animator,
            removesPresentersView,
            animated,
            completion
        ](bool completed) {
            const auto presented = weakPresented.lock();
            const auto presenting = weakPresenting.lock();
            const auto containingWindow = weakWindow.lock();
            if (!presented || !presenting) {
                animator->animationEnded(false);
                if (const auto coordinator = weakCoordinator.lock()) {
                    coordinator->completeModalTransition();
                }
                return;
            }

            if (completed) {
                if (removesPresentersView) {
                    presenting->view()->removeFromSuperview();
                    presenting->viewDidDisappear(animated);
                }
                presented->viewDidAppear(animated);
            } else {
                presented->viewWillDisappear(animated);
                if (removesPresentersView) {
                    presenting->viewWillAppear(animated);
                }
                presented->view()->removeFromSuperview();
                if (containingWindow) {
                    containingWindow->removePresentedViewController(presented);
                }
                if (removesPresentersView) {
                    presenting->viewDidAppear(animated);
                }
                presented->viewDidDisappear(animated);
            }

            presented->_isBeingPresented = false;
            presented->_isPerformingModalTransition = false;
            presenting->_isPerformingModalTransition = false;
            presented->_activeTransitionContext.reset();
            presentationController->presentationTransitionDidEnd(completed);
            if (!completed) {
                presenting->_presentedViewController.reset();
                presented->_presentingViewController.reset();
                presented->_presentationController.reset();
                presentationController->setContainerView(nullptr);
            }
            animator->animationEnded(completed);

            if (containingWindow) {
                containingWindow->updateFocus();
            }
            if (completed) {
                completion();
            }
            if (const auto coordinator = weakCoordinator.lock()) {
                coordinator->completeModalTransition();
            }
        }
    );
    otherViewController->_activeTransitionContext = context;

    try {
        const auto duration = animator->transitionDuration(context);
        if (!std::isfinite(duration) || duration < 0) {
            throw std::invalid_argument(
                "A transition animator must return a finite, non-negative duration"
            );
        }
        animator->animateTransition(context);
    } catch (...) {
        context->completeTransition(false);
        throw;
    }
}

void UIViewController::dismiss(bool animated, const std::function<void()>& completion) {
    const auto requester = rootVC();
    const auto coordinator = modalTransitionCoordinator();
    if (coordinator->_isModalTransitionInFlight) {
        coordinator->enqueueModalOperation({
            PendingModalOperationKind::dismiss,
            shared_from_this(),
            nullptr,
            animated,
            completion,
        });
        return;
    }

    const auto firstDismissed = requester->_presentedViewController
        ? requester->_presentedViewController
        : (!requester->_presentingViewController.expired() ? requester : nullptr);
    if (!firstDismissed) {
        return;
    }

    const auto finalPresenting = firstDismissed->_presentingViewController.lock();
    if (!finalPresenting) {
        return;
    }

    std::vector<std::shared_ptr<UIViewController>> dismissedControllers;
    for (auto controller = firstDismissed; controller;
         controller = controller->_presentedViewController) {
        dismissedControllers.push_back(controller);
    }
    const auto topDismissed = dismissedControllers.back();

    if (finalPresenting->_isPerformingModalTransition) {
        throw std::logic_error("A modal transition is already in progress");
    }
    for (const auto& controller : dismissedControllers) {
        if (controller->_isPerformingModalTransition) {
            throw std::logic_error("A modal transition is already in progress");
        }
    }

    const auto window = topDismissed->view()->window();
    if (!window) {
        throw std::logic_error(
            "UIViewController::dismiss requires the presented view to be in a window"
        );
    }

    auto presentationController = topDismissed->_presentationController;
    if (!presentationController) {
        throw std::logic_error("The presented controller has no presentation controller");
    }

    std::shared_ptr<UIViewControllerAnimatedTransitioning> animator;
    const auto transitioningDelegate = topDismissed->transitioningDelegate();
    if (topDismissed->modalPresentationStyle()
        == UIModalPresentationStyle::custom && transitioningDelegate) {
        animator = transitioningDelegate->animationControllerForDismissed(topDismissed);
    }
    if (!animator) {
        animator = new_shared<ModalTransitionAnimator>(
            topDismissed->modalTransitionStyle(),
            false
        );
    }

    coordinator->_isModalTransitionInFlight = true;
    finalPresenting->_isPerformingModalTransition = true;
    for (const auto& controller : dismissedControllers) {
        controller->_isPerformingModalTransition = true;
        controller->_isBeingDismissed = true;
        if (controller->_presentationController) {
            controller->_presentationController->dismissalTransitionWillBegin();
        }
    }

    const bool finalPresenterWasDetached =
        finalPresenting->view()->superview().expired();
    if (finalPresenterWasDetached) {
        finalPresenting->view()->setFrame(window->bounds());
        window->insertSubviewBelow(finalPresenting->view(), topDismissed->view());
        finalPresenting->viewWillAppear(animated);
    }

    std::vector<std::shared_ptr<UIViewController>> visibleIntermediateControllers;
    for (const auto& controller : dismissedControllers) {
        if (controller == topDismissed) {
            continue;
        }
        if (!controller->view()->superview().expired()) {
            controller->viewWillDisappear(false);
            visibleIntermediateControllers.push_back(controller);
        }
    }
    topDismissed->viewWillDisappear(animated);

    const auto weakFinalPresenting = std::weak_ptr<UIViewController>(finalPresenting);
    const auto weakWindow = std::weak_ptr<UIWindow>(window);
    const auto weakCoordinator = std::weak_ptr<UIViewController>(coordinator);
    const auto context = std::make_shared<PresentationTransitionContext>(
        window,
        topDismissed,
        finalPresenting,
        topDismissed->view()->frame(),
        topDismissed->view()->frame(),
        finalPresenting->view()->frame(),
        finalPresenting->view()->frame(),
        animated,
        topDismissed->modalPresentationStyle(),
        [
            dismissedControllers,
            visibleIntermediateControllers,
            weakFinalPresenting,
            weakWindow,
            weakCoordinator,
            presentationController,
            animator,
            finalPresenterWasDetached,
            animated,
            completion
        ](bool completed) {
            const auto finalPresenter = weakFinalPresenting.lock();
            const auto containingWindow = weakWindow.lock();
            if (!finalPresenter) {
                animator->animationEnded(false);
                if (const auto strongCoordinator = weakCoordinator.lock()) {
                    strongCoordinator->completeModalTransition();
                }
                return;
            }
            const auto top = dismissedControllers.back();

            if (completed) {
                top->viewDidDisappear(animated);
                for (const auto& intermediate : visibleIntermediateControllers) {
                    intermediate->viewDidDisappear(false);
                }
                if (finalPresenterWasDetached) {
                    finalPresenter->viewDidAppear(animated);
                }

                for (auto controller = dismissedControllers.rbegin();
                     controller != dismissedControllers.rend(); ++controller) {
                    (*controller)->view()->removeFromSuperview();
                    if (containingWindow) {
                        containingWindow->removePresentedViewController(*controller);
                    }
                    if ((*controller)->_presentationController) {
                        (*controller)->_presentationController
                            ->dismissalTransitionDidEnd(true);
                        (*controller)->_presentationController
                            ->setContainerView(nullptr);
                    }
                }

                finalPresenter->_presentedViewController.reset();
                for (const auto& controller : dismissedControllers) {
                    controller->_presentedViewController.reset();
                    controller->_presentingViewController.reset();
                    controller->_presentationController.reset();
                }
            } else {
                top->viewWillAppear(animated);
                for (const auto& intermediate : visibleIntermediateControllers) {
                    intermediate->viewWillAppear(false);
                }
                if (finalPresenterWasDetached) {
                    finalPresenter->viewWillDisappear(animated);
                }
                top->viewDidAppear(animated);
                for (const auto& intermediate : visibleIntermediateControllers) {
                    intermediate->viewDidAppear(false);
                }
                if (finalPresenterWasDetached) {
                    finalPresenter->view()->removeFromSuperview();
                    finalPresenter->viewDidDisappear(animated);
                }
                for (const auto& controller : dismissedControllers) {
                    if (controller->_presentationController) {
                        controller->_presentationController
                            ->dismissalTransitionDidEnd(false);
                    }
                }
            }

            for (const auto& controller : dismissedControllers) {
                controller->_isBeingDismissed = false;
                controller->_isPerformingModalTransition = false;
                controller->_activeTransitionContext.reset();
            }
            finalPresenter->_isPerformingModalTransition = false;
            animator->animationEnded(completed);

            if (containingWindow) {
                containingWindow->updateFocus();
            }
            if (completed) {
                completion();
            }
            if (const auto strongCoordinator = weakCoordinator.lock()) {
                strongCoordinator->completeModalTransition();
            }
        }
    );
    topDismissed->_activeTransitionContext = context;

    try {
        const auto duration = animator->transitionDuration(context);
        if (!std::isfinite(duration) || duration < 0) {
            throw std::invalid_argument(
                "A transition animator must return a finite, non-negative duration"
            );
        }
        animator->animateTransition(context);
    } catch (...) {
        context->completeTransition(false);
        throw;
    }
}

std::shared_ptr<UIViewController> UIViewController::presentedViewController() {
    for (auto controller = shared_from_this(); controller;
         controller = controller->parent().lock()) {
        if (controller->_presentedViewController) {
            return controller->_presentedViewController;
        }
    }
    return nullptr;
}

std::shared_ptr<UIViewController> UIViewController::presentingViewController() {
    return rootVC()->_presentingViewController.lock();
}

std::shared_ptr<UIPresentationController> UIViewController::presentationController() {
    return rootVC()->_presentationController;
}

bool UIViewController::isBeingPresented() {
    return rootVC()->_isBeingPresented;
}

bool UIViewController::isBeingDismissed() {
    return rootVC()->_isBeingDismissed;
}

void UIViewController::show(
    const std::shared_ptr<UIViewController>& viewController,
    const std::shared_ptr<UIResponder>& sender
) {
    if (!viewController) {
        throw std::invalid_argument("UIViewController::show requires a non-null view controller");
    }

    if (const auto parentViewController = parent().lock()) {
        parentViewController->show(viewController, sender ? sender : shared_from_this());
        return;
    }

    present(viewController, true);
}

void UIViewController::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UITraitEnvironment::traitCollectionDidChange(previousTraitCollection);
    if (_view) {
        _view->_traitCollection = _traitCollection;
        _view->traitCollectionDidChange(previousTraitCollection);
    }
    for (auto child : _children) {
        child->_traitCollection = _traitCollection;
        child->traitCollectionDidChange(previousTraitCollection);
    }
}

std::shared_ptr<UIFocusEnvironment> UIViewController::parentFocusEnvironment() {
    if (_presentationController) {
        return _presentationController;
    }
    return std::dynamic_pointer_cast<UIFocusEnvironment>(next());
}

std::shared_ptr<UIViewController> UIViewController::rootVC() {
    auto root = shared_from_this();
    while (!root->parent().expired()) {
        root = root->parent().lock();
    }
    return root;
}

std::shared_ptr<UIViewController> UIViewController::modalTransitionCoordinator() {
    auto coordinator = rootVC();
    while (const auto presenter = coordinator->_presentingViewController.lock()) {
        coordinator = presenter->rootVC();
    }
    return coordinator;
}

void UIViewController::enqueueModalOperation(PendingModalOperation operation) {
    _pendingModalOperations.push_back(std::move(operation));
}

void UIViewController::completeModalTransition() {
    _isModalTransitionInFlight = false;
    drainPendingModalOperations();
}

void UIViewController::drainPendingModalOperations() {
    if (_isModalTransitionInFlight || _isDrainingModalOperations) {
        return;
    }

    _isDrainingModalOperations = true;
    try {
        while (!_isModalTransitionInFlight && !_pendingModalOperations.empty()) {
            auto operation = std::move(_pendingModalOperations.front());
            _pendingModalOperations.pop_front();
            const auto source = operation.source.lock();
            if (!source) {
                continue;
            }

            try {
                if (operation.kind == PendingModalOperationKind::present) {
                    source->present(
                        operation.presentedViewController,
                        operation.animated,
                        operation.completion
                    );
                } else {
                    source->dismiss(operation.animated, operation.completion);
                }
            } catch (const std::logic_error&) {
                // Presentation state can legitimately change while an operation
                // is waiting. Ignore a now-inapplicable queued request rather
                // than corrupting the active presentation hierarchy.
            }
        }
    } catch (...) {
        _isDrainingModalOperations = false;
        throw;
    }
    _isDrainingModalOperations = false;
}
