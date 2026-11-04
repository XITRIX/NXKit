#pragma once

#include <UIViewController.h>
#include <UITabBarView.h>
#include <deque>
#include <functional>

namespace NXKit {
class UITabBarController;
class UITabBarControllerDelegate {
public:
    virtual ~UITabBarControllerDelegate() = default;
    virtual bool tabBarControllerShouldSelect(const std::shared_ptr<UITabBarController>& controller,
                                              const std::shared_ptr<UIViewController>& candidate) { return true; }
    virtual void tabBarControllerDidSelect(const std::shared_ptr<UITabBarController>& controller,
                                           const std::shared_ptr<UIViewController>& selected) {}
};

/** Bottom tab container. automatic currently resolves to tabBar on all platforms;
    tabSidebar is deferred and rejected. More/customization and UITab groups are
    not yet implemented. Configure its managed bar through child tabBarItems. */
class UITabBarController : public UIViewController, public UITabBarControllerInterface, public UITabBarDelegate {
public:
    using Mode = UITabBarControllerMode;
    UITabBarController() = default;
    [[nodiscard]] Mode mode() const override { return _mode; }
    void setMode(Mode mode) override;
    [[nodiscard]] std::vector<std::shared_ptr<UIViewController>> viewControllers() const override { return _controllers; }
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers) override;
    void setViewControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool animated);
    [[nodiscard]] std::optional<size_t> selectedIndex() const override { return _selectedIndex; }
    bool setSelectedIndex(size_t index) override;
    [[nodiscard]] std::shared_ptr<UIViewController> selectedViewController() const override;
    bool setSelectedViewController(const std::shared_ptr<UIViewController>& controller) override;
    [[nodiscard]] std::shared_ptr<UITabBar> tabBar();
    [[nodiscard]] std::shared_ptr<UITabBarControllerDelegate> delegate() const { return _delegate.lock(); }
    void setDelegate(const std::shared_ptr<UITabBarControllerDelegate>& delegate) { _delegate = delegate; }
    void loadView() override;
    void viewDidLoad() override;
    void viewWillAppear(bool animated) override;
    void viewDidAppear(bool animated) override;
    void viewWillDisappear(bool animated) override;
    void viewDidDisappear(bool animated) override;
    void viewDidLayoutSubviews() override;
    void viewSafeAreaInsetsDidChange() override;
    std::vector<std::shared_ptr<UIFocusEnvironment>> preferredFocusEnvironments() override;
    bool allowsActionDispatch() const override {
        return !_updating && !_appearanceTransition && UIViewController::allowsActionDispatch();
    }
    void tabBarDidSelect(const std::shared_ptr<UITabBar>& bar,
                         const std::shared_ptr<UITabBarItem>& item) override;
private:
    Mode _mode = Mode::automatic;
    std::vector<std::shared_ptr<UIViewController>> _controllers;
    std::optional<size_t> _selectedIndex;
    std::weak_ptr<UITabBarControllerDelegate> _delegate;
    std::shared_ptr<UITabBar> _tabBar;
    std::shared_ptr<UIView> _contentView;
    std::shared_ptr<UIView> _displayedHost;
    std::shared_ptr<UIViewController> _displayed;
    std::deque<std::function<void()>> _pending;
    bool _updating = false, _visible = false, _appearanceTransition = false;
    size_t _revision = 0;
    UIEdgeInsets _reservedInsets;
    void runUpdate(std::function<void()> update);
    void drainUpdates();
    void validate(const std::vector<std::shared_ptr<UIViewController>>& controllers);
    void applyControllers(std::vector<std::shared_ptr<UIViewController>> controllers, bool animated);
    void applySelection(const std::shared_ptr<UIViewController>& controller, bool animateBar);
    void synchronizeBar(bool animated = false);
    void updateDisplayedController();
    void updateLayout();
    bool handleSelection(const std::shared_ptr<UITabBarItem>& item, bool user);
};
}
