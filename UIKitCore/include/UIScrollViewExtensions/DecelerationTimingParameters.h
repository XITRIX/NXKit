//
// Created by Даниил Виноградов on 24.01.2025.
//

#pragma once

#include <Geometry.h>
#include <Timer.h>

namespace NXKit {
struct DecelerationTimingParameters {
public:
    NXPoint initialValue;
    NXPoint initialVelocity;
    NXFloat decelerationRate;
    NXFloat threshold;

    DecelerationTimingParameters(NXPoint initialValue, NXPoint initialVelocity, NXFloat decelerationRate, NXFloat threshold):
            initialValue(initialValue),
            initialVelocity(initialVelocity),
            decelerationRate(decelerationRate),
            threshold(threshold) { }

    [[nodiscard]] NXPoint destination() const;
    [[nodiscard]] NXFloat duration() const;
    [[nodiscard]] NXPoint valueAt(float time) const;


    [[nodiscard]] NXFloat durationTo(NXPoint value) const;
    [[nodiscard]] NXPoint velocityAt(NXFloat time) const;
};
}