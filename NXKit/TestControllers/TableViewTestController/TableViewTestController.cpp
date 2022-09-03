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
//    tableView->setMargins(<#float top#>, <#float right#>, <#float bottom#>, <#float left#>)
    setView(tableView);
}

void TableViewTestController::viewDidLoad() {
    tableView->dataSource = this;
    tableView->registerView("Cell", []() { return new UITableViewDefaultCell(); });
    tableView->reloadData();
}

int TableViewTestController::tableViewNumberOfRowsInSection(UITableView* tableView, int section) {
    return 200;
}

UITableViewCell* TableViewTestController::tableViewCellForRowAt(UITableView* tableView, IndexPath indexPath) {
    auto item = (UITableViewDefaultCell*) tableView->dequeueReusableCell("Cell", indexPath);
    item->setText("Test text #" + std::to_string(indexPath.row() + 1));
    item->setImage(new UIImage(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", 2));
    item->borderColor = UIColor::separator;
    return item;
}

int TableViewTestController::tableViewCellCanBeFocusedAt(UITableView *tableView, IndexPath indexPath) {
    if (indexPath.row() == 1) return false;
    return true;
}
