//
//  UITableView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

#pragma once

#include <Core/UIScrollView/UIScrollView.hpp>
#include <Core/UITableViewCell/UITableViewCell.hpp>

#include <map>

namespace NXKit {

class UITableView;
class UITableViewDataSource {
public:
    virtual int numberOfSectionsIn(UITableView* tableView) { return 1; }
    virtual int tableViewCellCanBeFocusedAt(UITableView* tableView, IndexPath indexPath) { return true; }
    virtual int tableViewNumberOfRowsInSection(UITableView* tableView, int section) = 0;
    virtual UITableViewCell* tableViewCellForRowAt(UITableView* tableView, IndexPath indexPath) = 0;
};

class UITableViewDelegate: public UIScrollViewDelegate {
public:
    virtual void tableViewDidSelectRowAtIndexPath(UITableView* tableView, IndexPath indexPath) {}
};

class UITableView: public UIScrollView {
public:
    float rowHeight = AUTO;
    float estimatedRowHeight = 70;

    UITableViewDelegate* delegate = nullptr;
    UITableViewDataSource* dataSource = nullptr;

    UITableView();
    virtual ~UITableView();

    UIView* getDefaultFocus() override;
    UIView* getNextFocus(NavigationDirection direction) override;
    void subviewFocusDidChange(UIView* focusedView, UIView* notifiedView) override;

    void addSubview(UIView *view) override;
    Size getContentSize() override;
    void layoutSubviews() override;
    void setContentOffset(Point offset, bool animated = true) override;

    void registerView(std::string reuseId, std::function<UITableViewCell*()> allocator);
    UITableViewCell* dequeueReusableCell(std::string reuseId, IndexPath indexPath);
    void reloadData();

    void setPaddings(float top, float left, float bottom, float right);
    void setPaddings(UIEdgeInsets paddings);
    UIEdgeInsets getPaddings() { return paddings; }
private:
    UIEdgeInsets paddings;
    std::map<std::string, std::function<UITableViewCell*()>> allocationMap;
    std::map<std::string, std::vector<UITableViewCell*>> cellsInQueue;
    std::vector<std::vector<float>> cellsHeights;
    std::vector<UITableViewCell*> dequeuedCells;
    std::vector<std::vector<UITableViewCell*>> cellsInIndexPaths;
    IndexPath selectedIndexPath = IndexPath(-1, -1);

    void recalculateEstimatedHeights();
    void dequeCellsForCurrentContentOffset();
    void addCellSubview(UITableViewCell *view);
};

}