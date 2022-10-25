//
//  UITableView2.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 10.10.2022.
//

#include "UITableView2.hpp"

namespace NXKit {

class UITableView2CellPrototype {
public:
    Rect frame;
    std::weak_ptr<UITableViewCell> cell;
};

UITableView2::UITableView2() {
    UIScrollView::delegate = std::dynamic_pointer_cast<UIScrollViewDelegate>(delegate.lock());
    setFixWidth(true);
}

UITableView2::~UITableView2() { }

Size UITableView2::getContentSize() {
    return contentSize;
}

void UITableView2::addSubview(std::shared_ptr<UIView> view) {
    UIView::addSubview(view);
}

void UITableView2::registerView(std::string reuseId, std::function<std::shared_ptr<UITableViewCell>()> allocator) {
    allocationMap[reuseId] = allocator;
    cellsQueue[reuseId] = std::vector<std::shared_ptr<UITableViewCell>>();
}

std::shared_ptr<UITableViewCell> UITableView2::_dequeueReusableCell(std::string reuseId, IndexPath indexPath) {
    if (cellsQueue[reuseId].size() > 0) {
        auto cell = cellsQueue[reuseId].back();
        cellsQueue[reuseId].pop_back();
        return cell;
    }

    return allocationMap[reuseId]()->shared_from_base<UITableViewCell>();
}

void UITableView2::reloadData() {
    if (dataSource.expired()) return;

    precalculateFrames();

    for (int section = 0; section < numberOfSections; section++) {
        for (int row = 0; row < numberOfRows[section]; row++) {
            addCellAt(IndexPath(row, section));
        }
    }
}

void UITableView2::precalculateFrames() {
    auto _dataSource = dataSource.lock();
    auto self = shared_from_base<UITableView2>();

    numberOfRows.clear();
    float width = getFrame().width() - contentInset.left - contentInset.right;
    float height = contentInset.top;

    numberOfSections = _dataSource->numberOfSectionsIn(self);
    for (int section = 0; section < numberOfSections; section++) {
        numberOfRows[section] = _dataSource->tableViewNumberOfRowsInSection(self, section);
        for (int row = 0; row < numberOfRows[section]; row++) {
            auto indexPath = IndexPath(row, section);
            cellsMatrix[indexPath].frame = Rect(contentInset.left, height, width, estimatedRowHeight);
            height += estimatedRowHeight;
        }
    }

    contentSize.height = height + contentInset.bottom;
}

void UITableView2::addCellAt(IndexPath indexPath) {
    auto self = shared_from_base<UITableView2>();
    auto _dataSource = dataSource.lock();

    auto cell = _dataSource->tableViewCellForRowAt(self, indexPath);
    if (indexPath.row() == 0) { cell->setBorderTop(1); }
    cell->setBorderBottom(1);
    cellsMatrix[indexPath].cell = cell;
    updateCellHeightAt(indexPath);
    cell->setPosition(cellsMatrix[indexPath].frame.origin);
    addSubview(cell);
}

void UITableView2::updateCellHeightAt(IndexPath indexPath) {
    auto cell = cellsMatrix[indexPath].cell.lock();
    cell->layoutIfNeeded();

    Size oldSize = cellsMatrix[indexPath].frame.size;
    cellsMatrix[indexPath].frame.size = cell->getFrame().size;

    float heightDifferance = cellsMatrix[indexPath].frame.size.height - oldSize.height;
    if (heightDifferance != 0) {
        contentSize.height += heightDifferance;
        
        for (int section = indexPath.section(); section < numberOfSections; section++) {
            for (int row = indexPath.row() + 1; row < numberOfRows[section]; row++) {
                cellsMatrix[IndexPath(row, section)].frame.origin.y += heightDifferance;
            }
        }
    }
}

void UITableView2::layoutSubviews() {
    UIScrollView::layoutSubviews();
    contentSize.width = getFrame().width() - contentInset.left - contentInset.right;
    for (auto view: getSubviews()) {
        auto cell = std::dynamic_pointer_cast<UITableViewCell>(view);
        if (!cell) continue;

        cell->setSize(Size(contentSize.width, UIView::AUTO));
    }
}

}
