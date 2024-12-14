#include "UIImage.h"
#include <tools/SharedBase.hpp>

using namespace NXKit;

UIImage::UIImage(std::shared_ptr<CGImage> cgImage, NXFloat scale):
    _cgImage(cgImage), _scale(scale), _size(cgImage->size())
{ }

std::shared_ptr<UIImage> UIImage::fromPath(std::string path) {
    auto imageData = NXData::fromPath(path);
    if (!imageData) { return nullptr; }
    return fromData(imageData);
}

std::shared_ptr<UIImage> UIImage::fromData(std::shared_ptr<NXData> data, NXFloat scale) {
    auto image = new_shared<CGImage>(data);
    return new_shared<UIImage>(image, scale);
}
