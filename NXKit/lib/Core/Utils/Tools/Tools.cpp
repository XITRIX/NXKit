//
//  Tools.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 29.08.2022.
//

#include <Core/Utils/Tools/Tools.hpp>

namespace NXKit {

float ntz(float value) {
    return isnan(value) ? 0.0f : value;
}

bool floatSame(float a, float b, float epsilon) {
    return abs(a - b) <= epsilon;
}

float minmax(float min, float val, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

}
