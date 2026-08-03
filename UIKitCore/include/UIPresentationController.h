#pragma once

#include <UIFocus.h>
#include <UIViewControllerTransitioning.h>

#include <memory>

namespace NXKit {

class UIView;
class UIViewController;

class UIPresentationController : public UIFocusEnvironment {
public:
    UIPresentationController(
        const std::shared_ptr<UIViewController>& presentedViewController,
        const std::shared_ptr<UIViewController>& presentingViewController
    );
    virtual ~UIPresentationController() = default;

    [[nodiscard]] std::shared_ptr<UIViewController> presentedViewController() const;
    [[nodiscard]] std::shared_ptr<UIViewController> presentingViewController() const;
    [[nodiscard]] std::shared_ptr<UIView> containerView() const;
    [[nodiscard]] virtual std::shared_ptr<UIView> presentedView() const;

    [[nodiscard]] virtual UIModalPresentationStyle presentationStyle() const;
    [[nodiscard]] virtual NXRect frameOfPresentedViewInContainerView() const;
    [[nodiscard]] virtual bool shouldRemovePresentersView() const;

    std::vector<std::shared_ptr<UIFocusEnvironment>>
    preferredFocusEnvironments() override;
    std::shared_ptr<UIFocusEnvironment> parentFocusEnvironment() override;

    virtual void presentationTransitionWillBegin() {}
    virtual void presentationTransitionDidEnd(bool completed) {}
    virtual void dismissalTransitionWillBegin() {}
    virtual void dismissalTransitionDidEnd(bool completed) {}
    virtual void containerViewWillLayoutSubviews() {}
    virtual void containerViewDidLayoutSubviews() {}

private:
    friend class UIViewController;
    friend class UIWindow;

    void setContainerView(const std::shared_ptr<UIView>& containerView);

    std::weak_ptr<UIViewController> _presentedViewController;
    std::weak_ptr<UIViewController> _presentingViewController;
    std::weak_ptr<UIView> _containerView;
};

} // namespace NXKit
