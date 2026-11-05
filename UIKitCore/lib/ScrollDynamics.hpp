#pragma once

#include <Geometry.h>
#include <algorithm>
#include <cmath>

namespace NXKit::Internal {

// Analytical exponential flight and elastic edge return. Native
// calibration, replay errors and remaining differences: docs/scroll-physics.
struct ScrollAxisMotion {
    // Separate from the passive pull return: measured free-flight edge impacts
    // peak at hitVelocity / (e * 11), independently at both content boundaries.
    static constexpr double impactFrequency = 11;
    double position = 0, velocity = 0;
    double minimum = 0, maximum = 0, target = 0;
    double omega = 12.1, damping = 24.2, threshold = .1666667, rate = 2.00200267;
    bool bounces = false, spring = false, active = false, fixedTarget = false;

    void begin(double p, double v, double low, double high, bool bounce,
               double k, double tolerance) {
        fixedTarget = false;
        position = p;
        velocity = v;
        minimum = low;
        maximum = high;
        bounces = bounce;
        rate = k;
        threshold = tolerance;
        spring = p < low || p > high;
        active = spring || std::abs(v) >= 10;
        target = std::clamp(p, low, high);
        omega = 12.1;
        damping = 2 * omega;
        if (spring && (p - target) * v > 0) {
            omega = impactFrequency;
            damping = 2 * omega;
        } else if (spring && high > low && (p - target) * v < 0) {
            // Inward release is less damped than a passive edge return. Values
            // are calibrated against the retained top/bottom UIKit reversals.
            omega = 13.2;
            // Deeper stretches with less inward momentum lose more velocity
            // before crossing. Calibrated by the separate deep-pull traces.
            const auto returnTime = omega * std::abs(p - target) / std::abs(v);
            damping = std::min(2 * omega * .999,
                15.5 + 4.25 * std::max(0.0, returnTime - .75));
        }
        if (!bounces && spring) {
            position = std::clamp(position, minimum, maximum);
            velocity = 0;
            active = false;
        }
        if (!active) velocity = 0;
    }

    void advance(double dt) {
        if (!active || dt <= 0) return;
        if (!spring) {
            // UIKit stops free flight at 10 pt/s, before its asymptote. Solve
            // that time too so the final offset is independent of frame rate.
            const auto stopTime = std::max(0.0, std::log(std::abs(velocity) / 10.0) / rate);
            const auto flightTime = std::min(dt, stopTime);
            const auto end = position + velocity * (-std::expm1(-rate * flightTime)) / rate;
            const auto bound = std::clamp(end, minimum, maximum);
            if (end != bound) {
                const auto fraction = 1 - rate * (bound - position) / velocity;
                const auto hit = fraction > 0
                    ? std::clamp(-std::log(fraction) / rate, 0.0, dt) : 0;
                position = bound;
                velocity *= std::exp(-rate * hit);
                dt -= hit;
                if (!bounces) {
                    velocity = 0;
                    active = false;
                    return;
                }
                spring = true;
                target = bound;
                omega = impactFrequency;
                damping = 2 * omega;
            } else {
                position = end;
                velocity *= std::exp(-rate * flightTime);
                if (dt >= stopTime) {
                    velocity = 0;
                    active = false;
                }
                return;
            }
        }
        const auto displacement = position - target;
        if (damping < 2 * omega) {
            const auto decay = damping * .5;
            const auto frequency = std::sqrt(omega * omega - decay * decay);
            const auto sineCoefficient = (velocity + decay * displacement) / frequency;
            auto crossingAngle = std::atan2(-displacement, sineCoefficient);
            if (crossingAngle <= 0) crossingAngle += std::acos(-1.0);
            const auto crossingTime = crossingAngle / frequency;
            if (crossingTime <= dt) {
                const auto crossingVelocity = std::exp(-decay * crossingTime) * frequency
                    * (-displacement * std::sin(crossingAngle)
                       + sineCoefficient * std::cos(crossingAngle));
                resumeFreeFlight(crossingVelocity, dt - crossingTime);
                return;
            }
            const auto angle = frequency * dt;
            const auto envelope = displacement * std::cos(angle) + sineCoefficient * std::sin(angle);
            const auto e = std::exp(-decay * dt);
            position = target + envelope * e;
            velocity = e * (-decay * envelope + frequency
                * (-displacement * std::sin(angle) + sineCoefficient * std::cos(angle)));
            settleSpringIfNeeded();
            return;
        }
        const auto c = velocity + omega * displacement;
        // A reversed flick can cross the edge with inward momentum. At that
        // instant the elastic force ends; keep the crossing velocity and spend
        // the rest of this frame in ordinary deceleration, rather than pulling
        // the content back to the old spring target.
        if (!fixedTarget && displacement * c < 0) {
            const auto crossingTime = -displacement / c;
            if (crossingTime <= dt) {
                resumeFreeFlight(c * std::exp(-omega * crossingTime), dt - crossingTime);
                return;
            }
        }
        const auto e = std::exp(-omega * dt);
        position = target + (displacement + c * dt) * e;
        velocity = (velocity - omega * c * dt) * e;
        settleSpringIfNeeded();
    }

private:
    void resumeFreeFlight(double crossingVelocity, double remainingTime) {
        position = target;
        velocity = crossingVelocity;
        spring = false;
        active = std::abs(velocity) >= 10;
        if (!active) velocity = 0;
        advance(remainingTime);
    }

    void settleSpringIfNeeded() {
        if (std::abs(position - target) <= threshold
            && std::abs(velocity) <= omega * threshold) {
            position = target;
            velocity = 0;
            active = false;
        }
    }
};

struct ScrollMotion {
    ScrollAxisMotion x, y;
    double elapsed = 0, initialFrame = 0;

    void advanceTo(double time) {
        const auto dt = std::max(0.0, time - elapsed);
        elapsed = std::max(elapsed, time);
        x.advance(dt);
        y.advance(dt);
    }
    bool active() const { return x.active || y.active; }
    NXPoint position() const { return {NXFloat(x.position), NXFloat(y.position)}; }
    NXPoint velocity() const { return {NXFloat(x.velocity), NXFloat(y.velocity)}; }
    NXPoint presentationPosition() const {
        auto preview = *this;
        preview.advanceTo(std::max(elapsed, initialFrame));
        return preview.position();
    }
};

} // namespace NXKit::Internal
