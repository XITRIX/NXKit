//
//  UITableViewCell.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>
#include <Core/UIControl/UIControl.hpp>

namespace NXKit {

class UITableViewCell: public UIControl {
public:
    UITableViewCell() {}
    std::string getReuseIdentifier() { return reuseIdentifier; }
private:
    friend class UITableView;
    std::string reuseIdentifier;
    IndexPath indexPath;
};

}
