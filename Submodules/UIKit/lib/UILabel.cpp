#include <UILabel.h>

using namespace NXKit;

UILabel::UILabel(): UIView(NXRect(), new_shared<CATextLayer>()) {

}

std::shared_ptr<CATextLayer> UILabel::_textLayer() const {
    return std::static_pointer_cast<CATextLayer>(layer());
}
