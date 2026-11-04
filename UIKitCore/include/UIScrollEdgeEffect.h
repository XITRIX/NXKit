#pragma once
#include <functional>
#include <utility>

namespace NXKit {
class UIScrollView;
// Value-enum translation of UIScrollEdgeEffect.Style's three singleton values.
enum class UIScrollEdgeEffectStyle { automatic, soft, hard };

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
