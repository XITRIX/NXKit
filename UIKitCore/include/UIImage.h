#pragma once

#include <CGImage.h>
#include <NXData.h>
#include <SkiaCtx.h>
#include <string>

namespace NXKit {

class UIImage {
public:
    UIImage(const std::shared_ptr<CGImage>& cgImage, NXFloat scale);

    static std::shared_ptr<UIImage> fromPath(const std::string& path, NXFloat scale = SkiaCtx::main()->getScaleFactor());
    static std::shared_ptr<UIImage> fromRes(const std::string& path, NXFloat scale = SkiaCtx::main()->getScaleFactor());
    static std::shared_ptr<UIImage> fromData(const std::shared_ptr<NXData>& data, NXFloat scale = SkiaCtx::main()->getScaleFactor());

    /** Monochrome SF Symbol on Apple platforms, ready for template tinting.
     Point size and CSS-style weight configure the native symbol; scale controls
     raster density. Unknown names and platforms without SF Symbols return null. */
    static std::shared_ptr<UIImage> systemImageNamed(const std::string& name,
        NXFloat pointSize = 17, NXFloat weight = 400, NXFloat scale = 1);

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
