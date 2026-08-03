#pragma once

#include <Geometry.h>

#include <memory>

namespace NXKit {

class UIPresentationController;
class UIView;
class UIViewController;

enum class UIModalPresentationStyle {
    fullScreen,
    custom,
    overFullScreen,
};

enum class UIModalTransitionStyle {
    coverVertical,
    flipHorizontal,
    crossDissolve,
    partialCurl,
};

enum class UITransitionContextViewControllerKey {
    from,
    to,
};

enum class UITransitionContextViewKey {
    from,
    to,
};

class UIViewControllerContextTransitioning {
public:
    virtual ~UIViewControllerContextTransitioning() = default;

    [[nodiscard]] virtual std::shared_ptr<UIView> containerView() const = 0;
    [[nodiscard]] virtual std::shared_ptr<UIViewController> viewControllerForKey(
        UITransitionContextViewControllerKey key
    ) const = 0;
    [[nodiscard]] virtual std::shared_ptr<UIView> viewForKey(
        UITransitionContextViewKey key
    ) const = 0;

    [[nodiscard]] virtual NXRect initialFrameForViewController(
        const std::shared_ptr<UIViewController>& viewController
    ) const = 0;
    [[nodiscard]] virtual NXRect finalFrameForViewController(
        const std::shared_ptr<UIViewController>& viewController
    ) const = 0;

    [[nodiscard]] virtual bool isAnimated() const = 0;
    [[nodiscard]] virtual UIModalPresentationStyle presentationStyle() const = 0;
    [[nodiscard]] virtual bool transitionWasCancelled() const = 0;

    // Custom animators must call this exactly once when their animation ends.
    virtual void completeTransition(bool didComplete) = 0;
};

class UIViewControllerAnimatedTransitioning {
public:
    virtual ~UIViewControllerAnimatedTransitioning() = default;

    [[nodiscard]] virtual double transitionDuration(
        const std::shared_ptr<UIViewControllerContextTransitioning>& transitionContext
    ) const = 0;
    virtual void animateTransition(
        const std::shared_ptr<UIViewControllerContextTransitioning>& transitionContext
    ) = 0;
    virtual void animationEnded(bool transitionCompleted) {}
};

class UIViewControllerTransitioningDelegate {
public:
    virtual ~UIViewControllerTransitioningDelegate() = default;

    virtual std::shared_ptr<UIViewControllerAnimatedTransitioning>
    animationControllerForPresented(
        const std::shared_ptr<UIViewController>& presented,
        const std::shared_ptr<UIViewController>& presenting,
        const std::shared_ptr<UIViewController>& source
    ) {
        return nullptr;
    }

    virtual std::shared_ptr<UIViewControllerAnimatedTransitioning>
    animationControllerForDismissed(
        const std::shared_ptr<UIViewController>& dismissed
    ) {
        return nullptr;
    }

    virtual std::shared_ptr<UIPresentationController>
    presentationControllerForPresented(
        const std::shared_ptr<UIViewController>& presented,
        const std::shared_ptr<UIViewController>& presenting,
        const std::shared_ptr<UIViewController>& source
    ) {
        return nullptr;
    }
};

} // namespace NXKit
