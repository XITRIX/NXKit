#include <BackdropEffectView.h>

#include "CABackdropEffectLayer.hpp"
#include "GlassAppearance.hpp"
#include "LayerCornerGeometry.hpp"
#include <CADisplayLink.h>
#include <CATransaction.h>
#include <UITouch.h>
#include <UIWindow.h>
#include <tools/IBTools.h>

#include <algorithm>
#include <chrono>
#include <cmath>

using namespace NXKit;

namespace {

// Observe the full touch lifetime without entering the recognizer arena.
// Descendant controls, ancestor scroll views and app pan gestures retain their
// normal delivery and cancellation behavior, even after a long drag.
class GlassTouchObserver final: public UIGestureRecognizer {
public:
    std::function<void(UITouchPhase, const std::shared_ptr<UITouch>&)> observe;
    GlassTouchObserver() { setCancelsTouchesInView(false); }
    bool canPrevent(const std::shared_ptr<UIGestureRecognizer>&) const override { return false; }
    bool canBePreventedBy(const std::shared_ptr<UIGestureRecognizer>&) const override { return false; }
    void touchesBegan(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override {
        for (const auto& touch : touches) observe(UITouchPhase::began, touch);
    }
    void touchesMoved(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override {
        for (const auto& touch : touches) observe(UITouchPhase::moved, touch);
    }
    void touchesEnded(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override {
        for (const auto& touch : touches) observe(UITouchPhase::ended, touch);
    }
    void touchesCancelled(std::vector<std::shared_ptr<UITouch>> touches, std::shared_ptr<UIEvent>) override {
        for (const auto& touch : touches) observe(UITouchPhase::cancelled, touch);
    }
};

}

struct BackdropEffectView::InteractionState {
    Internal::GlassInteraction motion;
    std::shared_ptr<GlassTouchObserver> observer;
    std::shared_ptr<CADisplayLink> displayLink;
    std::weak_ptr<UITouch> touch;
    std::weak_ptr<UIWindow> window;
    std::weak_ptr<UIView> parent;
    NXPoint origin;
    std::chrono::steady_clock::time_point lastFrame;
    bool ticking = false;
};

BackdropEffectView::~BackdropEffectView() = default;

BackdropEffectView::BackdropEffectView():
    BackdropEffectView(UIGlassEffect())
{}

BackdropEffectView::BackdropEffectView(const BackdropEffect& effect):
    UIView(NXRect(), new_shared<CABackdropEffectLayer>(effect))
{ _updateDisplayScale(); }

BackdropEffectView::BackdropEffectView(const UIGlassEffect& effect):
    UIView(
        NXRect(),
        new_shared<CABackdropEffectLayer>(effect._backdropEffect(
            NXSize(),
            UITraitCollection::current()
        ))
    ),
    _glassEffect(effect)
{
    _updateDisplayScale();
    _updateGlassEffect(true);
    _configureAppearance();
    _configureInteraction();
}

void BackdropEffectView::setFrame(NXRect frame) {
    UIView::setFrame(frame);
    _updateAppearance(_backdropLuminance);
    _updateGlassEffect();
}

void BackdropEffectView::setBounds(NXRect bounds) {
    UIView::setBounds(bounds);
    _updateAppearance(_backdropLuminance);
    _updateGlassEffect();
}

void BackdropEffectView::traitCollectionDidChange(
    std::shared_ptr<UITraitCollection> previousTraitCollection
) {
    _updateDisplayScale();
    _configureAppearance(false);
    UIView::traitCollectionDidChange(std::move(previousTraitCollection));
    _updateGlassEffect(true);
}

std::shared_ptr<CABackdropEffectLayer> BackdropEffectView::_backdropEffectLayer() const {
    return std::static_pointer_cast<CABackdropEffectLayer>(layer());
}

void BackdropEffectView::_updateDisplayScale() {
    const auto traits = traitCollection();
    const auto candidate = traits ? traits->displayScale() : 1;
    const auto scale = std::isfinite(candidate) && candidate > 0 ? candidate : 1;
    // Only an environment change updates the managed backing scale. A custom
    // layer override survives ordinary layout/style changes on the same screen.
    if (_resolvedDisplayScale == scale) return;
    _resolvedDisplayScale = scale;
    CATransaction::begin();
    CATransaction::setDisableActions(true);
    layer()->setContentsScale(scale);
    layer()->presentationOrSelf()->setContentsScale(scale);
    CATransaction::commit();
}

void BackdropEffectView::setEffect(const BackdropEffect& effect) {
    _backdropEffectLayer()->setEffect(effect);
    _backdropEffectLayer()->setGlassMaterial(std::nullopt, nullptr);
    _glassEffect.reset();
    _hasResolvedGlass = false;
    _configureAppearance();
    _configureInteraction();
}

void BackdropEffectView::setEffect(const UIGlassEffect& effect) {
    const auto size = bounds().size;
    const auto traits = traitCollection();
    const auto resolved = effect._backdropEffect(size, traits);
    _backdropEffectLayer()->setEffect(resolved);

    _glassEffect = effect;
    _backdropEffectLayer()->setGlassMaterial(_glassEffect, traits);
    _resolvedGlassSize = size;
    _resolvedInterfaceStyle = traits
        ? traits->userInterfaceStyle()
        : UIUserInterfaceStyle::unspecified;
    _hasResolvedGlass = true;
    _configureAppearance();
    _configureInteraction();
}

bool BackdropEffectView::applyXMLAttribute(const std::string& name, const std::string& value) {
    if (name == "isInteractive" || name == "adaptsAppearanceToBackdrop" || name == "contentVibrancyEnabled") {
        const auto interactive = valueToBool(value);
        if (!interactive || !_glassEffect) return false;
        auto glass = *_glassEffect;
        if (name == "isInteractive") glass.setInteractive(*interactive);
        else if(name == "contentVibrancyEnabled") glass.setContentVibrancyEnabled(*interactive);
        else glass.setAdaptsAppearanceToBackdrop(*interactive);
        setEffect(glass);
        return true;
    }
    return UIView::applyXMLAttribute(name, value);
}

std::shared_ptr<UITraitCollection> BackdropEffectView::traitsForSubviews() const {
    return _contentTraits ? _contentTraits : traitCollection();
}

void BackdropEffectView::_configureAppearance(bool propagateTraits) {
    _backdropLuminance.reset();
    _backdropEffectLayer()->setAppearanceObserver({});
    _updateAppearance(std::nullopt, propagateTraits);
    if (!_glassEffect || !_glassEffect->adaptsAppearanceToBackdrop()
        || _glassEffect->style() != UIGlassEffect::Style::regular) return;
    const auto weak = weak_from_base<BackdropEffectView>();
    _backdropEffectLayer()->setAppearanceObserver([weak](std::optional<NXFloat> luminance, NXSize) {
        if (const auto view = weak.lock()) {
            view->_updateAppearance(luminance);
        }
    });
}

void BackdropEffectView::_updateAppearance(std::optional<NXFloat> luminance, bool propagateTraits) {
    _backdropLuminance = luminance;
    const auto inherited = traitCollection();
    const auto inheritedStyle = inherited
        ? inherited->userInterfaceStyle() : UIUserInterfaceStyle::unspecified;
    const auto previousTraits = traitsForSubviews();
    const auto previousStyle = previousTraits
        ? previousTraits->userInterfaceStyle() : inheritedStyle;
    const bool enabled = _glassEffect && _glassEffect->adaptsAppearanceToBackdrop()
        && _glassEffect->style() == UIGlassEffect::Style::regular;
    const auto style = enabled && luminance
        ? Internal::glassAppearance(layer()->presentationOrSelf()->bounds().size,
                                   inheritedStyle, *luminance, previousStyle)
        : inheritedStyle;
    if (previousStyle == style && (!_contentTraits || style != inheritedStyle)) return;
    if (style == inheritedStyle) _contentTraits.reset();
    else {
        _contentTraits = inherited ? inherited->copy() : std::make_shared<UITraitCollection>();
        _contentTraits->_userInterfaceStyle = style;
    }
    if (previousStyle != style && propagateTraits) {
        // This propagates color traits through ordinary content while the
        // host itself keeps receiving the parent's original trait collection.
        UIView::traitCollectionDidChange(previousTraits);
        _updateGlassEffect(true);
        layer()->setNeedsDisplay();
    }
}

void BackdropEffectView::_configureInteraction() {
    const bool enabled = _glassEffect && _glassEffect->isInteractive();
    if (!enabled) {
        _resetInteraction();
        return;
    }
    if (!_interaction) {
        _interaction = std::make_unique<InteractionState>();
        _interaction->observer = new_shared<GlassTouchObserver>();
        const auto weak = weak_from_base<BackdropEffectView>();
        _interaction->observer->observe = [weak](UITouchPhase phase, const auto& touch) {
            if (const auto view = weak.lock()) view->_handleGlassTouch(phase, touch);
        };
        addGestureRecognizer(_interaction->observer);
    }
    // Keep this passive observer enabled after an effect is switched off.
    // It must finish accounting for an in-flight touch even if the effect is
    // enabled again before that finger lifts. _handleGlassTouch gates visuals.
}

void BackdropEffectView::_resetInteraction() {
    if (!_interaction) return;
    if (_interaction->displayLink) _interaction->displayLink->invalidate();
    _interaction->ticking = false;
    _interaction->touch.reset();
    _interaction->window.reset();
    _interaction->motion = {};
    _backdropEffectLayer()->setGlassInteraction(_interaction->motion);
}

void BackdropEffectView::_handleGlassTouch(UITouchPhase phase, const std::shared_ptr<UITouch>& touch) {
    if (!_interaction || !_glassEffect || !_glassEffect->isInteractive() || !touch) return;
    auto& state = *_interaction;
    if (phase == UITouchPhase::began) {
        if (!state.touch.expired()) return;
        const auto local = touch->locationIn(shared_from_this());
        if (!std::isfinite(local.x) || !std::isfinite(local.y)) return;
        // A rounded glass corner outside the material must not light the lens.
        const auto size = bounds().size;
        if (!std::isfinite(size.width) || !std::isfinite(size.height)
            || size.width <= 0 || size.height <= 0) return;
        const auto p = local - bounds().origin - NXPoint(size.width * 0.5f, size.height * 0.5f);
        if (Internal::layerCornerDistance(p, size, layer()->cornerRadius(), layer()->cornerCurve()) > 0) return;
        state.touch = touch;
        state.window = window();
        state.parent = superview();
        state.origin = local;
        state.motion.pressed = true;
        state.motion.move(local - bounds().origin, {});
    } else {
        if (state.touch.lock() != touch) return;
        if (phase == UITouchPhase::moved) {
            const auto local = touch->locationIn(shared_from_this());
            state.motion.move(local - bounds().origin, local - state.origin);
        } else {
            state.touch.reset();
            state.motion.pressed = false;
        }
    }
    _wakeInteraction();
}

void BackdropEffectView::_wakeInteraction() {
    auto& state = *_interaction;
    if (state.ticking) return;
    state.ticking = true;
    state.lastFrame = std::chrono::steady_clock::now();
    const auto weak = weak_from_base<BackdropEffectView>();
    state.displayLink = std::make_shared<CADisplayLink>([weak] {
        const auto view = weak.lock();
        if (!view || !view->_interaction) return;
        auto& state = *view->_interaction;
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration<double>(now - state.lastFrame).count();
        state.lastFrame = now;
        view->_advanceInteraction(elapsed);
    });
    layer()->setNeedsDisplay();
}

void BackdropEffectView::_advanceInteraction(double seconds) {
    if (!_interaction) return;
    auto& state = *_interaction;
    // Removal, reparenting, hiding or disabling anywhere above the effect must
    // never leave a held material or an active frame callback behind.
    if (!_glassEffect || !_glassEffect->isInteractive()
        || state.window.expired() || window() != state.window.lock()
        || state.parent.lock() != superview().lock()) {
        _resetInteraction();
        return;
    }
    for (auto ancestor = shared_from_this(); ancestor; ancestor = ancestor->superview().lock()) {
        if (ancestor->isHidden() || !ancestor->isUserInteractionEnabled() || ancestor->alpha() < 0.01f) {
            _resetInteraction();
            return;
        }
    }
    if (state.motion.pressed && state.touch.expired()) state.motion.pressed = false;
    state.motion.advance(seconds, layer()->presentationOrSelf()->bounds().size);
    _backdropEffectLayer()->setGlassInteraction(state.motion);
    if (!state.motion.atRest()) CALayer::requestFramerate(120);
    if (state.motion.atRest() && !state.motion.pressed) {
        state.displayLink->invalidate();
        state.ticking = false;
    }
}

NXAffineTransform BackdropEffectView::interactionTransform() const {
    return _backdropEffectLayer()->_interaction->transform(layer()->presentationOrSelf()->bounds().size);
}

BackdropEffect BackdropEffectView::effect() const {
    return _backdropEffectLayer()->effect();
}

void BackdropEffectView::_updateGlassEffect(bool force) {
    if (!_glassEffect) return;

    const auto size = bounds().size;
    const auto traits = traitsForSubviews();
    const auto interfaceStyle = traits
        ? traits->userInterfaceStyle()
        : UIUserInterfaceStyle::unspecified;
    if (!force
        && _hasResolvedGlass
        && _resolvedGlassSize == size
        && _resolvedInterfaceStyle == interfaceStyle)
    {
        return;
    }

    _backdropEffectLayer()->setEffect(
        _glassEffect->_backdropEffect(size, traits)
    );
    _backdropEffectLayer()->setGlassMaterial(_glassEffect, traits);
    _resolvedGlassSize = size;
    _resolvedInterfaceStyle = interfaceStyle;
    _hasResolvedGlass = true;
}
