#pragma once

#include <Geometry.h>

namespace NXKit {

enum class CALayerContentsGravity {
    bottom, bottomLeft, bottomRight,
    center, left, right,
    top, topLeft, topRight,
    resize, resizeAspect, resizeAspectFill
};

class CALayer;
struct ContentsGravityTransformation {
    NXPoint offset;
    NXSize scale;

    ContentsGravityTransformation(CALayer* layer);
    
private:
    NXSize defaultScale = NXSize(1.0, 1.0);
};

}

