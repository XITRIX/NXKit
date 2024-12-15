#include <UIBlurView.h>

using namespace NXKit;

UIBlurView::UIBlurView(): UIView(NXRect(), new_shared<CABlurLayer>()) {}

std::shared_ptr<CABlurLayer> UIBlurView::_blurLayer() const {
    return std::static_pointer_cast<CABlurLayer>(layer());
}
