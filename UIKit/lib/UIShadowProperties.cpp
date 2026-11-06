#include <UIShadowProperties.h>
#include <UIView.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace NXKit {
namespace {void finite(NXFloat value){if(!std::isfinite(value))throw std::invalid_argument("Shadow properties must be finite");}}
void UIShadowProperties::setOpacity(NXFloat value){finite(value);_opacity=value;}
void UIShadowProperties::setRadius(NXFloat value){finite(value);if(value<0)throw std::invalid_argument("Shadow radius must be nonnegative");_radius=value;}
void UIShadowProperties::setOffset(NXSize value){finite(value.width);finite(value.height);_offset=value;}
std::shared_ptr<UIShadowProperties> UIShadowProperties::propertiesFromView(const std::shared_ptr<UIView>& view) {
    auto result=std::make_shared<UIShadowProperties>();if(!view)return result;
    const auto layer=view->layer();result->_opacity=layer->shadowOpacity();result->_radius=layer->shadowRadius();
    result->_color=layer->shadowColor();const auto offset=layer->shadowOffset();result->_offset={offset.x,offset.y};return result;
}
void UIShadowProperties::applyToView(const std::shared_ptr<UIView>& view) const {
    if(!view)return;auto layer=view->layer();layer->setShadowColor(_color);layer->setShadowOpacity(_opacity);
    layer->setShadowRadius(_radius);layer->setShadowOffset({_offset.width,_offset.height});
}
void UIShadowProperties::scaleBy(NXFloat factor) {
    finite(factor);if(factor<0)throw std::invalid_argument("Shadow scale must be nonnegative");
    const auto radius=_radius*factor;const auto offset=NXSize{_offset.width*factor,_offset.height*factor};
    finite(radius);finite(offset.width);finite(offset.height);_radius=radius;_offset=offset;
}
std::shared_ptr<UIShadowProperties> UIShadowProperties::updatedShadowWithIntensity(NXFloat intensity) const {
    finite(intensity);auto result=copy();result->_opacity*=std::clamp(intensity,0.f,1.f);return result;
}
std::shared_ptr<UIShadowProperties> UIShadowProperties::copy() const {return std::make_shared<UIShadowProperties>(*this);}
}
