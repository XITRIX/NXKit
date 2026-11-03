#pragma once
#include <UIView.h>
namespace NXKit::Internal {
class ViewAppearanceAccess {
public:
    static bool hasExplicitTint(const UIView& view) {
        if(view._tintColor && view._tintColor!=UIColor::tint)return true;
        for(auto parent=view.superview().lock();parent;parent=parent->superview().lock())
            if(parent->_tintColor && parent->_tintColor!=UIColor::tint)return true;
        return false;
    }
};
}
