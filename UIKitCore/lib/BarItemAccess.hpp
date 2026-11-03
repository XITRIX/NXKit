#pragma once
#include <UIBarItem.h>
#include <algorithm>

namespace NXKit::Internal {
class BarItemAccess {
public:
    static void observe(UIBarItem& item, const std::shared_ptr<void>& owner,
                        std::function<void()> changed) {
        std::erase_if(item._observers, [&](const auto& observer) {
            return observer.owner.expired() || observer.owner.lock() == owner;
        });
        item._observers.push_back({owner, std::move(changed)});
    }
    static void setDefaultTitle(UIBarItem& item, const std::string& title) {
        if (item._hasExplicitTitle || item._title == title) return;
        item._title = title;
        item.notifyChanged();
    }
};
}
