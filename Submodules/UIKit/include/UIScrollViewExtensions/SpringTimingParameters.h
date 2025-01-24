//
// Created by Даниил Виноградов on 12.01.2024.
//

#pragma once

#include <Geometry.h>
#include <tools/SharedBase.hpp>

namespace NXKit {
struct TimingParameters {
    [[nodiscard]] virtual NXFloat duration() const { return 0; }
    [[nodiscard]] virtual NXPoint valueAt(NXFloat time) const { return {}; }
};

struct Spring {
    NXFloat mass;
    NXFloat stiffness;
    NXFloat dampingRatio;

    Spring(NXFloat mass, NXFloat stiffness, NXFloat dampingRatio):
            mass(mass), stiffness(stiffness), dampingRatio(dampingRatio) {}

    [[nodiscard]] NXFloat damping() const;
    [[nodiscard]] NXFloat beta() const;
    [[nodiscard]] NXFloat dampedNaturalFrequency() const;
};

struct SpringTimingParameters: TimingParameters {
    Spring spring;
    NXPoint displacement;
    NXPoint initialVelocity;
    NXFloat threshold;

    SpringTimingParameters(Spring spring, NXPoint displacement, NXPoint initialVelocity, NXFloat threshold);

    [[nodiscard]] NXFloat duration() const override;
    [[nodiscard]] NXPoint valueAt(NXFloat time) const override;

private:
    std::shared_ptr<TimingParameters> impl;
};

}
