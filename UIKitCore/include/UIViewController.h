#pragma once

#include <UIView.h>
#include <UITabBarItem.h>
#include <UINavigationItem.h>
#include <UINavigationControllerInterface.h>
#include <UITabBarControllerInterface.h>
#include <UIViewControllerTransitioning.h>
#include <NSDirectionalRectEdge.h>

#include <array>
#include <deque>
#include <utility>

namespace NXKit {

class UIPresentationController;
class UIScrollView;
namespace Internal { class ViewControllerContainerAccess; class ViewControllerContentScrollAccess; }

class UIViewController: public UIResponder, public UITraitEnvironment, public UIFocusEnvironment, public enable_shared_from_this<UIViewController> {
public:
    UIViewController();

    std::map<std::string, std::shared_ptr<UIView>> idStorage;

    bool allowsActionDispatch() const override;

    void setView(std::shared_ptr<UIView> view);
    std::shared_ptr<UIView> view();
    bool viewIsLoaded();
    void loadViewIfNeeded();

    std::shared_ptr<UIResponder> next() override;

    // Most of these methods are designed to be overriden in `UIViewController` subclasses
    virtual void loadView();
    virtual void viewDidLoad() {}
    virtual void viewWillAppear(bool animated);
    virtual void viewDidAppear(bool animated);
    virtual void viewWillDisappear(bool animated);
    virtual void viewDidDisappear(bool animated);

    virtual void viewSafeAreaInsetsDidChange() {}
    virtual void viewLayoutMarginsDidChange() {}
    virtual void viewWillLayoutSubviews() {}
    virtual void viewDidLayoutSubviews() {}

    std::vector<std::shared_ptr<UIViewController>> children() { return _children; }
    std::weak_ptr<UIViewController> parent() { return _parent; }

    /** Nearest enclosing container implementing the capability. These accessors
        follow actual containment, exclude self, and do not retain ancestors. */
    [[nodiscard]] std::shared_ptr<UINavigationControllerInterface> navigationController() const;
    [[nodiscard]] std::shared_ptr<UITabBarControllerInterface> tabBarController() const;

    /** Selects the scroll view observed by containing bars for one or more edges.
        A null value restores automatic discovery. This reference is non-owning. */
    void setContentScrollView(const std::shared_ptr<UIScrollView>& scrollView,
                              NSDirectionalRectEdge edge = NSDirectionalRectEdge::all);
    /** Returns the explicit selection for a single edge, or null for automatic
        discovery. Override and return nullptr directly to disable observation
        for an edge; return the base result to preserve its automatic fallback. */
    [[nodiscard]] virtual std::shared_ptr<UIScrollView> contentScrollViewForEdge(
        NSDirectionalRectEdge edge);


    std::string title() { return _title; }
    void setTitle(std::string title);
    [[nodiscard]] std::shared_ptr<UINavigationItem> navigationItem();
    [[nodiscard]] std::shared_ptr<UITabBarItem> tabBarItem();
    void setTabBarItem(const std::shared_ptr<UITabBarItem>& item);

    void addChild(const std::shared_ptr<UIViewController>& child);
    virtual void willMoveToParent(const std::shared_ptr<UIViewController>& parent);
    virtual void didMoveToParent(std::shared_ptr<UIViewController> parent);
    void removeFromParent();

    UIEdgeInsets additionalSafeAreaInsets() { return _additionalSafeAreaInsets; }
    void setAdditionalSafeAreaInsets(UIEdgeInsets additionalSafeAreaInsets);

    UIEdgeInsets systemMinimumLayoutMargins() { return _systemMinimumLayoutMargins; }

    bool viewRespectsSystemMinimumLayoutMargins() const { return _viewRespectsSystemMinimumLayoutMargins; }
    void setViewRespectsSystemMinimumLayoutMargins(bool viewRespectsSystemMinimumLayoutMargins);

    void present(const std::shared_ptr<UIViewController>& otherViewController, bool animated, const std::function<void()>& completion = [](){});
    void dismiss(bool animated, const std::function<void()>& completion = [](){});

    [[nodiscard]] std::shared_ptr<UIViewController> presentedViewController();
    [[nodiscard]] std::shared_ptr<UIViewController> presentingViewController();

