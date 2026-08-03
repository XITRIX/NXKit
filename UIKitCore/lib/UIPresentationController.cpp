#include <UIPresentationController.h>

#include <UIView.h>
#include <UIViewController.h>

using namespace NXKit;

UIPresentationController::UIPresentationController(
    const std::shared_ptr<UIViewController>& presentedViewController,
    const std::shared_ptr<UIViewController>& presentingViewController
) : _presentedViewController(presentedViewController),
    _presentingViewController(presentingViewController) {}

std::shared_ptr<UIViewController>
UIPresentationController::presentedViewController() const {
    return _presentedViewController.lock();
}

std::shared_ptr<UIViewController>
UIPresentationController::presentingViewController() const {
    return _presentingViewController.lock();
}

std::shared_ptr<UIView> UIPresentationController::containerView() const {
    return _containerView.lock();
}

std::shared_ptr<UIView> UIPresentationController::presentedView() const {
    const auto controller = presentedViewController();
    return controller ? controller->view() : nullptr;
}

UIModalPresentationStyle UIPresentationController::presentationStyle() const {
    const auto controller = presentedViewController();
    return controller
        ? controller->modalPresentationStyle()
        : UIModalPresentationStyle::fullScreen;
}

NXRect UIPresentationController::frameOfPresentedViewInContainerView() const {
    const auto container = containerView();
    return container ? container->bounds() : NXRect();
}

bool UIPresentationController::shouldRemovePresentersView() const {
    return false;
}

void UIPresentationController::setContainerView(
    const std::shared_ptr<UIView>& containerView
) {
    _containerView = containerView;
}
