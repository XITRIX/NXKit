#pragma once

namespace NXKit {

enum UIViewAnimationOptions {
    none = 0,
    allowUserInteraction = 1 << 0,
    beginFromCurrentState = 1 << 1,
    curveEaseIn = 1 << 2,
    curveEaseOut = 1 << 3,
    curveEaseInOut = 1 << 4,
    curveLinear = 1 << 5,
    curveEaseOutElastic = 1 << 6,
    customEaseOut = 1 << 9
};

}
