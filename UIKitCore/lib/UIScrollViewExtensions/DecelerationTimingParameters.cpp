//
// Created by Daniil Vinogradov on 12/01/2024.
//

#include <UIScrollViewExtensions/DecelerationTimingParameters.h>
#include <cmath>

namespace NXKit {

NXPoint DecelerationTimingParameters::destination() const {
    auto dCoeff = 1000 * log(decelerationRate);
    auto res = initialValue - initialVelocity / dCoeff;
    return res;
}

float DecelerationTimingParameters::duration() const {
    if (initialVelocity.magnitude() <= 0) { return 0; }

    auto dCoeff = 1000 * log(decelerationRate);
    auto res = log(-dCoeff * threshold / initialVelocity.magnitude()) / dCoeff;
    return res;
}

NXPoint DecelerationTimingParameters::valueAt(NXFloat time) const {
    auto dCoeff = 1000 * log(decelerationRate);
    auto res = initialValue + initialVelocity * (pow(decelerationRate, float(1000 * time)) - 1) / dCoeff;
    return res;
}

float DecelerationTimingParameters::durationTo(NXPoint value) const {
    if (value.distanceToSegment(initialValue, destination()) >= threshold) { return 0; }

    auto dCoeff = 1000 * log(decelerationRate);
    auto res = log(1.0f + dCoeff * (value - initialValue).magnitude() / initialVelocity.magnitude()) / dCoeff;
    return res;
}

NXPoint DecelerationTimingParameters::velocityAt(NXFloat time) const {
    return initialVelocity * pow(decelerationRate, NXFloat(1000 * time));
}
}