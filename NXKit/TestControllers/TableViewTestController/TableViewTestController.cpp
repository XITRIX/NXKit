//
//  TableViewTestController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

#include "TableViewTestController.hpp"

TableViewTestController::TableViewTestController() {
    setTitle("Table view");
}

void TableViewTestController::loadView() {
    tableView = NXKit::make_shared<UITableView>();
    tableView->setFixWidth(true);
    tableView->scrollingMode = UIScrollViewScrollingMode::centered;
    setView(tableView);
}

void TableViewTestController::viewDidLoad() {
    tableView->dataSource = shared_from_base<TableViewTestController>();
    tableView->registerView("Cell", []() { return new UITableViewDefaultCell(); });
    tableView->registerView("SwitchCell", []() { return new UITableViewSwitchCell(); });
    tableView->reloadData();
}

int TableViewTestController::tableViewNumberOfRowsInSection(std::shared_ptr<UITableView> tableView, int section) {
    return 200;
}

std::shared_ptr<UITableViewCell> TableViewTestController::tableViewCellForRowAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) {
    if (indexPath.row() < 4) {
        auto item = std::dynamic_pointer_cast<UITableViewSwitchCell>(tableView->dequeueReusableCell("SwitchCell", indexPath));
        item->setText("Switch cell #" + std::to_string(indexPath.row() + 1));
        item->setOn(true, false);
        item->setImage(NXKit::make_shared<UIImage>(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
        item->imageView->setTintColor(UIColor::label);
        return item;
    }

    auto item = std::dynamic_pointer_cast<UITableViewDefaultCell>(tableView->dequeueReusableCell("Cell", indexPath));
    item->setText("Detail cell #" + std::to_string(indexPath.row() + 1));
    item->setDetailText("Detail text");
    item->setImage(NXKit::make_shared<UIImage>(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
    item->imageView->setTintColor(UIColor::label);
    return item;
}

int TableViewTestController::tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) {
//    if (indexPath.row() == 1) return false;
    return true;
}
