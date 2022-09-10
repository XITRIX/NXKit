//
//  UIImageView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#include <Core/UIImageView/UIImageView.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

static float measureWidth(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode, float originalWidth, ImageScalingType type)
{
    if (widthMode == YGMeasureModeUndefined)
        return originalWidth;
    else if (widthMode == YGMeasureModeAtMost)
        if (type == ImageScalingType::FIT)
            return originalWidth;
        else
            return std::min(width, originalWidth);
    else if (widthMode == YGMeasureModeExactly)
        return width;
//    else
//        fatal("Unsupported Image width measure mode: " + std::to_string(widthMode));


    return width;
}

static float measureHeight(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode, float originalHeight, ImageScalingType type)
{
    if (heightMode == YGMeasureModeUndefined)
        return originalHeight;
    else if (heightMode == YGMeasureModeAtMost)
        if (type == ImageScalingType::FIT)
            return originalHeight;
        else
            return std::min(height, originalHeight);
    else if (heightMode == YGMeasureModeExactly)
        return height;
//    else
//        fatal("Unsupported Image height measure mode: " + std::to_string(heightMode));

    return height;
}

static YGSize imageMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode)
{
    UIImageView* image           = (UIImageView*)node->getContext();
//    int texture                  = image->getImage()->;
    float originalWidth          = image->getImage()->getSize().width;
    float originalHeight         = image->getImage()->getSize().height;
    ImageScalingType scalingType = image->getScalingType();

    YGSize size = {
        .width  = width,
        .height = height,
    };

//    if (texture == 0)
//        return size;

    // Stretched mode: we don't care about the size of the image
    if (scalingType == ImageScalingType::STRETCH)
    {
        return size;
    }
    // Fit scaling mode: scale the view according to image ratio
    else if (scalingType == ImageScalingType::FIT && ntz(height) > 0)
    {
        float imageAspectRatio = originalWidth / originalHeight;

        // Grow height as much as possible then deduce width
        if (heightMode != YGMeasureModeUndefined)
        {
            size.height = measureHeight(node, width, widthMode, height, heightMode, originalHeight, scalingType);
            size.width  = measureWidth(node, width, widthMode, height, heightMode, size.height * imageAspectRatio, scalingType);
        }
        // Grow width as much as possible then deduce height
        else
        {
            size.width  = measureWidth(node, width, widthMode, height, heightMode, originalWidth, scalingType);
            size.height = measureHeight(node, width, widthMode, height, heightMode, size.width * imageAspectRatio, scalingType);
        }
    }
    // Crop (and fallback) method: grow as much as possible in both directions
    else
    {
        size.width  = measureWidth(node, width, widthMode, height, heightMode, originalWidth, scalingType);
        size.height = measureHeight(node, width, widthMode, height, heightMode, originalHeight, scalingType);
    }

    return size;
}

UIImageView::UIImageView() {
    paint.image = -1;
    YGNodeSetMeasureFunc(this->ygNode, imageMeasureFunc);
}

UIImageView::~UIImageView() {
    delete image;
}

void UIImageView::setImage(UIImage* image) {
    if (this->image)
        delete this->image;
    
    this->image = image;
    setNeedsLayout();
}

void UIImageView::setScalingType(ImageScalingType scalingType) {
    this->scalingType = scalingType;
    setNeedsLayout();
}

void UIImageView::draw(NVGcontext* vgContext) {
    if (image->isDrawingTypeTemplate())
        paint.innerColor = getTintColor().raw();
    
    Rect bounds = getBounds();
    nvgBeginPath(vgContext);
    nvgRoundedRect(vgContext, 0, 0, bounds.width(), bounds.height(), cornerRadius);
    nvgFillPaint(vgContext, paint);
    nvgFill(vgContext);
}

void UIImageView::layoutSubviews() {
    UIView::layoutSubviews();
    invalidateImageBounds();
}

void UIImageView::invalidateImageBounds()
{
    if (!image) return;

    Rect bounds = getBounds();
    Size imageSize = image->getSize();

    float viewAspectRatio  = bounds.height() / bounds.width();
    float imageAspectRatio = imageSize.height / imageSize.width;

    float imageHeight = 0;
    float imageWidth = 0;
    float imageX = 0;
    float imageY = 0;

    switch (this->scalingType)
    {
        case ImageScalingType::FIT:
        {
            if (viewAspectRatio <= imageAspectRatio)
            {
                imageHeight = bounds.height();
                imageWidth  = imageHeight / imageAspectRatio;
                imageX      = (bounds.width() - imageWidth) / 2.0F;
                imageY      = 0;
            }
            else
            {
                imageWidth  = bounds.width();
                imageHeight = imageWidth * imageAspectRatio;
                imageY      = (bounds.height() - imageHeight) / 2.0F;
                imageX      = 0;
            }
            break;
        }
        case ImageScalingType::FILL:
        {
            if (viewAspectRatio >= imageAspectRatio)
            {
                imageHeight = bounds.height();
                imageWidth  = imageHeight / imageAspectRatio;
                imageX      = (bounds.width() - imageWidth) / 2.0F;
                imageY      = 0;
            }
            else
            {
                imageWidth  = bounds.width();
                imageHeight = imageWidth * imageAspectRatio;
                imageY      = (bounds.height() - imageHeight) / 2.0F;
                imageX      = 0;
            }
            break;
        }
        case ImageScalingType::STRETCH:
            imageX      = 0;
            imageY      = 0;
            imageWidth  = bounds.width();
            imageHeight = bounds.height();
            break;
        case ImageScalingType::CENTER:
            imageHeight = imageSize.height;
            imageWidth  = imageSize.width;
            imageX      = (bounds.width() - imageWidth) / 2.0F;
            imageY      = (bounds.height() - imageHeight) / 2.0F;
            break;
        default:
            printf("Unimplemented Image scaling type");
    }

    this->paint = image->getPaint();

    this->paint.xform[4] = imageX;
    this->paint.xform[5] = imageY;

    this->paint.extent[0] = imageWidth;
    this->paint.extent[1] = imageHeight;
}

}
