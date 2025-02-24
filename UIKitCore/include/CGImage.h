#pragma once

#include <optional>
#include <memory>
#include <utility>
#include <NXData.h>
#include <Geometry.h>

#include "include/core/SkImage.h"

namespace NXKit {

class CGImage {
public:
    sk_sp<SkImage> pointee;

//    CGImage(NXSize size);
    explicit CGImage(const std::shared_ptr<NXData>& sourceData);
//    CGImage(SDL_Surface* surface);
    explicit CGImage(sk_sp<SkImage> image, std::shared_ptr<NXData> sourceData);
    explicit CGImage(sk_sp<SkImage> image): CGImage(std::move(image), nullptr) {}
    ~CGImage();

    [[nodiscard]] NXSize size() const;
private:
    std::shared_ptr<NXData> sourceData;
};
}
