//
//  UITableViewCell.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 03.09.2022.
//

#include <Core/UITableViewCell/UITableViewCell.hpp>
#include <Core/UITableView/UITableView.hpp>

namespace NXKit {

bool UITableViewCell::isEnabled() {
    if (!tableView.expired() && !tableView.lock()->dataSource.expired())
        return tableView.lock()->dataSource.lock()->tableViewCellCanBeFocusedAt(tableView.lock(), indexPath);
    return true;
}

//bool UITableViewCell::canBecomeFocused() {
//    if (tableView->dataSource)
//        return tableView->dataSource->tableViewCellCanBeFocusedAt(tableView, indexPath);
//    return false;
//}

}
