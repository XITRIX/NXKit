#include <UIApplicationDelegate.h>
#include <UIKit.h>

namespace NXKit {

void UIApplicationDelegate::applicationNeedsXIBRegistration(UIApplication* application) {
    REGISTER_NIB(UIView)
    REGISTER_NIB(UILabel)
    REGISTER_NIB(UIControl)
    REGISTER_NIB(UIButton)
    REGISTER_NIB(UIImageView)
    REGISTER_NIB(UIScrollView)
}

}