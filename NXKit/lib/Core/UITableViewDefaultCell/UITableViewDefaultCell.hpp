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
    void setImage(UIImage* image);

    bool canBecomeFocused() override;
private:
    UILabel* label;
    UIImageView* imageView;
};

}