    [[nodiscard]] UIModalPresentationStyle modalPresentationStyle() const {
        return _modalPresentationStyle;
    }
    void setModalPresentationStyle(UIModalPresentationStyle presentationStyle) {
        _modalPresentationStyle = presentationStyle;
    }

    [[nodiscard]] UIModalTransitionStyle modalTransitionStyle() const {
        return _modalTransitionStyle;
    }
    void setModalTransitionStyle(UIModalTransitionStyle transitionStyle) {
        _modalTransitionStyle = transitionStyle;
    }

    [[nodiscard]] std::shared_ptr<UIViewControllerTransitioningDelegate>
    transitioningDelegate() const {
        return _transitioningDelegate.lock();
    }
    void setTransitioningDelegate(
        const std::shared_ptr<UIViewControllerTransitioningDelegate>& transitioningDelegate
    ) {
        _transitioningDelegate = transitioningDelegate;
    }

    [[nodiscard]] std::shared_ptr<UIPresentationController> presentationController();
    [[nodiscard]] bool isBeingPresented();
    [[nodiscard]] bool isBeingDismissed();

    virtual void show(
        const std::shared_ptr<UIViewController>& viewController,
        const std::shared_ptr<UIResponder>& sender = nullptr
    );


    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;

    // Focus
    std::shared_ptr<UIFocusEnvironment> parentFocusEnvironment() override;

private:
    friend class UIView;
    friend class Internal::ViewControllerContainerAccess;
    friend class Internal::ViewControllerContentScrollAccess;
    std::array<std::weak_ptr<UIScrollView>, 4> _contentScrollViews;
    std::array<std::uint64_t, 4> _contentScrollViewGetterGeneration {};
    struct TabBarItemObserver { std::weak_ptr<void> owner; std::function<void()> changed; };
    std::vector<TabBarItemObserver> _tabBarItemObservers;
    std::vector<TabBarItemObserver> _contentScrollObservers;
    std::shared_ptr<UITabBarItem> _tabBarItem;
    std::shared_ptr<UINavigationItem> _navigationItem;
    bool _ownsDefaultTabBarItem = false;
    UIEdgeInsets _containerSafeAreaInsets;
    void invalidateContainerSafeAreaInsets();

    enum class PendingModalOperationKind {
        present,
        dismiss,
    };

    struct PendingModalOperation {
        PendingModalOperationKind kind;
        std::weak_ptr<UIViewController> source;
        std::shared_ptr<UIViewController> presentedViewController;
        bool animated;
        std::function<void()> completion;
    };

    std::shared_ptr<UIView> _view;
    std::weak_ptr<UIViewController> _parent;
    std::vector<std::shared_ptr<UIViewController>> _children;
    UIEdgeInsets _additionalSafeAreaInsets;
    UIEdgeInsets _systemMinimumLayoutMargins = UIEdgeInsets(0, 16, 0, 16);
    bool _viewRespectsSystemMinimumLayoutMargins = true;
    std::string _title = "";

    std::shared_ptr<UIViewController> _presentedViewController;
    std::weak_ptr<UIViewController> _presentingViewController;
    std::weak_ptr<UIViewControllerTransitioningDelegate> _transitioningDelegate;
    std::shared_ptr<UIPresentationController> _presentationController;
    std::shared_ptr<UIViewControllerContextTransitioning> _activeTransitionContext;
    UIModalPresentationStyle _modalPresentationStyle = UIModalPresentationStyle::fullScreen;
    UIModalTransitionStyle _modalTransitionStyle = UIModalTransitionStyle::coverVertical;
    bool _isBeingPresented = false;
    bool _isBeingDismissed = false;
    bool _isPerformingModalTransition = false;
    bool _activePresentationCanBeInterrupted = false;
    bool _isReversingModalPresentation = false;
    bool _isModalTransitionInFlight = false;
    bool _isDrainingModalOperations = false;
    std::deque<PendingModalOperation> _pendingModalOperations;

    std::shared_ptr<UIViewController> rootVC();
    std::shared_ptr<UIViewController> modalTransitionCoordinator();
    void enqueueModalOperation(PendingModalOperation operation);
    void completeModalTransition();
    void drainPendingModalOperations();
};

}
