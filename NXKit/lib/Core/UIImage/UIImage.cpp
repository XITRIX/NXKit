//
//  UIImage.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#include <Core/UIImage/UIImage.hpp>
#include <Core/Application/Application.hpp>

using namespace lunasvg;

namespace NXKit {

UIImage::UIImage(std::string path, bool isTemplate, float upscale):
    upscale(upscale), isTemplate(isTemplate)
{
    auto context = Application::shared()->getVideoContext()->getNVGContext();
    
    pngTexture = nvgCreateImage(context, path.c_str(), 0);
    if (pngTexture != 0) {
        int width, height;
        nvgImageSize(context, this->pngTexture, &width, &height);
        size = Size(width, height);
        imageType = UIImageType::PNG;
        return;
    }

    svgFile = Document::loadFromFile(path);
    if (svgFile) {
        int svgMultiplier = Application::shared()->getVideoContext()->getScaleFactor() * upscale;
        size = Size(svgFile->width() * svgMultiplier, svgFile->height() * svgMultiplier);
        auto bitmap = svgFile->renderToBitmap(size.width, size.height);
        bitmap.convertToRGBA();

        auto bitmapData = bitmap.data();
        if (isTemplate) {
            for (int i = 0; i < bitmap.height() * bitmap.width() * 4; i++) {
                if (i % 4 == 3) continue;
                bitmapData[i] = 255;
            }
        }

        pngTexture = nvgCreateImageRGBA(context, bitmap.width(), bitmap.height(), 0, bitmapData);
        imageType = UIImageType::SVG;
        return;
    }
}

UIImage::~UIImage() {
    auto context = Application::shared()->getVideoContext()->getNVGContext();
    if (pngTexture != 0)
        nvgDeleteImage(context, pngTexture);
}

NVGpaint UIImage::getPaint() {
    auto context = Application::shared()->getContext();
    return nvgImagePattern(context, 0, 0, size.width, size.height, 0, this->pngTexture, 1.0f);
}

}
