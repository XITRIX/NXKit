#pragma once
#include <Geometry.h>
#include <cmath>
#include <optional>
#include <stdexcept>
namespace NXKit {
class UIView;
/// Value-type C++ counterpart of UIKit's iOS 26 UICornerRadius.
class UICornerRadius {
public:
    static UICornerRadius fixed(NXFloat radius) { return {false, radius}; }
    static UICornerRadius containerConcentric(std::optional<NXFloat> minimum = std::nullopt) {
        return {true, minimum.value_or(0)};
    }
    static UICornerRadius fixedRadius(NXFloat radius) { return fixed(radius); }
    static UICornerRadius containerConcentricRadius() { return containerConcentric(); }
    static UICornerRadius containerConcentricRadiusWithMinimum(NXFloat minimum) { return containerConcentric(minimum); }
    bool operator==(const UICornerRadius&) const = default;
private:
    friend class UIView;
    UICornerRadius(bool concentric, NXFloat value): _concentric(concentric), _value(value) {
        if (!std::isfinite(value) || value < 0) throw std::invalid_argument("Corner radius must be finite and nonnegative");
    }
    bool _concentric;
    NXFloat _value;
};
}
