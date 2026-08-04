#include <BackdropEffectView.h>

#include "CABackdropEffectLayer.hpp"

using namespace NXKit;

BackdropEffectView::BackdropEffectView():
    BackdropEffectView(BackdropEffect::glass())
{}

BackdropEffectView::BackdropEffectView(const BackdropEffect& effect):
    UIView(NXRect(), new_shared<CABackdropEffectLayer>(effect))
{}

std::shared_ptr<CABackdropEffectLayer> BackdropEffectView::_backdropEffectLayer() const {
    return std::static_pointer_cast<CABackdropEffectLayer>(layer());
}

void BackdropEffectView::setEffect(const BackdropEffect& effect) {
    _backdropEffectLayer()->setEffect(effect);
}

BackdropEffect BackdropEffectView::effect() const {
    return _backdropEffectLayer()->effect();
}
