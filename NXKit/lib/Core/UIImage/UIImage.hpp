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
    UIImage(std::string path, float upscale = 1);
    ~UIImage();

    Size getSize() { return size; }
    UIImageType getImageType() { return imageType; }

    NVGpaint getPaint();
private:
    UIImageType imageType;
    int pngTexture = 0;
    std::unique_ptr<lunasvg::Document> svgFile;
    float upscale = 1;
    Size size;
};

}
