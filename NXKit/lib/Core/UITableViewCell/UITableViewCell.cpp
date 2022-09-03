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
    if (tableView->dataSource)
        return tableView->dataSource->tableViewCellCanBeFocusedAt(tableView, indexPath);
    return false;
}

//bool UITableViewCell::canBecomeFocused() {
//    if (tableView->dataSource)
//        return tableView->dataSource->tableViewCellCanBeFocusedAt(tableView, indexPath);
//    return false;
//}

}
