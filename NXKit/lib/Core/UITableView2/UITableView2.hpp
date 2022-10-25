//
//  UITableView2.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 10.10.2022.
//

#pragma once

#include <Core/UIScrollView/UIScrollView.hpp>
#include <Core/UITableView/UITableView.hpp>

namespace NXKit {

class UITableView2;
class UITableView2CellPrototype;
class UITableView2DataSource {
public:
    virtual int numberOfSectionsIn(std::shared_ptr<UITableView2> tableView) { return 1; }
    virtual int tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) { return true; }
    virtual int tableViewNumberOfRowsInSection(std::shared_ptr<UITableView2> tableView, int section) = 0;
    virtual std::shared_ptr<UITableViewCell> tableViewCellForRowAt(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) = 0;
};

class UITableView2Delegate: public UIScrollViewDelegate {
public:
    virtual void tableViewDidSelectRowAtIndexPath(std::shared_ptr<UITableView2> tableView, IndexPath indexPath) {}
};

class UITableView2: public UIScrollView {
public:
    float rowHeight = AUTO;
    float estimatedRowHeight = 170;

    std::weak_ptr<UITableView2Delegate> delegate;
    std::weak_ptr<UITableView2DataSource> dataSource;

    UITableView2();
    virtual ~UITableView2();

    Size getContentSize() override;
    void layoutSubviews() override;
    void addSubview(std::shared_ptr<UIView> view) override;

    void registerView(std::string reuseId, std::function<std::shared_ptr<UITableViewCell>()> allocator);
    void reloadData();

    template <class T, class = std::enable_if<std::is_base_of<UITableViewCell, T>::value>>
    std::shared_ptr<T> dequeueReusableCell(std::string reuseId, IndexPath indexPath) {
        return std::dynamic_pointer_cast<T>(_dequeueReusableCell(reuseId, indexPath));
    }

private:
    int numberOfSections;
    std::map<int, int> numberOfRows;
    Size contentSize;

    std::map<std::string, std::function<std::shared_ptr<UITableViewCell>()>> allocationMap;
    std::map<std::string, std::vector<std::shared_ptr<UITableViewCell>>> cellsQueue;
    std::map<IndexPath, UITableView2CellPrototype> cellsMatrix;

    std::shared_ptr<UITableViewCell> _dequeueReusableCell(std::string reuseId, IndexPath indexPath);
    void precalculateFrames();
    void addCellAt(IndexPath indexPath);
    void updateCellHeightAt(IndexPath indexPath);
};

}
