#pragma once

#include <UIControl.h>
#include <UIPanGestureRecognizer.h>
#include <UIScrollView.h>
#include <UITouch.h>
#include <algorithm>
#include <stdexcept>

namespace GlassDemoListDetail {
// Selection belongs to the list until a drag is established. This recognizer
// only retires row selection; it never prevents the enclosing scroll pan.
class SelectionDrag final : public NXKit::UIPanGestureRecognizer {
public:
    SelectionDrag(std::weak_ptr<NXKit::UIControl> row, std::weak_ptr<NXKit::UIScrollView> list)
        : _row(std::move(row)), _list(std::move(list)) {}
    bool canPrevent(const std::shared_ptr<NXKit::UIGestureRecognizer>&) const override { return false; }
    void touchesBegan(std::vector<std::shared_ptr<NXKit::UITouch>> touches, std::shared_ptr<NXKit::UIEvent> event) override {
        const auto row = _row.lock();
        const auto list = _list.lock();
        auto target = touches.empty() ? nullptr : touches.front()->view().lock();
        if (!row || !list || !row->isDescendantOf(list)) { setState(NXKit::UIGestureRecognizerState::failed); return; }
        while (target && target != row) {
            // Accessories own their own gestures and cancellation policy.
            if (std::dynamic_pointer_cast<NXKit::UIControl>(target)) { setState(NXKit::UIGestureRecognizerState::failed); return; }
            target = target->superview().lock();
        }
        if (!row || target != row) { setState(NXKit::UIGestureRecognizerState::failed); return; }
        UIPanGestureRecognizer::touchesBegan(std::move(touches), std::move(event));
    }
    void touchesMoved(std::vector<std::shared_ptr<NXKit::UITouch>> touches, std::shared_ptr<NXKit::UIEvent> event) override {
        const auto row = _row.lock();
        const auto list = _list.lock();
        if (!row || !list || !row->isDescendantOf(list)) { setState(NXKit::UIGestureRecognizerState::failed); return; }
        if (state() == NXKit::UIGestureRecognizerState::possible && row->isTracking()
            && (!list->canCancelContentTouches() || !list->touchesShouldCancelInContentView(row))) {
            setState(NXKit::UIGestureRecognizerState::failed);
            return;
        }
        UIPanGestureRecognizer::touchesMoved(std::move(touches), std::move(event));
    }
private:
    std::weak_ptr<NXKit::UIControl> _row;
    std::weak_ptr<NXKit::UIScrollView> _list;
};
}

// Demo lists deliberately use selectable UIControls in place of UITableView
// cells. Keep that list-specific cancellation policy here; UIScrollView's
// public default continues to protect an ordinary admitted UIControl.
class GlassDemoListView : public NXKit::UIScrollView {
public:
    using UIScrollView::UIScrollView;
    void registerSelectableRow(const std::shared_ptr<NXKit::UIControl>& row) {
        if (!row) throw std::invalid_argument("Selectable row cannot be null");
        std::erase_if(_rows, [](const auto& weak) { return weak.expired(); });
        if (containsRow(row)) return;
        _rows.push_back(row);
        row->addGestureRecognizer(NXKit::new_shared<GlassDemoListDetail::SelectionDrag>(row, weak_from_base<NXKit::UIScrollView>()));
    }
    bool touchesShouldCancelInContentView(std::shared_ptr<NXKit::UIView> view) override {
        auto ancestor = view;
        while (ancestor && ancestor.get() != this) {
            if (const auto control = std::dynamic_pointer_cast<NXKit::UIControl>(ancestor))
                return containsRow(control) || UIScrollView::touchesShouldCancelInContentView(control);
            ancestor = ancestor->superview().lock();
        }
        return UIScrollView::touchesShouldCancelInContentView(std::move(view));
    }
private:
    std::vector<std::weak_ptr<NXKit::UIControl>> _rows;
    bool containsRow(const std::shared_ptr<NXKit::UIControl>& row) const {
        return std::any_of(_rows.begin(), _rows.end(), [&](const auto& weak) { return weak.lock() == row; });
    }
};
