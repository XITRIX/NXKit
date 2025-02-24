#include <UIImageView.h>
#include <tools/IBTools.h>

#include <utility>

using namespace NXKit;

UIImageView::UIImageView(std::shared_ptr<UIImage> image): UIImageView(NXRect()) {
    _image = std::move(image);
    updateTextureFromImage();
}

UIImageView::UIImageView(NXRect frame): UIView(frame) {
    setUserInteractionEnabled(false);
}

void UIImageView::setImage(const std::shared_ptr<UIImage>& image) {
    if (image == _image) { return; }
    _image = image;

    updateTextureFromImage();
//    setNeedsLayout();
}

void UIImageView::updateTextureFromImage() {
    if (_image) {
//        _image->cgImage()
        layer()->setContentsTemplateMode(_image->_isTemplate);
        layer()->setContents(_image->cgImage());
        layer()->setContentsScale(_image->scale());

        auto bounds = this->bounds();
        bounds.size = _image->size();
        setBounds(bounds);
    } else {
        layer()->setContents(nullptr);
        layer()->setContentsScale(UITraitCollection::current()->displayScale());
    }
}

// void UIImageView::sizeToFit() {
//     UIView::sizeToFit();
// }

 NXSize UIImageView::sizeThatFits(NXSize size) {
     if (!_image) return UIView::sizeThatFits(size);
     return _image->size();
 }

 bool UIImageView::applyXMLAttribute(const std::string& name, const std::string& value) {
     if (UIView::applyXMLAttribute(name, value)) return true;

     REGISTER_XIB_ATTRIBUTE(imagePath, valuePathToImage, setImage)
     REGISTER_XIB_ATTRIBUTE(image, valueResToImage, setImage)

     return false;
 }
