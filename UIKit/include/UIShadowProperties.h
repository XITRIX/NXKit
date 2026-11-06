#pragma once
#include <Geometry.h>
#include <UIColor.h>
#include <memory>
#include <optional>
#include <utility>

namespace NXKit {
class UIView;
/** Portable counterpart of UIKit's private _UIShadowProperties value object.
    This is an NXKit extension, not an Apple public UIKit class. */
class UIShadowProperties {
public:
    NXFloat opacity() const { return _opacity; }
    void setOpacity(NXFloat value);
    std::optional<UIColor> color() const { return _color; }
    void setColor(std::optional<UIColor> value) { _color=std::move(value); }
    NXSize offset() const { return _offset; }
    void setOffset(NXSize value);
    NXFloat radius() const { return _radius; }
    void setRadius(NXFloat value);
    static std::shared_ptr<UIShadowProperties> propertiesFromView(const std::shared_ptr<UIView>& view);
    void applyToView(const std::shared_ptr<UIView>& view) const;
    void scaleBy(NXFloat factor);
    std::shared_ptr<UIShadowProperties> updatedShadowWithIntensity(NXFloat intensity) const;
    std::shared_ptr<UIShadowProperties> copy() const;
private:
    NXFloat _opacity=0, _radius=0;
    std::optional<UIColor> _color=UIColor::black;
    NXSize _offset;
};
}
