//
//  UITableViewRadioCell.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#pragma once

#include <Core/UILabel/UILabel.hpp>
#include <Core/UIImageView/UIImageView.hpp>
#include <Core/UITableViewCell/UITableViewCell.hpp>

namespace NXKit {

class UICheckBoxView: public UIView {
public:
    UICheckBoxView();
    void draw(NVGcontext* vgContext) override;
};

class UITableViewRadioCell: public UITableViewCell  {
public:
    UITableViewRadioCell();

    void setImage(std::shared_ptr<UIImage> image);
    void setText(std::string text);
    void setOn(bool isOn);

//private:
    std::shared_ptr<UILabel> label;
    std::shared_ptr<UICheckBoxView> checkbox;
    std::shared_ptr<UIImageView> imageView;
};

}
