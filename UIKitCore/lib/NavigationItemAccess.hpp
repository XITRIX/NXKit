#pragma once
#include <UINavigationItem.h>
#include <algorithm>
namespace NXKit::Internal {
class NavigationItemAccess {
public:
    static void observe(UINavigationItem& item,const std::shared_ptr<void>& owner,std::function<void(bool)> changed) {
        std::erase_if(item._observers,[&](const auto& observer){return observer.owner.expired()||observer.owner.lock()==owner;});
        item._observers.push_back({owner,std::move(changed)});
    }
    static void defaultTitle(UINavigationItem& item,const std::string& title) {
        if(item._explicitTitle||item._title==title)return;item._title=title;item.changed();
    }
};
}
