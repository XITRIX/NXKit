#pragma once
#include <UIView.h>
#include <algorithm>
#include <cmath>
#include <vector>
namespace NXKit::Internal {
/** An out-of-line row in the bar. Search and palette rows can use the same
    extent allocation and settling boundaries without changing scroll physics.
    This is deliberately internal until those components have usable APIs. */
struct NavigationBarAccessory {
    std::shared_ptr<UIView> view;
    NXFloat expandedHeight=0,minimumHeight=0;
};
struct NavigationBarAccessoryLayout {
    std::vector<NavigationBarAccessory> rows;
    NXFloat expandedHeight() const {NXFloat h=0;for(const auto& row:rows)h+=row.expandedHeight;return h;}
    NXFloat collapseRange() const {NXFloat h=0;for(const auto& row:rows)h+=row.expandedHeight-row.minimumHeight;return h;}
    NXFloat visibleHeight(NXFloat offset) const {return expandedHeight()-std::clamp(offset,0.f,collapseRange());}
    NXFloat snap(NXFloat offset,NXFloat velocity) const {
        NXFloat boundary=0;
        for(const auto& row:rows) {
            const auto end=boundary+row.expandedHeight-row.minimumHeight;
            if(offset>boundary&&offset<end)return std::abs(velocity)>.15f?(velocity>0?end:boundary):(offset<(boundary+end)*.5f?boundary:end);
            boundary=end;
        }
        return offset;
    }
    void arrange(NXFloat width,NXFloat top,NXFloat offset) const {
        for(const auto& row:rows) {
            const auto consumed=std::clamp(offset,0.f,row.expandedHeight-row.minimumHeight);
            const auto height=row.expandedHeight-consumed;
            row.view->setFrame({0,top,width,height});
            top+=height;offset-=consumed;
        }
    }
};
}
