//
//  UITableViewDefaultCell.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#pragma once

#include <Core/UILabel/UILabel.hpp>
#include <Core/UIImageView/UIImageView.hpp>
#include <Core/UITableViewCell/UITableViewCell.hpp>

namespace NXKit {

class UITableViewDefaultCell: public UITableViewCell  {
public:
    UITableViewDefaultCell();

    void setText(std::string text);
    void setDetailText(std::string text);
    void setImage(std::shared_ptr<UIImage> image);

    void tintColorDidChange() override;

//private:
    std::shared_ptr<UILabel> label = nullptr;
    std::shared_ptr<UILabel> detailLabel = nullptr;
    std::shared_ptr<UIImageView> imageView = nullptr;
};

}
