//
//  UIImageView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>
#include <Core/UIImage/UIImage.hpp>

namespace NXKit {

enum class ImageScalingType
{
    // The image is scaled to fit the view boundaries, aspect ratio is conserved
    FIT,
    // The image is scaled to fill the view boundaries, aspect ratio is conserved
    FILL,
    // The image is stretched to fit the view boundaries (aspect ratio is not conserved). The original image dimensions are entirely ignored in the layout process.
    STRETCH,
    // The image is either cropped (not enough space) or untouched (too much space)
    CENTER,
};

class UIImageView: public UIView {
public:
    UIImageView();
    virtual ~UIImageView();

    void setImage(std::shared_ptr<UIImage> image, bool freeOld = true);
    std::shared_ptr<UIImage> getImage() { return image; }

    void setScalingType(ImageScalingType scalingType);
    ImageScalingType getScalingType() { return scalingType; }

    void draw(NVGcontext* vgContext) override;
    void layoutSubviews() override;

private:
    NVGpaint paint;
    std::shared_ptr<UIImage> image = nullptr;
    ImageScalingType scalingType = ImageScalingType::FIT;
    
    void invalidateImageBounds();
};

}
