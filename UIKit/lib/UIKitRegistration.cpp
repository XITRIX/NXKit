#include <UIKitRegistration.h>
#include <UINib.h>
#include <UINavigationBar.h>
#include <UIButton.h>
#include <UITabBarView.h>
#include <BackdropEffectView.h>
#include <GlassContainerView.h>
#include <LensView.h>

void NXKit::registerUIKitViews() {
    UINib::registerXIB("UINavigationBar", UINavigationBar::init);
    UINib::registerXIB("UITabBarView", UITabBarView::init);
    UINib::registerXIB("UITabBar", UITabBarView::init);
    UINib::registerXIB("UIButton", UIButton::init);
    UINib::registerXIB("BackdropEffectView", BackdropEffectView::init);
    UINib::registerXIB("GlassContainerView", GlassContainerView::init);
    UINib::registerXIB("LensView", LensView::init);
}
