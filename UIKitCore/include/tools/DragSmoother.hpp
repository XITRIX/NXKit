#pragma once

#include <Geometry.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace NXKit {

// Portable presentation utility, not a UIKit gesture-recognition API. Feed
// measured targets/velocities and advance once per display frame. It never
// predicts beyond the last measurement or alters the gesture's release speed.
class DragSmoother {
public:
    void reset(NXPoint position) {
        validate(position);
        _target = position;
        _x = position.x; _y = position.y;
    }
    void setTarget(NXPoint target, NXPoint velocity = {}) {
        validate(target);
        validate(velocity);
        _target = target;
        // More smoothing for slow quantized movement, less lag for fast drags.
        const auto speed = std::hypot(double(velocity.x), double(velocity.y));
        _timeConstant = std::clamp(.032 / (1 + speed / 200), .008, .032);
    }
    NXPoint advance(double seconds) {
        if (!std::isfinite(seconds) || seconds < 0)
            throw std::invalid_argument("Drag frame duration must be finite and nonnegative");
        if (seconds == 0) return position();
        const auto alpha = -std::expm1(-seconds / _timeConstant);
        _x = advanceAxis(_x, _target.x, alpha);
        _y = advanceAxis(_y, _target.y, alpha);
        if (isSettled()) { _x = _target.x; _y = _target.y; }
        return position();
    }
    [[nodiscard]] NXPoint position() const { return {NXFloat(_x), NXFloat(_y)}; }
    [[nodiscard]] NXPoint target() const { return _target; }
    [[nodiscard]] bool isSettled() const { return position() == _target; }
private:
    NXPoint _target;
    double _x = 0, _y = 0;
    double _timeConstant = .032;
    static double advanceAxis(double current, double target, double alpha) {
        const double delta = target - current;
        // Long content offsets must converge even when one float ULP exceeds
        // the usual subpixel tolerance. Calculate in double to avoid overflow.
        const auto tolerance = std::max(.01, std::abs(target) * std::numeric_limits<double>::epsilon());
        if (std::abs(delta) <= tolerance) return target;
        return current + delta * alpha;
    }
    static void validate(NXPoint point) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y))
            throw std::invalid_argument("Drag position and velocity must be finite");
    }
};

} // namespace NXKit
