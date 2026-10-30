#include <UIImage.h>
#include <tools/SharedBase.hpp>
#include <cmath>
#include <stdexcept>

#if defined(__APPLE__)
namespace NXKit::Internal {
std::shared_ptr<UIImage> appleSystemImage(const std::string&,NXFloat,NXFloat,NXFloat);
}
#endif

using namespace NXKit;

UIImage::UIImage(const std::shared_ptr<CGImage>& cgImage, NXFloat scale):
    _cgImage(cgImage), _scale(scale), _size(cgImage->size())
{ }

std::shared_ptr<UIImage> UIImage::fromPath(const std::string& path, NXFloat scale) {
    auto imageData = NXData::fromPath(path);
    if (!imageData) { return nullptr; }
    return fromData(imageData, scale);
}

std::shared_ptr<UIImage> UIImage::fromRes(const std::string& path, NXFloat scale) {
    auto imageData = NXData::fromRes(path);
    if (!imageData) { return nullptr; }
    return fromData(imageData, scale);
}

std::shared_ptr<UIImage> UIImage::fromData(const std::shared_ptr<NXData>& data, NXFloat scale) {
    auto image = new_shared<CGImage>(data);
    return new_shared<UIImage>(image, scale);
}

std::shared_ptr<UIImage> UIImage::systemImageNamed(const std::string& name,NXFloat pointSize,NXFloat weight,NXFloat scale) {
    if(!std::isfinite(pointSize)||pointSize<=0||!std::isfinite(weight)||!std::isfinite(scale)||scale<=0)
        throw std::invalid_argument("Symbol size, weight and scale must be finite; size and scale must be positive");
    if(name.empty()) return nullptr;
#if defined(__APPLE__)
    return Internal::appleSystemImage(name,pointSize,weight,scale);
#else
    return nullptr;
#endif
}
