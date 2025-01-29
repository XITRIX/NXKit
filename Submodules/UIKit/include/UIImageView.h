#pragma once

#include <UIView.h>
#include <UIImage.h>

namespace NXKit {

class UIImageView: public UIView {
public:
    static std::shared_ptr<UIView> init() { return new_shared<UIImageView>(); }

    explicit UIImageView(std::shared_ptr<UIImage> image = nullptr);
    explicit UIImageView(NXRect frame);

    void setImage(const std::shared_ptr<UIImage>& image);
    std::shared_ptr<UIImage> image() { return _image; }

    // void sizeToFit() override;
    NXSize sizeThatFits(NXSize size) override;
    bool applyXMLAttribute(const std::string& name, const std::string& value) override;

private:
    std::shared_ptr<UIImage> _image;
    void updateTextureFromImage();
};

}
