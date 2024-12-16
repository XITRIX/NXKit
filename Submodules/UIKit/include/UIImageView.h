#pragma once

#include <UIView.h>
#include <UIImage.h>

namespace NXKit {

class UIImageView: public UIView {
public:
    static std::shared_ptr<UIImageView> init();

    UIImageView(std::shared_ptr<UIImage> image = nullptr);
    UIImageView(NXRect frame);

    void setImage(std::shared_ptr<UIImage> image);
    std::shared_ptr<UIImage> image() { return _image; }

    // void sizeToFit() override;
    NXSize sizeThatFits(NXSize size) override;
    // bool applyXMLAttribute(std::string name, std::string value) override;

private:
    std::shared_ptr<UIImage> _image;
    void updateTextureFromImage();
};

}
