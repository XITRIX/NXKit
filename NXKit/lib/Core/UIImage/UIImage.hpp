//
//  UIImage.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#include <Core/Geometry/Geometry.hpp>
#include <Core/Utils/SharedBase/SharedBase.hpp>
#include <string>
#include <nanovg.h>
#include <lunasvg/lunasvg.h>

namespace NXKit {

enum class UIImageType {
    PNG,
    SVG,
};

class UIImage: public enable_shared_from_base<UIImage> {
public:
    UIImage(std::string path, bool isTemplate = false, float upscale = 1);
    virtual ~UIImage();

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
