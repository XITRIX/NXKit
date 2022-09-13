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
    tableView = new UITableView();
    tableView->setFixWidth(true);
    tableView->scrollingMode = UIScrollViewScrollingMode::scrollingEdge;
    setView(tableView);
}

void TableViewTestController::viewDidLoad() {
    tableView->dataSource = this;
    tableView->registerView("Cell", []() { return new UITableViewDefaultCell(); });
    tableView->registerView("SwitchCell", []() { return new UITableViewSwitchCell(); });
    tableView->reloadData();
}

int TableViewTestController::tableViewNumberOfRowsInSection(UITableView* tableView, int section) {
    return 200;
}

UITableViewCell* TableViewTestController::tableViewCellForRowAt(UITableView* tableView, IndexPath indexPath) {
    if (indexPath.row() < 4) {
        auto item = (UITableViewSwitchCell*) tableView->dequeueReusableCell("SwitchCell", indexPath);
        item->setText("Switch cell #" + std::to_string(indexPath.row() + 1));
        item->setOn(true, false);
        item->setImage(new UIImage(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
        item->imageView->setTintColor(UIColor::label);
        return item;
    }

    auto item = (UITableViewDefaultCell*) tableView->dequeueReusableCell("Cell", indexPath);
    item->setText("Detail cell #" + std::to_string(indexPath.row() + 1));
    item->setDetailText("Detail text");
    item->setImage(new UIImage(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
    item->imageView->setTintColor(UIColor::label);
    return item;
}

int TableViewTestController::tableViewCellCanBeFocusedAt(UITableView *tableView, IndexPath indexPath) {
//    if (indexPath.row() == 1) return false;
    return true;
}
