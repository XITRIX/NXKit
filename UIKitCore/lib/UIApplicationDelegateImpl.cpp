#include <UIApplicationDelegate.h>
#include <UIKitCore.h>

namespace NXKit {

void UIApplicationDelegate::applicationNeedsXIBRegistration(UIApplication* application) {
    REGISTER_NIB(UIView)
    REGISTER_NIB(UILabel)
    REGISTER_NIB(UIControl)
    REGISTER_NIB(UIImageView)
    REGISTER_NIB(UIScrollView)
    REGISTER_NIB(UITextView)
}

}
