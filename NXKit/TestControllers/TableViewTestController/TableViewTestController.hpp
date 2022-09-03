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

    int tableViewNumberOfRowsInSection(UITableView* tableView, int section) override;
    UITableViewCell* tableViewCellForRowAt(UITableView* tableView, IndexPath indexPath) override;
    int tableViewCellCanBeFocusedAt(UITableView *tableView, IndexPath indexPath) override;
private:
    UITableView* tableView;
};
