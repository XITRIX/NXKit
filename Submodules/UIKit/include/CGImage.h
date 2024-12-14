#pragma once

#include <SDL.h>
//#include <SDL_gpu.h>
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
    CGImage(const NXData& sourceData);
//    CGImage(SDL_Surface* surface);
    CGImage(sk_sp<SkImage> image, std::optional<NXData> sourceData);
    CGImage(sk_sp<SkImage> image): CGImage(image, std::nullopt) {}
    ~CGImage();

    [[nodiscard]] NXSize size() const;
private:
    std::optional<NXData> sourceData;
};
}
