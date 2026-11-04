#pragma once

#include <UIViewController.h>

namespace NXKit::Internal {

class ViewControllerContentScrollAccess {
public:
    /** Applies the controller's override/selection, then follows only the first
        subview chain. Resolution does not load an unloaded controller's view. */
    static std::shared_ptr<UIScrollView> resolvedContentScrollView(
        UIViewController& controller, NSDirectionalRectEdge edge);
};

}
