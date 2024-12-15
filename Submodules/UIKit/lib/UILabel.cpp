#include <UILabel.h>

using namespace NXKit;

UILabel::UILabel(): UIView(NXRect(), new_shared<CATextLayer>()) {

}

std::shared_ptr<CATextLayer> UILabel::_textLayer() {
    return std::static_pointer_cast<CATextLayer>(layer());
}

void UILabel::setTextColor(UIColor textColor) {
    _textLayer()->setTextColor(textColor);
}

UIColor UILabel::textColor() {
    return _textLayer()->textColor();
}
