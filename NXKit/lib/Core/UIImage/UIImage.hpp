//
//  UIImage.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#include <Core/Geometry/Geometry.hpp>
#include <string>
#include <nanovg.h>
#include <lunasvg/lunasvg.h>

namespace NXKit {

enum class UIImageType {
    PNG,
    SVG,
};

class UIImage {
public:
    UIImage(std::string path, bool isTemplate = false, float upscale = 1);
    ~UIImage();

    Size getSize() { return size; }
    UIImageType getImageType() { return imageType; }

    bool isDrawingTypeTemplate() { return isTemplate; }

    NVGpaint getPaint();
private:
    bool isTemplate = false;
    UIImageType imageType;
    int pngTexture = 0;
    std::unique_ptr<lunasvg::Document> svgFile;
    float upscale = 1;
    Size size;
};

}
