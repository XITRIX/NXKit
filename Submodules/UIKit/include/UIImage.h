#pragma once

#include <CGImage.h>
#include <NXData.h>
#include <SkiaCtx.h>

namespace NXKit {

class UIImage {
public:
    UIImage(const std::shared_ptr<CGImage>& cgImage, NXFloat scale);

    static std::shared_ptr<UIImage> fromPath(const std::string& path);
    static std::shared_ptr<UIImage> fromData(const std::shared_ptr<NXData>& data, NXFloat scale = SkiaCtx::main()->getScaleFactor());

    [[nodiscard]] std::shared_ptr<CGImage> cgImage() const { return _cgImage; }
    [[nodiscard]] NXSize size() const { return _size / _scale; }
    [[nodiscard]] NXFloat scale() const { return _scale; }

    void setRenderModeAsTemplate(bool isTemplate) { _isTemplate = isTemplate; }

private:
    friend class UIImageView;

    bool _isTemplate = false;
    std::shared_ptr<CGImage> _cgImage;
    NXSize _size;
    NXFloat _scale;
};
}
