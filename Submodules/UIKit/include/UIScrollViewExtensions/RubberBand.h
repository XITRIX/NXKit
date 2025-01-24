//
// Created by Даниил Виноградов on 12.01.2024.
//

#pragma once

#include <Geometry.h>

namespace NXKit {

struct RubberBand {
    float coeff;
    NXSize dims;
    NXRect bounds;

    RubberBand(float coeff, NXSize dims, NXRect bounds):
            coeff(coeff), dims(dims), bounds(bounds) {}

    NXPoint clamp(NXPoint point) const;
};

}