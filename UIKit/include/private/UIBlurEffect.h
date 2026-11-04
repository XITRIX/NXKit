#pragma once
#include <Geometry.h>
#include <memory>
#include <utility>
namespace NXKit {
class UIImage;
/** Explicit private UIKit extension, observed on iOS 26. This is not the
    public UIBlurEffect style API. Mask alpha selects the local blur radius. */
class UIBlurEffect {
public:
    static UIBlurEffect effectWithVariableBlurRadius(NXFloat radius,const std::shared_ptr<UIImage>& imageMask);
    [[nodiscard]] NXFloat blurRadius() const { return _radius; }
    [[nodiscard]] std::shared_ptr<UIImage> imageMask() const { return _mask; }
private:
    UIBlurEffect(NXFloat radius,std::shared_ptr<UIImage> mask):_radius(radius),_mask(std::move(mask)){}
    NXFloat _radius;
    std::shared_ptr<UIImage> _mask;
};
}
