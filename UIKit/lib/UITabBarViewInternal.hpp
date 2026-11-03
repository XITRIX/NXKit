#pragma once
#include <UITabBarView.h>
#include <UIKit.h>
#include <UITouch.h>
#include <CADisplayLink.h>
#include <UIPanGestureRecognizer.h>
#include <chrono>
#include <functional>

namespace NXKit::Internal {
struct TabBarViewState {
    size_t _count = 0;
    bool _compact = false, _lifted = false, _dragging = false;
    size_t _selected = 0, _dragStart = 0;
    float _position = 0, _selectionStep = 1;
    std::vector<NXKit::NXFloat> _selectionCenters;
    std::weak_ptr<NXKit::UITouch> _trackingTouch;
    NXKit::NXPoint _lastContact;
    NXKit::NXFloat _grabOffsetX = 0;
    bool _grabbedLens = false;
    NXKit::Timer _lastContactTime;
    double _minimumLiftRemaining = 0;
    std::shared_ptr<NXKit::CADisplayLink> _liftLink;
    std::chrono::steady_clock::time_point _lastLiftFrame;
    std::weak_ptr<NXKit::UIView> _liftParent;
    std::weak_ptr<NXKit::UIWindow> _liftWindow;
    std::optional<NXKit::NXRect> _comparisonFrame;
    std::shared_ptr<NXKit::BackdropEffectView> _glass, _selectionFill;
    std::shared_ptr<NXKit::UIView> _normal, _selectedHost, _selectedContent;
    std::shared_ptr<NXKit::LensView> _lens;
    std::shared_ptr<NXKit::UIImageView> _normalImage, _selectedImage;
    std::vector<std::shared_ptr<NXKit::UIControl>> _controls;
    std::vector<std::shared_ptr<NXKit::UIView>> _normalSymbols, _selectedSymbols, _selectedItems;
    std::vector<std::shared_ptr<NXKit::UILabel>> _normalLabels, _selectedLabels;

    std::vector<std::shared_ptr<UITabBarItem>> items;
    std::weak_ptr<UITabBarDelegate> delegate;
    std::weak_ptr<UITabBarDelegate> owner;
    std::function<bool(const std::shared_ptr<UITabBarItem>&, bool)> managedSelection;
    std::function<void(size_t)> legacySelectionChanged;
    std::function<void()> customLayout;
    std::optional<UIColor> unselectedTint;
    std::vector<std::shared_ptr<UIView>> badges;
    std::vector<std::shared_ptr<UILabel>> badgeLabels;
    std::shared_ptr<UIPanGestureRecognizer> pan;
    bool hasSelection = false;
    bool resetting = false;
    size_t generation = 0;
};
class TabBarViewAccess {
public:
    static const TabBarViewState& state(const UITabBarView& view) { return *view._state; }
    static TabBarViewState& state(UITabBarView& view) { return *view._state; }
    static void setItems(UITabBarView& view, std::vector<std::shared_ptr<UITabBarItem>> items, bool animated) {
        view.installItems(std::move(items), animated);
    }
    static void select(UITabBarView& view, const std::shared_ptr<UITabBarItem>& item, bool animated = false) {
        view.commitSelection(item, animated);
    }
    static void arrange(UITabBarView& view) { view.arrange(); }
    static void advance(UITabBarView& view, double seconds) { view.advanceLift(seconds); }
    static void position(UITabBarView& view, float position) { view.setSelectionPosition(position); }
    static void lift(UITabBarView& view, bool lifted, bool animated) { view.setLifted(lifted, animated); }
    static void comparisonContent(UITabBarView& view, std::shared_ptr<UIImage> normal,
                                  std::shared_ptr<UIImage> selected) {
        view.setComparisonContent(std::move(normal), std::move(selected));
    }
    static void comparisonGeometry(UITabBarView& view, NXRect frame) { view.setComparisonGeometry(frame); }
};
}
