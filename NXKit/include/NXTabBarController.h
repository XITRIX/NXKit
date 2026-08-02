#pragma once

#include <IndexPath.h>
#include <NXControl.h>
#include <UILabel.h>
#include <UIScrollView.h>
#include <UIViewController.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace NXKit {

class NXTabBarController;

class NXTabBarControllerDelegate {
public:
    virtual ~NXTabBarControllerDelegate() = default;

    virtual bool tabBarControllerShouldSelect(
        const std::shared_ptr<NXTabBarController>& tabBarController,
        const std::shared_ptr<UIViewController>& viewController
    ) { return true; }

    virtual void tabBarControllerDidSelect(
        const std::shared_ptr<NXTabBarController>& tabBarController,
        const std::shared_ptr<UIViewController>& viewController
    ) {}
};

class NXTabBarButton: public NXControl {
public:
    NXTabBarButton();

    void didUpdateFocusIn(UIFocusUpdateContext context, UIFocusAnimationCoordinator* coordinator) override;
    void setSelected(bool selected) override;

    [[nodiscard]] std::string text() const { return _titleLabel->text(); }
    void setText(const std::string& text);

    [[nodiscard]] std::shared_ptr<UILabel> titleLabel() const { return _titleLabel; }

    [[nodiscard]] bool selectionFollowsFocus() const { return _selectionFollowsFocus; }
    void setSelectionFollowsFocus(bool selectionFollowsFocus) { _selectionFollowsFocus = selectionFollowsFocus; }

private:
    friend class NXTabBar;

    std::shared_ptr<UILabel> _titleLabel;
    std::shared_ptr<UIView> _rectView;
    bool _selectionFollowsFocus = true;
    std::function<void()> _selectionAction;
};

class NXTabBar : public UIScrollView {
public:
    using ItemSection = std::vector<std::string>;
    using Items = std::vector<ItemSection>;

    NXTabBar();

    std::function<void(const IndexPath&)> selectionDidChange;

    [[nodiscard]] const Items& items() const { return _items; }
    void setItems(Items items);

    [[nodiscard]] std::optional<IndexPath> selectedIndexPath() const { return _selectedIndexPath; }
    bool setSelectedIndexPath(const IndexPath& indexPath);
    bool activateItemAt(const IndexPath& indexPath);
    void clearSelection();

    [[nodiscard]] bool selectionFollowsFocus() const { return _selectionFollowsFocus; }
    void setSelectionFollowsFocus(bool selectionFollowsFocus);

private:
    friend class NXTabBarController;

    [[nodiscard]] bool contains(const IndexPath& indexPath) const;
    [[nodiscard]] std::optional<IndexPath> firstIndexPath() const;

    Items _items;
    std::optional<IndexPath> _selectedIndexPath;
    std::shared_ptr<UIView> _container;
    std::vector<std::vector<std::shared_ptr<NXTabBarButton>>> _buttons;
    bool _selectionFollowsFocus = true;
    std::function<bool(const IndexPath&)> _selectionHandler;
    std::function<void(const IndexPath&)> _primaryActionHandler;
};

class NXTabBarController : public UIViewController {
public:
    using ViewControllerSection = std::vector<std::shared_ptr<UIViewController>>;
    using ViewControllerSections = std::vector<ViewControllerSection>;

    NXTabBarController() = default;
    explicit NXTabBarController(ViewControllerSection controllers);
    explicit NXTabBarController(ViewControllerSections controllers);

    std::weak_ptr<NXTabBarControllerDelegate> delegate;

    [[nodiscard]] std::vector<std::shared_ptr<UIViewController>> viewControllers() const;
    [[nodiscard]] const ViewControllerSections& viewControllerSections() const { return _viewControllers; }
    void setViewControllers(ViewControllerSection controllers);
    void setViewControllerSections(ViewControllerSections controllers);

    [[nodiscard]] std::optional<size_t> selectedIndex() const;
    bool setSelectedIndex(size_t index);

    [[nodiscard]] std::optional<IndexPath> selectedIndexPath() const { return _selectedIndexPath; }
    bool setSelectedIndexPath(const IndexPath& indexPath);

    [[nodiscard]] std::shared_ptr<UIViewController> selectedViewController() const;
    bool setSelectedViewController(const std::shared_ptr<UIViewController>& viewController);

    std::shared_ptr<NXTabBar> tabBar();

    [[nodiscard]] NXFloat tabBarWidth() const { return _tabBarWidth; }
    void setTabBarWidth(NXFloat tabBarWidth);

    void reloadTabBarItems();

    void loadView() override;
    void viewDidLoad() override;
    void viewWillAppear(bool animated) override;
    void viewDidAppear(bool animated) override;
    void viewWillDisappear(bool animated) override;
    void viewDidDisappear(bool animated) override;
private:
    [[nodiscard]] bool contains(const IndexPath& indexPath) const;
    [[nodiscard]] std::optional<IndexPath> firstIndexPath() const;
    [[nodiscard]] std::optional<IndexPath> indexPathForFlatIndex(size_t index) const;
    [[nodiscard]] std::optional<IndexPath> indexPathOf(
        const std::shared_ptr<UIViewController>& viewController
    ) const;
    [[nodiscard]] std::shared_ptr<UIViewController> viewControllerAt(const IndexPath& indexPath) const;

    static void validateViewControllers(const ViewControllerSections& controllers);
    void attachConfiguredChildren();
    void removeUnconfiguredChildren(
        const std::vector<std::shared_ptr<UIViewController>>& previousViewControllers
    );
    bool handleUserSelection(const IndexPath& indexPath);
    bool focusPresentedViewController();
    bool focusSelectedTab();
    void restoreTabBarSelection();
    std::shared_ptr<UIViewController> _presentedViewController;
    ViewControllerSections _viewControllers;
    std::optional<IndexPath> _selectedIndexPath;
    std::shared_ptr<NXTabBar> _tabBar;
    std::shared_ptr<UIView> _contentView;
    NXFloat _tabBarWidth = 410;

    void updateTabSelection();
};

}
