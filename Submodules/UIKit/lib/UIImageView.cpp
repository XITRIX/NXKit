#include <UIImageView.h>

using namespace NXKit;

std::shared_ptr<UIImageView> UIImageView::init() {
    return new_shared<UIImageView>();
}

UIImageView::UIImageView(std::shared_ptr<UIImage> image): UIImageView(NXRect()) {
    _image = image;
    updateTextureFromImage();
}

UIImageView::UIImageView(NXRect frame): UIView(frame) {
    setUserInteractionEnabled(false);
}

void UIImageView::setImage(std::shared_ptr<UIImage> image) {
    if (image == _image) { return; }
    _image = image;

    updateTextureFromImage();
//    setNeedsLayout();
}

void UIImageView::updateTextureFromImage() {
    if (_image) {
        layer()->setContents(_image->cgImage());
        layer()->setContentsScale(_image->scale());

        auto bounds = this->bounds();
        bounds.size = _image->size();
        setBounds(bounds);
    } else {
        layer()->setContents(nullptr);
        layer()->setContentsScale(SkiaCtx::main()->getScaleFactor());
    }
}

// void UIImageView::sizeToFit() {
//     UIView::sizeToFit();
// }

 NXSize UIImageView::sizeThatFits(NXSize size) {
     if (!_image) return UIView::sizeThatFits(size);
     return _image->size();
 }

// bool UIImageView::applyXMLAttribute(std::string name, std::string value) {
//     if (UIView::applyXMLAttribute(name, value)) return true;

//     REGISTER_XIB_ATTRIBUTE(image, valueToImage, setImage)

//     return false;
// }
