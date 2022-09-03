//
//  UITableView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

//#include <Core/Utils/Tools/Tools.hpp>
#include <Core/UITableView/UITableView.hpp>

namespace NXKit {

UITableView::UITableView() {
    UIScrollView::delegate = delegate;
    setPaddings(32, 40, 47, 80);
}

void UITableView::setPaddings(float top, float left, float bottom, float right) {
    this->paddings = UIEdgeInsets(top, left, bottom, right);
}

void UITableView::setPaddings(UIEdgeInsets paddings) {
    this->paddings = paddings;
}

void UITableView::setContentOffset(Point offset, bool animated) {
    UIScrollView::setContentOffset(offset, animated);
    if (!animated)
        dequeCellsForCurrentContentOffset();
}

void UITableView::addSubview(UIView *view) { }

Size UITableView::getContentSize() {
    Size size;
    size.width = getBounds().size.width;
    size.height = getPaddings().top + getPaddings().bottom;

    for (int section = 0; section < cellsHeights.size(); section++) {
        for (int row = 0; row < cellsHeights[section].size(); row++) {
            size.height += cellsHeights[section][row];
        }
    }

    return size;
}

void UITableView::registerView(std::string reuseId, std::function<UITableViewCell*()> allocator) {
    allocationMap[reuseId] = allocator;
    cellsInQueue[reuseId] = std::vector<UITableViewCell*>();
}

UITableViewCell* UITableView::dequeueReusableCell(std::string reuseId, IndexPath indexPath) {
    if (cellsInQueue[reuseId].size() > 0) {
        auto cell = pop(&cellsInQueue[reuseId]);
        cell->indexPath = indexPath;
        return cell;
    }

    auto cell = allocationMap[reuseId]();
    cell->reuseIdentifier = reuseId;
    cell->indexPath = indexPath;
    return cell;
}

void UITableView::reloadData() {
    if (!dataSource) return;

    recalculateEstimatedHeights();
    dequeCellsForCurrentContentOffset();
}

void UITableView::recalculateEstimatedHeights() {
    cellsHeights.clear();
    cellsInIndexPaths.clear();

    int sections = dataSource->numberOfSectionsIn(this);

    for (int section = 0; section < sections; section++) {
        cellsHeights.push_back(std::vector<float>());
        cellsInIndexPaths.push_back(std::vector<UITableViewCell*>());

        int rows = dataSource->tableViewNumberOfRowsInSection(this, section);
        for (int row = 0; row < rows; row++) {
            IndexPath indexPath = IndexPath(row, section);
            if (selectedIndexPath.section() == -1 && delegate && dataSource->tableViewCellCanBeFocusedAt(this, indexPath))
                selectedIndexPath = indexPath;
            
            cellsHeights[section].push_back(isnan(rowHeight) ? estimatedRowHeight : rowHeight);
            cellsInIndexPaths[section].push_back(nullptr);
        }
    }
}

void UITableView::dequeCellsForCurrentContentOffset() {
    auto paddings = getPaddings();
    float currentY = paddings.top;
    Rect bounds = getBounds();

    std::vector<IndexPath> presentedIndexPaths;
    for (int i = (int)dequeuedCells.size() - 1; i >= 0; i--) {
        auto cell = dequeuedCells[i];
        if (!cell->getFrame().intersects(bounds)) {
            cell->removeFromSuperview();
            cellsInQueue[cell->getReuseIdentifier()].push_back(cell);
            dequeuedCells.erase(dequeuedCells.begin() + i);
            cellsInIndexPaths[cell->indexPath.section()][cell->indexPath.row()] = nullptr;
        } else {
            presentedIndexPaths.push_back(cell->indexPath);
        }
    }

    for (int section = 0; section < cellsHeights.size(); section++) {
        for (int row = 0; row < cellsHeights[section].size(); row++) {
            if (currentY >= bounds.minY() && currentY <= bounds.maxY()) {
                IndexPath indexPath = IndexPath(row, section);
                if (cellsInIndexPaths[indexPath.section()][indexPath.row()] == nullptr) {
                    
                    auto cell = dataSource->tableViewCellForRowAt(this, indexPath);
                    if (row == 0) cell->setBorderTop(1);
                    else { cell->setBorderTop(0); }
                    cell->setBorderBottom(1);
                    
                    cell->layoutSubviews();
                    float height = cell->getBounds().height();
                    cellsHeights[section][row] = height;
                    
                    cell->setWidth(bounds.width() - paddings.left - paddings.right);
                    cell->setPosition(Point(paddings.left, currentY));
                    
                    addCellSubview(cell);
                    dequeuedCells.push_back(cell);
                    cellsInIndexPaths[indexPath.section()][indexPath.row()] = cell;
                }
            }
            currentY += cellsHeights[section][row];
        }
    }
}

void UITableView::addCellSubview(UITableViewCell *view) {
    view->setSuperview(this);
    subviews.push_back(view);
}

void UITableView::layoutSubviews() {
    UIScrollView::layoutSubviews();
    dequeCellsForCurrentContentOffset();
}

}
