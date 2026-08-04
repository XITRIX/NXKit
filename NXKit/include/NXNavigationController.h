#pragma once

#include <NXNavigationItem.h>
#include <UIViewController.h>

#include <deque>
#include <memory>
#include <vector>

namespace NXKit {

class NXNavigationController;
class UILabel;
class UIView;
class UIImageView;

class NXNavigationControllerDelegate {
public:
    virtual ~NXNavigationControllerDelegate() = default;

    virtual void navigationControllerWillShow(
        const std::shared_ptr<NXNavigationController>& navigationController,
        const std::shared_ptr<UIViewController>& viewController,
        bool animated
    ) {}

    virtual void navigationControllerDidShow(
        const std::shared_ptr<NXNavigationController>& navigationController,
        const std::shared_ptr<UIViewController>& viewController,
        bool animated
    ) {}
};

class NXNavigationController : public UIViewController {
public:
    explicit NXNavigationController(const std::shared_ptr<UIViewController>& rootViewController);

    std::weak_ptr<NXNavigationControllerDelegate> delegate;

    [[nodiscard]] std::vector<std::shared_ptr<UIViewController>> viewControllers() const {
        return _viewControllers;
    }
    void setViewControllers(
        std::vector<std::shared_ptr<UIViewController>> viewControllers,
        bool animated
    );

    void pushViewController(
        const std::shared_ptr<UIViewController>& viewController,
        bool animated
    );
    std::shared_ptr<UIViewController> popViewController(bool animated);
    std::vector<std::shared_ptr<UIViewController>> popToViewController(
        const std::shared_ptr<UIViewController>& viewController,
        bool animated
    );
    std::vector<std::shared_ptr<UIViewController>> popToRootViewController(bool animated);

    [[nodiscard]] std::shared_ptr<UIViewController> topViewController() const;
    [[nodiscard]] std::shared_ptr<UIViewController> visibleViewController() const;
    [[nodiscard]] bool isTransitioning() const { return _isTransitioning; }

    std::shared_ptr<NXNavigationItem> navigationItemFor(
        const std::shared_ptr<UIViewController>& viewController
    );
    std::shared_ptr<NXNavigationItem> topNavigationItem();

    [[nodiscard]] std::shared_ptr<NXNavigationStatusView> defaultStatusWidget() const {
        return _defaultStatusWidget;
    }
    [[nodiscard]] std::shared_ptr<NXNavigationActionsView> defaultActionsWidget() const {
        return _defaultActionsWidget;
    }

    void show(
        const std::shared_ptr<UIViewController>& viewController,
        const std::shared_ptr<UIResponder>& sender = nullptr
    ) override;
    bool allowsActionDispatch() const override {
        return !_isTransitioning && UIViewController::allowsActionDispatch();
    }

    void loadView() override;
    void viewDidLoad() override;
    void viewWillAppear(bool animated) override;
    void viewDidAppear(bool animated) override;
    void viewWillDisappear(bool animated) override;
    void viewDidDisappear(bool animated) override;
    void viewSafeAreaInsetsDidChange() override;
    void viewDidLayoutSubviews() override;

private:
    enum class Operation {
        none,
        push,
        pop,
    };

    struct PendingStackUpdate {
        std::vector<std::shared_ptr<UIViewController>> viewControllers;
        bool animated;
    };

    std::vector<std::shared_ptr<UIViewController>> _viewControllers;
    std::deque<PendingStackUpdate> _pendingStackUpdates;
    std::shared_ptr<UIViewController> _visibleViewController;
    std::shared_ptr<UIView> _navigationBar;
    std::shared_ptr<UIView> _toolbar;
    std::shared_ptr<UIView> _contentView;
    std::shared_ptr<UIView> _titleContainer;
    std::shared_ptr<UIView> _navigationLeadingContainer;
    std::shared_ptr<UIView> _navigationTrailingContainer;
    std::shared_ptr<UIView> _toolbarLeadingContainer;
    std::shared_ptr<UIView> _toolbarTrailingContainer;
    std::shared_ptr<UIImageView> _iconView;
    std::shared_ptr<UILabel> _titleLabel;
    std::shared_ptr<NXNavigationStatusView> _defaultStatusWidget;
    std::shared_ptr<NXNavigationActionsView> _defaultActionsWidget;
    std::vector<std::shared_ptr<UIView>> _mountedItemWidgets;
    bool _isTransitioning = false;

    struct NavigationItemAssociation {
        std::weak_ptr<UIViewController> viewController;
        std::shared_ptr<NXNavigationItem> item;
    };
    std::vector<NavigationItemAssociation> _navigationItems;

    void validateViewControllers(
        const std::vector<std::shared_ptr<UIViewController>>& viewControllers
    );
    const std::vector<std::shared_ptr<UIViewController>>& requestedViewControllers() const;
    void beginStackUpdate(
        std::vector<std::shared_ptr<UIViewController>> viewControllers,
        bool animated
    );
    void completeStackUpdate();
    void attachConfiguredChildren();
    void detachRemovedControllers(
        const std::vector<std::shared_ptr<UIViewController>>& controllers
    );
    void updateChrome();
    void updateBarSafeAreas();
    void mountWidget(
        const std::shared_ptr<UIView>& widget,
        NXNavigationWidgetPlacement placement
    );
    std::vector<NXResponderAction> resolvedActions();
    void updateVisibleViewController(
        Operation operation,
        bool animated,
        std::vector<std::shared_ptr<UIViewController>> removedControllers = {}
    );
};

}
