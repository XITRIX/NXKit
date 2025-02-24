#pragma once

namespace NXKit {

enum class UIViewContentMode {
    scaleToFill = 0,

    scaleAspectFit = 1, // contents scaled to fit with fixed aspect. remainder is transparent

    scaleAspectFill = 2, // contents scaled to fill with fixed aspect. some portion of content may be clipped.

    redraw = 3, // redraw on bounds change (calls -setNeedsDisplay)

    center = 4, // contents remain same size. positioned adjusted.

    top = 5,

    bottom = 6,

    left = 7,

    right = 8,

    topLeft = 9,

    topRight = 10,

    bottomLeft = 11,

    bottomRight = 12,
};

}
