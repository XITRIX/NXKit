#include "CGImage.h"

#include <utility>
#include "include/core/SkData.h"

using namespace NXKit;

//#include <UIApplication/UIApplication.hpp>
//#include <utility>

//CGImage::CGImage(NXSize size) {
//    pointee = SkImages::
////    GPU_SetAnchor(pointee, 0, 0);
////    GPU_SetBlendMode(pointee, GPU_BLEND_NORMAL_FACTOR_ALPHA);
//}

CGImage::CGImage(sk_sp<SkImage> image, std::shared_ptr<NXData> sourceData) {
    this->sourceData = std::move(sourceData);
    pointee = std::move(image);

//    GPU_SetSnapMode(pointee, GPU_SNAP_POSITION_AND_DIMENSIONS);
//    GPU_SetBlendMode(pointee, GPU_BLEND_NORMAL_FACTOR_ALPHA);
//    GPU_SetImageFilter(pointee, GPU_FILTER_LINEAR);
}

CGImage::CGImage(const std::shared_ptr<NXData>& sourceData) {
    auto data = sourceData->data();
    auto dataCount = sourceData->count();

    auto skData = SkData::MakeWithoutCopy(data, dataCount);
    auto image = SkImages::DeferredFromEncodedData(skData);

    new (this) CGImage(image, sourceData);
}

//CGImage::CGImage(SDL_Surface* surface) {
//    auto pointer = GPU_CopyImageFromSurface(surface);
//    if (!pointer) { return; }
//
//    new (this) CGImage(pointer, std::nullopt);
//}

CGImage::~CGImage() {
    sourceData = nullptr;
    pointee = nullptr;
//    GPU_FreeTarget(pointee->target);
//    GPU_FreeImage(pointee);
}

NXSize CGImage::size() const {
    if (!pointee) return {};
    return {(NXFloat) pointee->width(), (NXFloat) pointee->height()};
}
