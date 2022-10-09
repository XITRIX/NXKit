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

class UITableView;
class UITableViewCell: public UIControl {
public:
    UITableViewCell() {}

    bool isEnabled() override;
//    bool canBecomeFocused() override;

    std::string getReuseIdentifier() { return reuseIdentifier; }
private:
    friend class UITableView;
    std::shared_ptr<UITableView> tableView;
    std::string reuseIdentifier;
    IndexPath indexPath;
};

}
