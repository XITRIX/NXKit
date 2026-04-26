//
// Created by Daniil Vinogradov on 12/01/2024.
//

#include <UIScrollViewExtensions/DecelerationTimingParameters.h>
#include <algorithm>
#include <cmath>

namespace NXKit {

namespace {

NXFloat decelerationCoefficient(NXFloat decelerationRate) {
    auto clampedRate = std::clamp(decelerationRate, 0.001f, 0.999999f);
    return 1000.0f * std::log(clampedRate);
}

NXFloat projectedDistanceAlongTrajectory(NXPoint from, NXPoint to, NXPoint value) {
    auto trajectory = to - from;
    auto trajectoryLength = trajectory.magnitude();
    if (trajectoryLength <= 0) return 0;

    auto normalizedTrajectory = trajectory / trajectoryLength;
    auto offset = value - from;
    auto projection = offset.x * normalizedTrajectory.x + offset.y * normalizedTrajectory.y;
    return std::clamp(projection, 0.0f, trajectoryLength);
}

}

NXPoint DecelerationTimingParameters::destination() const {
    auto dCoeff = decelerationCoefficient(decelerationRate);
    if (!std::isfinite(dCoeff) || dCoeff >= 0) return initialValue;
    return initialValue - initialVelocity / dCoeff;
}

float DecelerationTimingParameters::duration() const {
    auto velocityMagnitude = initialVelocity.magnitude();
    if (velocityMagnitude <= 0 || threshold <= 0) return 0;

    auto dCoeff = decelerationCoefficient(decelerationRate);
    if (!std::isfinite(dCoeff) || dCoeff >= 0) return 0;

    auto durationArgument = -dCoeff * threshold / velocityMagnitude;
    if (durationArgument <= 0 || !std::isfinite(durationArgument)) return 0;

    auto result = std::log(durationArgument) / dCoeff;
    if (!std::isfinite(result)) return 0;
    return std::max(0.0f, result);
}

NXPoint DecelerationTimingParameters::valueAt(NXFloat time) const {
    auto dCoeff = decelerationCoefficient(decelerationRate);
    if (!std::isfinite(dCoeff) || dCoeff >= 0) return initialValue;

    auto clampedTime = std::max(0.0f, time);
    auto exponentialTerm = std::pow(decelerationRate, float(1000 * clampedTime));
    if (!std::isfinite(exponentialTerm)) return initialValue;

    return initialValue + initialVelocity * (exponentialTerm - 1) / dCoeff;
}

float DecelerationTimingParameters::durationTo(NXPoint value) const {
    auto velocityMagnitude = initialVelocity.magnitude();
    if (velocityMagnitude <= 0 || threshold <= 0) return 0;
    if (value.distanceToSegment(initialValue, destination()) >= threshold) { return 0; }

    auto dCoeff = decelerationCoefficient(decelerationRate);
    if (!std::isfinite(dCoeff) || dCoeff >= 0) return 0;

    auto distanceToValue = projectedDistanceAlongTrajectory(initialValue, destination(), value);
    auto durationArgument = 1.0f + dCoeff * distanceToValue / velocityMagnitude;
    if (durationArgument <= 0 || !std::isfinite(durationArgument)) return 0;

    auto result = std::log(durationArgument) / dCoeff;
    if (!std::isfinite(result)) return 0;
    return std::max(0.0f, result);
}

NXPoint DecelerationTimingParameters::velocityAt(NXFloat time) const {
    auto clampedTime = std::max(0.0f, time);
    auto exponentialTerm = std::pow(decelerationRate, NXFloat(1000 * clampedTime));
    if (!std::isfinite(exponentialTerm)) return {};
    return initialVelocity * exponentialTerm;
}
}
