//
//  UITableView.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 02.09.2022.
//

//#include <Core/Utils/Tools/Tools.hpp>
#include <Core/UITableView/UITableView.hpp>
#include <Core/Application/Application.hpp>

namespace NXKit {

UITableView::UITableView() {
    UIScrollView::delegate = std::dynamic_pointer_cast<UIScrollViewDelegate>(delegate);
    setPaddings(32, 40, 47, 80);
}

UITableView::~UITableView() {
//    for (auto it = cellsInQueue.begin(); it != cellsInQueue.end(); it++) {
//        for (auto cell: it->second) {
//            delete cell;
//        }
//    }
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

std::shared_ptr<UIView> UITableView::getDefaultFocus() {
    if (selectedIndexPath.section() != -1 && cellsInIndexPaths[selectedIndexPath.section()][selectedIndexPath.row()] != nullptr)
        return cellsInIndexPaths[selectedIndexPath.section()][selectedIndexPath.row()];
    return nullptr;
}

std::shared_ptr<UIView> UITableView::getNextFocus(NavigationDirection direction) {
//    if (currentFocus >= getSubviews().size()) currentFocus = (int) getSubviews().size() - 1;

    if ((direction == NavigationDirection::LEFT || direction == NavigationDirection::RIGHT))
        return UIView::getNextFocus(direction);

    if (direction == NavigationDirection::UP) {
        IndexPath newFocus = selectedIndexPath;
        std::shared_ptr<UIView> newFocusView;

        do {
            newFocus = newFocus.prev();
            if (newFocus.section() < 0) return UIScrollView::getNextFocus(direction);
            newFocusView = cellsInIndexPaths[newFocus.section()][newFocus.row()];
        } while (!newFocusView || !dataSource->tableViewCellCanBeFocusedAt(shared_from_base<UITableView>(), newFocus));

        if (newFocusView && newFocusView->canBecomeFocused()) {
            selectedIndexPath = newFocus;
            return newFocusView;
        }
    }

    if (direction == NavigationDirection::DOWN) {
        IndexPath newFocus = selectedIndexPath;
        std::shared_ptr<UIView> newFocusView = nullptr;

        do {
            newFocus = newFocus.next(dataSource->tableViewNumberOfRowsInSection(shared_from_base<UITableView>(), newFocus.section()));
            if (newFocus.section() >= dataSource->numberOfSectionsIn(shared_from_base<UITableView>())) return UIScrollView::getNextFocus(direction);
            newFocusView = cellsInIndexPaths[newFocus.section()][newFocus.row()];
        } while (!newFocusView || !dataSource->tableViewCellCanBeFocusedAt(shared_from_base<UITableView>(), newFocus));

        if (newFocusView && newFocusView->canBecomeFocused()) {
            selectedIndexPath = newFocus;
            return newFocusView;
        }
    }

    return UIScrollView::getNextFocus(direction);
}

void UITableView::subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView) {
    UIScrollView::subviewFocusDidChange(focusedView, notifiedView);
    auto cell = std::dynamic_pointer_cast<UITableViewCell>(notifiedView);
    if (cell && cell->tableView.get() == this) {
        selectedIndexPath = cell->indexPath;
    }
}

void UITableView::addSubview(std::shared_ptr<UIView> view) { }

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
    cellsInQueue[reuseId] = std::vector<std::shared_ptr<UITableViewCell>>();
}

std::shared_ptr<UITableViewCell> UITableView::dequeueReusableCell(std::string reuseId, IndexPath indexPath) {
    // Dequeue existing Cell
    if (cellsInQueue[reuseId].size() > 0) {
        auto cell = pop(&cellsInQueue[reuseId]);
        cell->indexPath = indexPath;
        return cell;
    }

    // Instantiate new Cell
    auto cell = std::shared_ptr<UITableViewCell>(allocationMap[reuseId]());
    cell->reuseIdentifier = reuseId;
    cell->indexPath = indexPath;
    cell->tableView = shared_from_base<UITableView>();
    cell->onEvent = [this, cell](UIControlTouchEvent event) {
        if (event == UIControlTouchEvent::touchUpInside) {
            if (this->delegate)
                this->delegate->tableViewDidSelectRowAtIndexPath(shared_from_base<UITableView>(), cell->indexPath);
            Application::shared()->setFocus(cell);
        }
    };
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

    int sections = dataSource->numberOfSectionsIn(shared_from_base<UITableView>());

    for (int section = 0; section < sections; section++) {
        cellsHeights.push_back(std::vector<float>());
        cellsInIndexPaths.push_back(std::vector<std::shared_ptr<UITableViewCell>>());

        int rows = dataSource->tableViewNumberOfRowsInSection(shared_from_base<UITableView>(), section);
        for (int row = 0; row < rows; row++) {
            IndexPath indexPath = IndexPath(row, section);
            if (selectedIndexPath.section() == -1 && dataSource && dataSource->tableViewCellCanBeFocusedAt(shared_from_base<UITableView>(), indexPath))
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
            auto nextCellBottom = currentY + cellsHeights[section][row];
            if (nextCellBottom >= bounds.minY() && currentY <= bounds.maxY()) {
                IndexPath indexPath = IndexPath(row, section);
                if (cellsInIndexPaths[indexPath.section()][indexPath.row()] == nullptr) {
                    auto cell = dataSource->tableViewCellForRowAt(shared_from_base<UITableView>(), indexPath);
                    if (row == 0) cell->setBorderTop(1);
                    else { cell->setBorderTop(0); }
                    cell->setBorderBottom(1);
                    cell->borderColor = UIColor::separator;
                    
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

void UITableView::addCellSubview(std::shared_ptr<UITableViewCell> view) {
    view->setSuperview(shared_from_base<UITableView>());
    subviews.push_back(view);
}

void UITableView::layoutSubviews() {
    UIScrollView::layoutSubviews();
    dequeCellsForCurrentContentOffset();
}

}
