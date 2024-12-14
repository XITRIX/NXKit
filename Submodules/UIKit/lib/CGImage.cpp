#include "CGImage.h"
#include "include/core/SkData.h"

using namespace NXKit;

//#include <UIApplication/UIApplication.hpp>
//#include <utility>

//CGImage::CGImage(NXSize size) {
//    pointee = SkImages::
////    GPU_SetAnchor(pointee, 0, 0);
////    GPU_SetBlendMode(pointee, GPU_BLEND_NORMAL_FACTOR_ALPHA);
//}

CGImage::CGImage(sk_sp<SkImage> image, std::optional<NXData> sourceData) {
    this->sourceData = std::move(sourceData);
    pointee = image;

//    GPU_SetSnapMode(pointee, GPU_SNAP_POSITION_AND_DIMENSIONS);
//    GPU_SetBlendMode(pointee, GPU_BLEND_NORMAL_FACTOR_ALPHA);
//    GPU_SetImageFilter(pointee, GPU_FILTER_LINEAR);
}

CGImage::CGImage(const NXData& sourceData) {
    auto data = sourceData;
    auto dataCount = data.count();

    auto skData = SkData::MakeFromMalloc(data.data(), data.count());
    auto image = SkImages::DeferredFromEncodedData(skData);

    new (this) CGImage(image, data);
}

//CGImage::CGImage(SDL_Surface* surface) {
//    auto pointer = GPU_CopyImageFromSurface(surface);
//    if (!pointer) { return; }
//
//    new (this) CGImage(pointer, std::nullopt);
//}

CGImage::~CGImage() {
//    GPU_FreeTarget(pointee->target);
//    GPU_FreeImage(pointee);
}

NXSize CGImage::size() const {
    return NXSize(pointee->width(), pointee->height());
}
