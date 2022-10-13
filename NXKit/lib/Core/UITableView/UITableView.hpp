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
    virtual int numberOfSectionsIn(std::shared_ptr<UITableView> tableView) { return 1; }
    virtual int tableViewCellCanBeFocusedAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) { return true; }
    virtual int tableViewNumberOfRowsInSection(std::shared_ptr<UITableView> tableView, int section) = 0;
    virtual std::shared_ptr<UITableViewCell> tableViewCellForRowAt(std::shared_ptr<UITableView> tableView, IndexPath indexPath) = 0;
};

class UITableViewDelegate: public UIScrollViewDelegate {
public:
    virtual void tableViewDidSelectRowAtIndexPath(std::shared_ptr<UITableView> tableView, IndexPath indexPath) {}
};

class UITableView: public UIScrollView {
public:
    float rowHeight = AUTO;
    float estimatedRowHeight = 70;

    std::weak_ptr<UITableViewDelegate> delegate;
    std::weak_ptr<UITableViewDataSource> dataSource;

    UITableView();
    virtual ~UITableView();

    std::shared_ptr<UIView> getDefaultFocus() override;
    std::shared_ptr<UIView> getNextFocus(NavigationDirection direction) override;
    void subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView) override;

    void addSubview(std::shared_ptr<UIView> view) override;
    Size getContentSize() override;
    void layoutSubviews() override;
    void setContentOffset(Point offset, bool animated = true) override;

    void registerView(std::string reuseId, std::function<std::shared_ptr<UITableViewCell>()> allocator);
    std::shared_ptr<UITableViewCell> dequeueReusableCell(std::string reuseId, IndexPath indexPath);
    void reloadData();

    void setPaddings(float top, float left, float bottom, float right);
    void setPaddings(UIEdgeInsets paddings);
    UIEdgeInsets getPaddings() { return paddings; }
private:
    UIEdgeInsets paddings;
    std::map<std::string, std::function<std::shared_ptr<UITableViewCell>()>> allocationMap;
    std::map<std::string, std::vector<std::shared_ptr<UITableViewCell>>> cellsInQueue;
    std::vector<std::vector<float>> cellsHeights;
    std::vector<UITableViewCell*> dequeuedCells;
    std::vector<std::vector<UITableViewCell*>> cellsInIndexPaths;
    IndexPath selectedIndexPath = IndexPath(-1, -1);

    void recalculateEstimatedHeights();
    void dequeCellsForCurrentContentOffset();
    void addCellSubview(std::shared_ptr<UITableViewCell> view);
};

}
