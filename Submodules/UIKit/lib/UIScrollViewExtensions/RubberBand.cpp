//
// Created by Даниил Виноградов on 12.01.2024.
//

#include <UIScrollViewExtensions/RubberBand.h>

namespace NXKit {

NXPoint RubberBand::clamp(NXPoint point) const{
    auto x = Geometry::rubberBandClamp(point.x, coeff, dims.width, bounds.minX(), bounds.maxX());
    auto y = Geometry::rubberBandClamp(point.y, coeff, dims.height, bounds.minY(), bounds.maxY());
    return {x, y};
}

}