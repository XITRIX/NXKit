#include <BackdropEffectView.h>

#include "CABackdropEffectLayer.hpp"

using namespace NXKit;

BackdropEffectView::BackdropEffectView():
    BackdropEffectView(UIGlassEffect())
{}

BackdropEffectView::BackdropEffectView(const BackdropEffect& effect):
    UIView(NXRect(), new_shared<CABackdropEffectLayer>(effect))
{}

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
    _updateGlassEffect(true);
}

void BackdropEffectView::setFrame(NXRect frame) {
    UIView::setFrame(frame);
    _updateGlassEffect();
}

void BackdropEffectView::setBounds(NXRect bounds) {
    UIView::setBounds(bounds);
    _updateGlassEffect();
}

void BackdropEffectView::traitCollectionDidChange(
    std::shared_ptr<UITraitCollection> previousTraitCollection
) {
    UIView::traitCollectionDidChange(std::move(previousTraitCollection));
    _updateGlassEffect(true);
}

std::shared_ptr<CABackdropEffectLayer> BackdropEffectView::_backdropEffectLayer() const {
    return std::static_pointer_cast<CABackdropEffectLayer>(layer());
}

void BackdropEffectView::setEffect(const BackdropEffect& effect) {
    _backdropEffectLayer()->setEffect(effect);
    _glassEffect.reset();
    _hasResolvedGlass = false;
}

void BackdropEffectView::setEffect(const UIGlassEffect& effect) {
    const auto size = bounds().size;
    const auto traits = traitCollection();
    const auto resolved = effect._backdropEffect(size, traits);
    _backdropEffectLayer()->setEffect(resolved);

    _glassEffect = effect;
    _resolvedGlassSize = size;
    _resolvedInterfaceStyle = traits
        ? traits->userInterfaceStyle()
        : UIUserInterfaceStyle::unspecified;
    _hasResolvedGlass = true;
}

BackdropEffect BackdropEffectView::effect() const {
    return _backdropEffectLayer()->effect();
}

void BackdropEffectView::_updateGlassEffect(bool force) {
    if (!_glassEffect) return;

    const auto size = bounds().size;
    const auto traits = traitCollection();
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
    _resolvedGlassSize = size;
    _resolvedInterfaceStyle = interfaceStyle;
    _hasResolvedGlass = true;
}
