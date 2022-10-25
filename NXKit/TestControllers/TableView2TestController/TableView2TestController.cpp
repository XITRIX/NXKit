//
//  TableViewTestController.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

#include "TableView2TestController.hpp"

TableView2TestController::TableView2TestController() {
    setTitle("Table view");
}

void TableView2TestController::loadView() {
    tableView = NXKit::make_shared<UITableView2>();
    tableView->scrollingMode = UIScrollViewScrollingMode::centered;
    tableView->contentInset = UIEdgeInsets(32, 40, 47, 80);
//    tableView->backgroundColor = UIColor::red;
    setView(tableView);
}

void TableView2TestController::viewDidLoad() {
    tableView->dataSource = shared_from_base<TableView2TestController>();
    tableView->registerView("Cell", []() { return make_shared<UITableViewDefaultCell>(); });
    tableView->registerView("SwitchCell", []() { return make_shared<UITableViewSwitchCell>(); });
    tableView->reloadData();
}

int TableView2TestController::tableViewNumberOfRowsInSection(std::shared_ptr<UITableView2> tableView, int section) {
    return 15;
}

std::shared_ptr<UITableViewCell> TableView2TestController::tableViewCellForRowAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) {
    if (indexPath.row() < 4) {
//        auto item = std::dynamic_pointer_cast<UITableViewSwitchCell>(tableView->dequeueReusableCell("SwitchCell", indexPath));
//        item->setText("Switch cell #" + std::to_string(indexPath.row() + 1));
//        item->setOn(true, false);
//        item->setImage(NXKit::make_shared<UIImage>(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
//        item->imageView->setTintColor(UIColor::label);
//        return item;
    }

    auto item = tableView->dequeueReusableCell<UITableViewDefaultCell>("Cell", indexPath);
    item->setText("Detail cell #" + std::to_string(indexPath.row() + 1));
    item->setDetailText("Detail text");
    item->setImage(NXKit::make_shared<UIImage>(Application::shared()->getResourcesPath() + "Images/test/" + std::to_string(indexPath.row()%10) + ".svg", true, 2));
    item->imageView->setTintColor(UIColor::label);
    return item;

//    return nullptr;
}

int TableView2TestController::tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) {
//    if (indexPath.row() == 1) return false;
    return true;
}
