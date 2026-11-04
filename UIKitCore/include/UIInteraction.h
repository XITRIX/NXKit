#pragma once

#include <memory>

namespace NXKit {
class UIView;

/** A style-independent interaction attached to one UIView. Implementations keep
    a weak view reference and update it in didMoveToView, as in UIKit. */
class UIInteraction {
public:
    virtual ~UIInteraction() = default;
    [[nodiscard]] virtual std::weak_ptr<UIView> view() const = 0;
    virtual void willMoveToView(const std::shared_ptr<UIView>& view) = 0;
    virtual void didMoveToView(const std::shared_ptr<UIView>& view) = 0;

private:
    friend class UIView;
    std::weak_ptr<UIView> _attachedView;
};
}
