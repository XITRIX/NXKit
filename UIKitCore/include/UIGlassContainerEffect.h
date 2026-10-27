#pragma once
#include <Geometry.h>

namespace NXKit {
/** Copied glass grouping configuration. UIKit's default spacing is zero. */
class UIGlassContainerEffect {
public:
    [[nodiscard]] NXFloat spacing() const { return _spacing; }
    /** Distance controlling the onset of merging, in container-local points.
     Must be finite and nonnegative. Reinstall the effect after changing it. */
    void setSpacing(NXFloat value);
private:
    NXFloat _spacing = 0;
};
}
