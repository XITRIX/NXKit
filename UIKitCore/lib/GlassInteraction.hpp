#pragma once

// Motion adapted from liquid_glass_widgets' stretch.dart and glass_spring.dart.
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
    NXPoint dragTarget;
    NXPoint location;
    double glowProgress = 0;

    void move(NXPoint point, NXPoint delta) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)
            || !std::isfinite(delta.x) || !std::isfinite(delta.y)) return;
        location = point;
        const double magnitude = std::hypot(delta.x, delta.y);
        const double resistance = 0.5 / (1 + magnitude * 0.01);
        dragTarget = NXPoint(delta.x * resistance, delta.y * resistance);
    }

    void advance(double dt) {
        if (!std::isfinite(dt) || dt <= 0) return;
        inflation.advance(pressed ? 1 : 0, dt, pressed ? 0.250 : 0.280, pressed ? 0.25 : 0.45);
        dragX.advance(pressed ? dragTarget.x : 0, dt, pressed ? 0.150 : 0.500, pressed ? 0 : 0.3);
        dragY.advance(pressed ? dragTarget.y : 0, dt, pressed ? 0.150 : 0.500, pressed ? 0 : 0.3);
        glowProgress = std::clamp(glowProgress + (pressed ? dt / 0.150 : -dt / 0.060), 0.0, 1.0);
    }

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
            && glowProgress == (pressed ? 1 : 0);
    }

    NXAffineTransform transform(NXSize size) const {
        if (!std::isfinite(size.width) || !std::isfinite(size.height)
            || size.width <= 0 || size.height <= 0) return NXAffineTransform::identity;
        const double dimension = std::max(size.width, size.height);
        const double pressScale = 1 + 17 / std::max(17.0, dimension) * inflation.value;
        const double aspect = size.width / size.height;
        const double dampX = aspect > 1 ? 1 / aspect : 1;
        const double dampY = aspect < 1 ? aspect : 1;
        const double relativeX = std::abs(dragX.value) / dimension;
        const double relativeY = std::abs(dragY.value) / dimension;
        const double sx = (1 + std::clamp(relativeX * 0.1 * dampX, 0.0, 0.3))
            * (1 - std::clamp(relativeY * 0.1 * dampX, 0.0, 0.15));
        const double sy = (1 + std::clamp(relativeY * 0.1 * dampY, 0.0, 0.3))
            * (1 - std::clamp(relativeX * 0.1 * dampY, 0.0, 0.15));
        const auto signedSmoothstep = [](double value) {
            const double t = std::clamp(std::abs(value) / 20, 0.0, 1.0);
            return std::copysign(t * t * (3 - 2 * t), value);
        };
        const double px = size.width * (0.5 - signedSmoothstep(dragX.value) * 0.5);
        const double py = size.height * (0.5 - signedSmoothstep(dragY.value) * 0.5);
        // Centre inflation, then elongation anchored on the opposite edge.
        return NXAffineTransform(
            pressScale * sx, 0, 0, pressScale * sy,
            size.width * 0.5 * (1 - pressScale)
                + pressScale * (px * (1 - sx) + sx * dragX.value * 0.1 * dampX),
            size.height * 0.5 * (1 - pressScale)
                + pressScale * (py * (1 - sy) + sy * dragY.value * 0.1 * dampY)
        );
    }
};

}
