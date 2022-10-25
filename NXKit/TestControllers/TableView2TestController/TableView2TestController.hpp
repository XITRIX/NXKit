//
//  TableView2TestController.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 11.10.2022.
//

#pragma once

#include "ListViewController.hpp"

class TableView2TestController: public UIViewController, public UITableView2DataSource {
public:
    TableView2TestController();
    void loadView() override;
    void viewDidLoad() override;

    int tableViewNumberOfRowsInSection(std::shared_ptr<UITableView2> tableView, int section) override;
    std::shared_ptr<UITableViewCell> tableViewCellForRowAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) override;
    int tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) override;
private:
    std::shared_ptr<UITableView2> tableView;
};
