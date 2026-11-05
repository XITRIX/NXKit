#include <UIViewController.h>
#include <UIScrollView.h>
#include "ViewControllerContentScrollAccess.hpp"

#include <stdexcept>

namespace NXKit {
namespace {

std::size_t edgeIndex(NSDirectionalRectEdge edge) {
    switch (edge) {
        case NSDirectionalRectEdge::top: return 0;
        case NSDirectionalRectEdge::leading: return 1;
        case NSDirectionalRectEdge::bottom: return 2;
        case NSDirectionalRectEdge::trailing: return 3;
        default:
            throw std::invalid_argument("contentScrollViewForEdge requires a single directional edge");
    }
}

}

void UIViewController::setContentScrollView(const std::shared_ptr<UIScrollView>& scrollView,
                                           NSDirectionalRectEdge edge) {
    const auto mask = static_cast<unsigned>(edge);
    if ((mask & ~static_cast<unsigned>(NSDirectionalRectEdge::all)) != 0) {
        throw std::invalid_argument("setContentScrollView received an invalid directional edge mask");
    }
    for (std::size_t index = 0; index < _contentScrollViews.size(); ++index) {
        if (mask & (1u << index)) _contentScrollViews[index] = scrollView;
    }
    if (_view && mask) {_view->setNeedsLayout();Internal::ViewControllerContentScrollAccess::changed(*this);}
}

std::shared_ptr<UIScrollView> UIViewController::contentScrollViewForEdge(NSDirectionalRectEdge edge) {
    const auto index = edgeIndex(edge);
    ++_contentScrollViewGetterGeneration[index];
    return _contentScrollViews[index].lock();
}

std::shared_ptr<UIScrollView> Internal::ViewControllerContentScrollAccess::resolvedContentScrollView(
    UIViewController& controller, NSDirectionalRectEdge edge) {
    const auto index = edgeIndex(edge);
    const auto generation = controller._contentScrollViewGetterGeneration[index];
    auto selection = controller.contentScrollViewForEdge(edge);
    if (selection) return selection;
    // Objective-C can distinguish an inherited method from an override. In C++,
    // a direct nullptr override is recognized because it never called the base
    // getter. Calling the base getter preserves automatic discovery when unset.
    if (generation == controller._contentScrollViewGetterGeneration[index]
        || !controller._contentScrollViews[index].expired()) return nullptr;

    auto candidate = controller._view;
    while (candidate) {
        if (auto scrollView = std::dynamic_pointer_cast<UIScrollView>(candidate)) return scrollView;
        const auto& children = candidate->subviews();
        candidate = children.empty() ? nullptr : children.front();
    }
    return nullptr;
}

}
