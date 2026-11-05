#pragma once

#include <UIViewController.h>
#include <algorithm>

namespace NXKit::Internal {

class ViewControllerContentScrollAccess {
public:
    static void observe(UIViewController& controller,const std::shared_ptr<void>& owner,std::function<void()> changed) {
        std::erase_if(controller._contentScrollObservers,[&](const auto& entry){return entry.owner.expired()||entry.owner.lock()==owner;});
        controller._contentScrollObservers.push_back({owner,std::move(changed)});
    }
    static void changed(UIViewController& controller) {
        std::erase_if(controller._contentScrollObservers,[](const auto& entry){return entry.owner.expired();});
        const auto observers=controller._contentScrollObservers;
        for(const auto& entry:observers)if(const auto owner=entry.owner.lock())entry.changed();
    }
    /** Applies the controller's override/selection, then follows only the first
        subview chain. Resolution does not load an unloaded controller's view. */
    static std::shared_ptr<UIScrollView> resolvedContentScrollView(
        UIViewController& controller, NSDirectionalRectEdge edge);
};

}
