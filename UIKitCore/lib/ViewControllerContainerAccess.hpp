#pragma once
#include <UIViewController.h>
#include <algorithm>

namespace NXKit::Internal {
class ViewControllerContainerAccess {
public:
    static void observeTabBarItem(UIViewController& controller, const std::shared_ptr<void>& owner,
                                  std::function<void()> changed) {
        std::erase_if(controller._tabBarItemObservers, [&](const auto& observer) {
            return observer.owner.expired() || observer.owner.lock() == owner;
        });
        controller._tabBarItemObservers.push_back({owner, std::move(changed)});
    }
    static void setChildSafeAreaInsets(const std::shared_ptr<UIViewController>& parent,
                                       const std::shared_ptr<UIViewController>& child,
                                       UIEdgeInsets insets) {
        if (child->parent().lock() != parent || child->_containerSafeAreaInsets == insets) return;
        child->_containerSafeAreaInsets = insets;
        child->invalidateContainerSafeAreaInsets();
    }
};
}
