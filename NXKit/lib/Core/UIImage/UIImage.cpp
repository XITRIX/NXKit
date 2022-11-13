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

std::map<std::string, std::pair<int, int>> UIImage::pngTextureStash;

UIImage::UIImage(std::string path, bool isTemplate, float upscale):
    path(path), upscale(upscale), isTemplate(isTemplate)
{
    auto context = Application::shared()->getVideoContext()->getNVGContext();

    if (pngTextureStash.count(path)) {
        pngTexture = pngTextureStash[path].first;

        int width, height;
        nvgImageSize(context, this->pngTexture, &width, &height);
        size = Size(width, height);

        pngTextureStash[path].second++;
        return;
    }

    pngTexture = nvgCreateImage(context, path.c_str(), 0);
    if (pngTexture != 0) {
        int width, height;
        nvgImageSize(context, this->pngTexture, &width, &height);
        size = Size(width, height);
//        imageType = UIImageType::PNG;
        pngTextureStash[path] = std::pair<int, int>(pngTexture, 1);
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
//        imageType = UIImageType::SVG;
        pngTextureStash[path] = std::pair<int, int>(pngTexture, 1);
        return;
    }
}

UIImage::~UIImage() {
    auto context = Application::shared()->getVideoContext()->getNVGContext();
    if (pngTexture != 0) {
        pngTextureStash[path].second--;
        if (pngTextureStash[path].second == 0) {
            nvgDeleteImage(context, pngTexture);
            pngTextureStash.erase(path);
        }
    }
}

NVGpaint UIImage::getPaint() {
    auto context = Application::shared()->getContext();
    return nvgImagePattern(context, 0, 0, size.width, size.height, 0, this->pngTexture, 1.0f);
}

}
