#pragma once
#include <functional>
#include <utility>

namespace NXKit {
class UIScrollView;
// Value-enum translation of UIKit's singleton styles. `none` is a portable
// extension: disable the material without changing the independent hidden flag.
// Kits resolve automatic appearance; UIKit uses none at the bottom, soft at
// other attached edges, while NXKit uses its shared hard material.
enum class UIScrollEdgeEffectStyle { automatic, soft, hard, none };

/** Properties of one UIScrollView edge. The owning component kit supplies its
    appearance; Core keeps this metadata independently usable. */
class UIScrollEdgeEffect final {
public:
    [[nodiscard]] UIScrollEdgeEffectStyle style() const { return _style; }
    void setStyle(UIScrollEdgeEffectStyle style);
    [[nodiscard]] bool isHidden() const { return _hidden; }
    void setHidden(bool hidden);
private:
    friend class UIScrollView;
    explicit UIScrollEdgeEffect(std::function<void()> invalidate):_invalidate(std::move(invalidate)){}
    UIScrollEdgeEffectStyle _style=UIScrollEdgeEffectStyle::automatic;
    bool _hidden=false;
    std::function<void()> _invalidate;
};
}
