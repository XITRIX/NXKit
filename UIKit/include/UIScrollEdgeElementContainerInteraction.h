#pragma once
#include <UIInteraction.h>
#include <UIRectEdge.h>
#include <memory>
namespace NXKit {
class UIScrollView;
/** Identifies controls that overlay a scroll edge. The interaction and scroll
    references are weak; the containing UIView owns the interaction. */
class UIScrollEdgeElementContainerInteraction final:public UIInteraction {
public:
    [[nodiscard]] std::weak_ptr<UIView> view() const override { return _view; }
    [[nodiscard]] std::shared_ptr<UIScrollView> scrollView() const { return _scroll.lock(); }
    void setScrollView(const std::shared_ptr<UIScrollView>& scrollView);
    [[nodiscard]] UIRectEdge edge() const { return _edge; }
    void setEdge(UIRectEdge edge);
    void willMoveToView(const std::shared_ptr<UIView>&) override;
    void didMoveToView(const std::shared_ptr<UIView>&) override;
private:
    std::weak_ptr<UIView> _view;
    std::weak_ptr<UIScrollView> _scroll;
    UIRectEdge _edge=UIRectEdge::none;
    void invalidate();
};
}
