#pragma once

#include <CGImage.h>
#include <NXData.h>
#include <SkiaCtx.h>

namespace NXKit {

class UIImage {
public:
    UIImage(std::shared_ptr<CGImage> cgImage, NXFloat scale);

    static std::shared_ptr<UIImage> fromPath(std::string path);
    static std::shared_ptr<UIImage> fromData(std::shared_ptr<NXData> data, NXFloat scale = SkiaCtx::main()->getScaleFactor());

    std::shared_ptr<CGImage> cgImage() { return _cgImage; }
    NXSize size() const { return _size; }
    NXFloat scale() const { return _scale; }

private:
    std::shared_ptr<CGImage> _cgImage;
    NXSize _size;
    NXFloat _scale;
};
}
