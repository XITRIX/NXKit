#pragma once

#include <optional>
#include <memory>
#include <NXData.h>
#include <Geometry.h>

#include "include/core/SkImage.h"

namespace NXKit {

class CGImage {
public:
    sk_sp<SkImage> pointee;

//    CGImage(NXSize size);
    CGImage(std::shared_ptr<NXData> sourceData);
//    CGImage(SDL_Surface* surface);
    CGImage(sk_sp<SkImage> image, std::shared_ptr<NXData> sourceData);
    CGImage(sk_sp<SkImage> image): CGImage(image, nullptr) {}
    ~CGImage();

    [[nodiscard]] NXSize size() const;
private:
    std::shared_ptr<NXData> sourceData;
};
}
