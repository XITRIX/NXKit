#include <UIView.h>
#include <UIWindow.h>
#include <SkiaCtx.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace NXKit;

UICornerConfiguration UIView::cornerConfiguration() const {
    return _cornerConfiguration.value_or(UICornerConfiguration::corners(UICornerRadius::fixed(0)));
}
void UIView::setCornerConfiguration(const UICornerConfiguration& configuration) {
    if (_cornerConfiguration == configuration) return;
    _cornerConfiguration = configuration;
    updateCornerConfigurationTree();
}
std::array<NXFloat,4> UIView::resolvedCornerRadii() const {
    if (!_cornerConfiguration) {
        if (dynamic_cast<const UIWindow*>(this)) {
            if (const auto context=SkiaCtx::main()) return context->windowCornerRadii();
        }
        return {};
    }
    const auto& config=*_cornerConfiguration;
    const auto size=bounds().size;
    const auto limit=std::isfinite(size.width)&&std::isfinite(size.height)
        ? std::max(0.f,std::min(size.width,size.height)*.5f) : 0.f;
    if (config._capsule) {
        const auto radius=std::min(limit,config._maximumRadius.value_or(limit));
        return {radius,radius,radius,radius};
    }
    std::array<NXFloat,4> result{};
    for (size_t i=0;i<4;++i) {
        if (!config._radii[i]) continue;
        const auto& radius=*config._radii[i];
        result[i]=radius._value;
        if (!radius._concentric) continue;
        // Unconfigured rectangular wrappers don't establish a corner container.
        auto container=_superview.lock();
        NXPoint location{(i==1||i==3)?bounds().maxX():bounds().minX(),i>=2?bounds().maxY():bounds().minY()};
        const auto toParent=[](NXPoint point,const UIView& view) {
            const auto box=view.bounds(); const auto anchor=view.layer()->anchorPoint();
            point-=box.origin+NXPoint{box.width()*anchor.x,box.height()*anchor.y};
            return point.applying(view.transform())+view.layer()->position();
        };
        location=toParent(location,*this);
        while (container) {
            if (container->_cornerConfiguration || std::dynamic_pointer_cast<UIWindow>(container)) break;
            location=toParent(location,*container);
            container=container->_superview.lock();
        }
        if (!container) continue;
        const auto outer=container->bounds();
        const auto horizontal=(i==1||i==3)?outer.maxX()-location.x:location.x-outer.minX();
        const auto vertical=i>=2?outer.maxY()-location.y:location.y-outer.minY();
        const auto inset=std::max(horizontal,vertical);
        if (std::isfinite(inset)) result[i]=std::max(radius._value,container->resolvedCornerRadii()[i]-inset);
    }
    for (const auto group:config._groups) {
        NXFloat maximum=0;
        for (size_t i=0;i<4;++i) if (group&(1u<<i)) maximum=std::max(maximum,result[i]);
        for (size_t i=0;i<4;++i) if (group&(1u<<i)) result[i]=maximum;
    }
    for (auto& radius:result) radius=std::clamp(radius,0.f,limit);
    return result;
}
NXFloat UIView::effectiveRadius(UIRectCorner corner) const {
    const auto mask=static_cast<unsigned>(corner);
    if (mask&~15u) throw std::invalid_argument("Invalid UIRectCorner mask");
    const auto radii=resolvedCornerRadii();
    NXFloat maximum=0;
    for (size_t i=0;i<4;++i) if (mask&(1u<<i)) maximum=std::max(maximum,radii[i]);
    return maximum;
}
void UIView::updateCornerConfigurationTree() {
    if (_cornerConfiguration) {
        const auto radii=resolvedCornerRadii();
        if (_layer->_configuredCornerRadii != radii) {
            _layer->setConfiguredCornerRadii(radii);
            setNeedsLayout();
        }
    }
    for (const auto& child:_subviews) child->updateCornerConfigurationTree();
    if (_mask) _mask->updateCornerConfigurationTree();
}
void UIView::setTransform(NXAffineTransform transform) {
    _layer->setAffineTransform(transform);
    updateCornerConfigurationTree();
}
void UIView::setTransform3D(NXTransform3D transform) {
    _layer->setTransform(transform);
    updateCornerConfigurationTree();
}

void UIView::clearCornerConfiguration() {
    _cornerConfiguration.reset();
    _layer->_configuredCornerRadii.reset();
    if (_layer->_presentation) _layer->_presentation->_configuredCornerRadii.reset();
    _layer->setNeedsDisplay();
    setNeedsLayout();
    updateCornerConfigurationTree();
}
