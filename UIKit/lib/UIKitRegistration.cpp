#include <UIKitRegistration.h>
#include <UINib.h>
#include <UIButton.h>
#include <BackdropEffectView.h>
#include <GlassContainerView.h>
#include <LensView.h>

void NXKit::registerUIKitViews() {
    UINib::registerXIB("UIButton", UIButton::init);
    UINib::registerXIB("BackdropEffectView", BackdropEffectView::init);
    UINib::registerXIB("GlassContainerView", GlassContainerView::init);
    UINib::registerXIB("LensView", LensView::init);
}
