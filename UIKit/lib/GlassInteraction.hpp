#pragma once

// Analytic oscillator originally adapted from liquid_glass_widgets' glass_spring.dart.
// Size policy, centered stretch and pull curves calibrated from UIKit traces;
// see docs/liquid-glass-interaction for runtime evidence and remaining limits.
// Copyright 2025 Tim Lehmann for whynotmake.it
// Copyright (c) 2024–2026 Sebastian Degenaar
// SPDX-License-Identifier: MIT. See licenses/liquid_glass_widgets-MIT.txt.

#include <Geometry.h>
#include <NXAffineTransform.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace NXKit::Internal {

struct GlassSpring {
    double value = 0;
    double velocity = 0;

    // Exact damped oscillator, preserving velocity on retarget. Independent of
    // refresh rate and stable after a stalled frame (unlike Euler integration).
    void advance(double target, double dt, double duration, double bounce) {
        const double omega = 2 * std::numbers::pi / duration;
        const double zeta = 1 - bounce;
        const double displacement = value - target;
        const double decay = std::exp(-zeta * omega * dt);
        if (bounce == 0) {
            const double coefficient = velocity + omega * displacement;
            value = target + (displacement + coefficient * dt) * decay;
            velocity = (velocity - omega * coefficient * dt) * decay;
        } else {
            const double frequency = omega * std::sqrt(1 - zeta * zeta);
            const double coefficient = (velocity + zeta * omega * displacement) / frequency;
            const double cosine = std::cos(frequency * dt);
            const double sine = std::sin(frequency * dt);
            const double wave = displacement * cosine + coefficient * sine;
            value = target + decay * wave;
            velocity = decay * (-zeta * omega * wave
                + frequency * (coefficient * cosine - displacement * sine));
        }
        if (std::abs(value - target) < 0.0001 && std::abs(velocity) < 0.001) {
            value = target;
            velocity = 0;
        }
    }

    bool atRest(double target) const { return value == target && velocity == 0; }
};

struct GlassInteraction {
    bool pressed = false;
    GlassSpring inflation;
    GlassSpring dragX;
    GlassSpring dragY;
    GlassSpring stretchX;
    GlassSpring stretchY;
    NXPoint dragTarget;
    NXPoint displacement;
    NXPoint location;
    NXSize interactionSize = {52, 52};
    double glowProgress = 0;

    void move(NXPoint point, NXPoint delta) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)
            || !std::isfinite(delta.x) || !std::isfinite(delta.y)) return;
        location = point;
        displacement = delta;
        updateDragTarget();
    }

    static bool validSize(NXSize size) {
        return std::isfinite(size.width) && std::isfinite(size.height)
            && size.width > 0 && size.height > 0;
    }

    // Measured on UIKit 27.0 using public XCTest gestures and live presentation
    // layers. These are fitted behavior, not a documented Apple size contract.
    static double sizeWeight(double dimension) {
        return std::clamp((dimension - 44) / 116, 0.0, 1.0);
    }

    static bool usesLargeResponse(NXSize size) {
        return std::max(size.width, size.height) >= 120;
    }

    void updateDragTarget() {
        if (!validSize(interactionSize)) { dragTarget = {}; return; }
        const double resistance = usesLargeResponse(interactionSize)
            ? 3 + 9 * sizeWeight(std::min(interactionSize.width, interactionSize.height)) : 1;
        const auto pull = [resistance](double delta, double dimension) {
            // Resistance changes at the edge, independently on each axis.
            // This also keeps diagonal input from reducing either axis's pull.
            const double distance = std::abs(delta);
            return std::copysign((distance + 3 * std::max(0.0, distance - dimension * .5))
                * dimension / (8000 * resistance), delta);
        };
        dragTarget = NXPoint(pull(displacement.x, interactionSize.width),
                             pull(displacement.y, interactionSize.height));
    }

    void advance(double dt, NXSize size) {
        if (!std::isfinite(dt) || dt <= 0) return;
        interactionSize = size;
        updateDragTarget();
        const double weight = validSize(size) && usesLargeResponse(size)
            ? sizeWeight(std::max(size.width, size.height)) : 0;
        const double pressDuration = std::lerp(2 * std::numbers::pi / 24, 2 * std::numbers::pi / 20, weight);
        const double releaseDuration = std::lerp(.4, .358, weight);
        const double releaseBounce = std::lerp(.625, .4, weight);
        inflation.advance(pressed ? 1 : 0, dt, pressed ? pressDuration : releaseDuration,
                          pressed ? .375 : releaseBounce);
        const double dragDuration = pressed
            ? std::lerp(2 * std::numbers::pi / 22, 2 * std::numbers::pi / 18, weight) : releaseDuration;
        dragX.advance(pressed ? dragTarget.x : 0, dt, dragDuration, pressed ? .375 : releaseBounce);
        dragY.advance(pressed ? dragTarget.y : 0, dt, dragDuration, pressed ? .375 : releaseBounce);
        // Deformation follows unsigned pull, independently of translation.
        // Reversing the finger must not flip the current shape instantly; both
        // presentation values and their velocities survive the new target.
        stretchX.advance(pressed ? std::abs(dragTarget.x) : 0, dt, dragDuration, pressed ? .375 : releaseBounce);
        stretchY.advance(pressed ? std::abs(dragTarget.y) : 0, dt, dragDuration, pressed ? .375 : releaseBounce);
        glowProgress = std::clamp(glowProgress + (pressed ? dt / 0.150 : -dt / 0.060), 0.0, 1.0);
    }

    void advance(double dt) { advance(dt, interactionSize); }

    NXFloat glow() const {
        // Flutter Curves.easeOut: cubic Bezier (0, 0, 0.58, 1).
        // Invert x(t), then evaluate y(t), instead of approximating the curve
        // with a frame-rate-dependent easing step.
        if (glowProgress == 0 || glowProgress == 1) return glowProgress;
        double low = 0, high = 1;
        for (int i = 0; i < 16; ++i) {
            const double t = (low + high) * 0.5;
            const double x = 3 * (1 - t) * t * t * 0.58 + t * t * t;
            if (x < glowProgress) low = t;
            else high = t;
        }
        const double t = (low + high) * 0.5;
        return t * t * (3 - 2 * t);
    }

    bool atRest() const {
        return inflation.atRest(pressed ? 1 : 0)
            && dragX.atRest(pressed ? dragTarget.x : 0)
            && dragY.atRest(pressed ? dragTarget.y : 0)
            && stretchX.atRest(pressed ? std::abs(dragTarget.x) : 0)
            && stretchY.atRest(pressed ? std::abs(dragTarget.y) : 0)
            && glowProgress == (pressed ? 1 : 0);
    }

    NXAffineTransform transform(NXSize size) const {
        if (!validSize(size)) return NXAffineTransform::identity;
        const double dimension = std::max(size.width, size.height);
        const double growth = usesLargeResponse(size)
            ? std::lerp(16.0, 4.0, sizeWeight(std::min(size.width, size.height))) : 16;
        const double pressScale = 1 + growth / std::max(16.0, dimension) * inflation.value;
        // Equal/opposite relative stretch preserves the mean inflation. UIKit
        // centres it and applies the resisted translation before scaling.
        const double stretch = std::clamp(stretchX.value / size.width - stretchY.value / size.height, -.45, .45);
        const double sx = pressScale + stretch;
        const double sy = pressScale - stretch;
        return NXAffineTransform(
            sx, 0, 0, sy,
            size.width * .5 * (1 - sx) + sx * dragX.value,
            size.height * .5 * (1 - sy) + sy * dragY.value
        );
    }
};

}
