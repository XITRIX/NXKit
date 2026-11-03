#pragma once

#include <UIView.h>
#include <CADisplayLink.h>
#include <tools/DragSmoother.hpp>

namespace NXKit {

// Demo binding: recognition remains immediate; only the dragged object's
// displayed position follows the measured target on frame boundaries.
class DragPresentation : public std::enable_shared_from_this<DragPresentation> {
public:
    explicit DragPresentation(const std::shared_ptr<UIView>& view,
                              std::function<void(NXPoint)> apply = {})
        : _view(view), _apply(std::move(apply)) {}
    void begin() {
        stop();
        if (const auto view = _view.lock()) _origin = view->frame().origin;
    }
    [[nodiscard]] NXPoint origin() const { return _origin; }
    void stop() { _displayLink.reset(); }
    void update(NXPoint point, NXPoint velocity) {
#ifdef __SWITCH__
        const auto view = _view.lock();
        if (!view) return;
        if (!_displayLink) {
            _smoother.reset(view->frame().origin);
            _time = Timer();
            _displayLink = std::make_unique<CADisplayLink>([weak = weak_from_this()] {
                if (const auto self = weak.lock()) self->tick();
            });
        }
        _smoother.setTarget(point, velocity);
        CALayer::requestFramerate(60);
#else
        apply(point);
#endif
    }
private:
    std::weak_ptr<UIView> _view;
    std::function<void(NXPoint)> _apply;
    std::unique_ptr<CADisplayLink> _displayLink;
    DragSmoother _smoother;
    Timer _time;
    NXPoint _origin;
    void apply(NXPoint point) {
        if (_apply) _apply(point);
        else if (const auto view = _view.lock()) {
            auto frame = view->frame(); frame.origin = point; view->setFrame(frame);
        }
    }
    void tick() {
        const auto view = _view.lock();
        if (!view || !view->window() || view->isHidden()) { stop(); return; }
        const Timer now;
        const auto seconds = std::max(0.0, (now - _time) / 1000);
        _time = now;
        CALayer::requestFramerate(60);
        apply(_smoother.advance(seconds));
        if (_smoother.isSettled()) stop();
    }
};

} // namespace NXKit
