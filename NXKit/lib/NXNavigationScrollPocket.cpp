#include "NXNavigationScrollPocket.hpp"
#include "ViewControllerContentScrollAccess.hpp"

#include <UIScrollView.h>
#include <UIScrollEdgeEffect.h>

#include <algorithm>
#include <cmath>

namespace NXKit::Internal {

NXRect navigationScrollPocketMaterialRect(UIViewController& controller,
                                        const std::shared_ptr<UIView>& bar,
                                        NSDirectionalRectEdge edge) {
    if (!bar || !controller.viewIsLoaded()
        || (edge != NSDirectionalRectEdge::top && edge != NSDirectionalRectEdge::bottom)) return {};
    auto scroll = ViewControllerContentScrollAccess::resolvedContentScrollView(controller, edge);
    if (!scroll) return {};
    const auto effect = edge == NSDirectionalRectEdge::top
        ? scroll->topEdgeEffect() : scroll->bottomEdgeEffect();
    if (!effect || effect->isHidden()) return {};

    // An explicitly selected view must still belong to this screen. Hidden or
    // removed content never lends a material to the navigation controller.
    auto ancestor = std::static_pointer_cast<UIView>(scroll);
    while (ancestor && ancestor != controller.view()) {
        if (ancestor->isHidden() || ancestor->alpha() <= 0) return {};
        ancestor = ancestor->superview().lock();
    }
    if (!ancestor || ancestor->isHidden() || ancestor->alpha() <= 0) return {};

    const auto modelBounds = scroll->bounds();
    const auto bounds = scroll->layer()->presentationOrSelf()->bounds();
    const auto safe = scroll->safeAreaInsets();
    const auto contentHeight = scroll->contentSize().height;
    if (!std::isfinite(contentHeight) || contentHeight <= 0
        || bounds.width() <= 0 || bounds.height() <= 0) return {};
    NXFloat inset = edge == NSDirectionalRectEdge::top ? safe.top : safe.bottom;
    inset = std::clamp(inset, NXFloat(0), bounds.height());
    if (inset <= 0) return {};
    const NXFloat safeBoundary = edge == NSDirectionalRectEdge::top
        ? bounds.minY() + inset : bounds.maxY() - inset;
    if (edge == NSDirectionalRectEdge::top
        ? safeBoundary <= 0 || bounds.minY() >= contentHeight
        : safeBoundary >= contentHeight || bounds.maxY() <= 0) return {};

    const NXFloat originY = edge == NSDirectionalRectEdge::top
        ? modelBounds.minY() : modelBounds.maxY() - inset;
    const auto a = scroll->convertToView({modelBounds.minX(), originY}, bar);
    const auto b = scroll->convertToView({modelBounds.maxX(), originY + inset}, bar);
    const auto barBounds = bar->bounds();
    const NXFloat left = std::max(barBounds.minX(), std::min(a.x, b.x));
    const NXFloat right = std::min(barBounds.maxX(), std::max(a.x, b.x));
    const NXFloat top = std::max(barBounds.minY(), std::min(a.y, b.y));
    const NXFloat bottom = std::min(barBounds.maxY(), std::max(a.y, b.y));
    if (!std::isfinite(left) || !std::isfinite(right)
        || !std::isfinite(top) || !std::isfinite(bottom)
        || right <= left || bottom <= top) return {};
    return {left, top, right - left, bottom - top};
}

void NavigationScrollPocketLayer::draw(SkCanvas* context) {
    const auto rect = _regionProvider ? _regionProvider() : NXRect{};
    if (rect.width() <= 0 || rect.height() <= 0) return;
    context->save();
    context->clipRect(SkRect::MakeXYWH(rect.minX(), rect.minY(), rect.width(), rect.height()));
    CABlurLayer::draw(context);
    context->restore();
}

}
