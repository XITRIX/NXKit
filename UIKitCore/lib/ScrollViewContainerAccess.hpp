#pragma once
#include <UIScrollView.h>
#include <algorithm>
namespace NXKit::Internal {
// Container coordination is independent of the application's weak public delegate.
// Multiple containers can observe a scroll view, with lifetimes tied to their owners.
class ScrollViewContainerAccess {
public:
    static void observe(UIScrollView& scroll,const std::shared_ptr<void>& owner,
                        std::function<void()> changed,std::function<void(NXPoint,NXPoint&,NXPoint&)> willEnd,
                        std::function<void(NXRect&)> bounds={}) {
        remove(scroll,owner);
        scroll._containerObservers.push_back({owner,std::move(changed),std::move(willEnd),std::move(bounds)});
    }
    static void remove(UIScrollView& scroll,const std::shared_ptr<void>& owner) {
        std::erase_if(scroll._containerObservers,[&](const auto& entry){return entry.owner.expired()||entry.owner.lock()==owner;});
    }
    static void updateInsets(UIScrollView& scroll,const std::function<void()>& changes) {
        ++scroll._coordinatedInsetChanges;
        struct Restore {UIScrollView& scroll;~Restore(){--scroll._coordinatedInsetChanges;}} restore{scroll};
        changes();
        scroll.synchronizeContainerInsets();
    }
};
}
