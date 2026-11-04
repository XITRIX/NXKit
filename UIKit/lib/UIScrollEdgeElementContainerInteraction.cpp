#include <UIScrollEdgeElementContainerInteraction.h>
#include <UIScrollView.h>
using namespace NXKit;
void UIScrollEdgeElementContainerInteraction::invalidate(){if(const auto view=_view.lock()){view->setNeedsLayout();view->layer()->setNeedsDisplay();}if(const auto scroll=_scroll.lock())scroll->layer()->setNeedsDisplay();}
void UIScrollEdgeElementContainerInteraction::setScrollView(const std::shared_ptr<UIScrollView>& scroll){if(_scroll.lock()==scroll)return;invalidate();_scroll=scroll;invalidate();}
void UIScrollEdgeElementContainerInteraction::setEdge(UIRectEdge edge){if(_edge==edge)return;_edge=edge;invalidate();}
void UIScrollEdgeElementContainerInteraction::willMoveToView(const std::shared_ptr<UIView>&){invalidate();}
void UIScrollEdgeElementContainerInteraction::didMoveToView(const std::shared_ptr<UIView>& view){_view=view;invalidate();}
