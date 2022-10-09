//
//  TableViewTestController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

#pragma once

#include "ListViewController.hpp"

class TableViewTestController: public UIViewController, public UITableViewDataSource {
public:
    TableViewTestController();
    void loadView() override;
    void viewDidLoad() override;

    int tableViewNumberOfRowsInSection(std::shared_ptr<UITableView> tableView, int section) override;
    std::shared_ptr<UITableViewCell> tableViewCellForRowAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) override;
    int tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) override;
private:
    std::shared_ptr<UITableView> tableView;
};
