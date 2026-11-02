//
//  YGLayout.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 10.03.2023.
//

#include <YGLayout.h>
#include <UIView.h>
#include <UIViewController.h>
#include <cmath>

namespace NXKit {

YGSize YGLayout::YGMeasureView(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
    const float constrainedWidth = (widthMode == YGMeasureModeUndefined) ? MAXFLOAT : width;
    const float constrainedHeight = (heightMode == YGMeasureModeUndefined) ? MAXFLOAT: height;

    auto* view = (UIView*) YGNodeGetContext(node);
    NXSize sizeThatFits = NXSize();

    // The default implementation of sizeThatFits: returns the existing size of
    // the view. That means that if we want to layout an empty UIView, which
    // already has got a frame set, its measured size should be CGSizeZero, but
    // UIKit returns the existing size.
    //
    // See https://github.com/facebook/yoga/issues/606 for more information.
    if (!view->_yoga->isUIView || !view->subviews().empty()) {
      sizeThatFits = view->sizeThatFits(NXSize(
                                            constrainedWidth,
                                            constrainedHeight
                                        ));
    }

    
    return (YGSize) {
      .width = YGSanitizeMeasurement(constrainedWidth, sizeThatFits.width, widthMode),
      .height = YGSanitizeMeasurement(constrainedHeight, sizeThatFits.height, heightMode),
    };
}

float YGLayout::YGSanitizeMeasurement(float constrainedSize, float measuredSize, YGMeasureMode measureMode) {
    float result;
    if (measureMode == YGMeasureModeExactly) {
      result = constrainedSize;
    } else if (measureMode == YGMeasureModeAtMost) {
      result = fminf(constrainedSize, measuredSize);
    } else {
      result = measuredSize;
    }

    return result;
}

void YGLayout::YGAttachNodesFromViewHierachy(const std::shared_ptr<UIView>& view)
{
  auto yoga = view->_yoga;
  const YGNodeRef node = yoga->_node;
  const auto traits = view->traitCollection() ? view->traitCollection() : UITraitCollection::current();
  const auto scale = traits ? traits->displayScale() : 1.f;
  YGConfigSetPointScaleFactor(yoga->_config, std::isfinite(scale) && scale > 0 ? scale : 1.f);

  // Only leaf nodes should have a measure function
  if (yoga->isLeaf()) {
    YGRemoveAllChildren(node);
    YGNodeSetMeasureFunc(node, YGMeasureView);
  } else {
    YGNodeSetMeasureFunc(node, nullptr);

    std::vector<std::shared_ptr<UIView>> subviewsToInclude;
    for (auto& subview: view->subviews()) {
        if (subview->_yoga->_isEnabled && subview->_yoga->_isIncludedInLayout) {
            subviewsToInclude.push_back(subview);
        }
    }

    if (!YGNodeHasExactSameChildren(node, subviewsToInclude)) {
        YGRemoveAllChildren(node);
        for (int i = 0; i < subviewsToInclude.size(); i++) {
            YGNodeInsertChild(node, subviewsToInclude[i]->_yoga->_node, i);
        }
    }

    for (auto& subview: subviewsToInclude) {
        YGAttachNodesFromViewHierachy(subview);
    }
  }
}

bool YGLayout::YGNodeHasExactSameChildren(const YGNodeRef node, std::vector<std::shared_ptr<UIView>> subviews) {
    if (YGNodeGetChildCount(node) != subviews.size()) {
        return false;
    }

    for (int i = 0; i < subviews.size(); i++) {
        if (YGNodeGetChild(node, i) != subviews[i]->_yoga->_node) {
            return false;
        }
    }

    return true;
}

void YGLayout::YGRemoveAllChildren(const YGNodeRef node) {
    if (node == nullptr) {
        return;
    }

    YGNodeRemoveAllChildren(node);
}

YGLayout::YGLayout(const std::shared_ptr<UIView>& view):
    _config(YGConfigNew()), _node(YGNodeNewWithConfig(_config)), _view(view)
{
    YGNodeSetContext(_node, view.get());
    _isEnabled = false;

    // In case we've changed UIView's sizeToFit implementation, there is no need in this check anymore
//    isUIView = strcmp(typeid(view.get()).name(), typeid(UIView*).name()) == 0;
    isUIView = false;
}

YGLayout::~YGLayout() {
    YGNodeFree(_node);
    YGConfigFree(_config);
}

void YGLayout::layoutIfNeeded() {
    if (!isEnabled() || !isRoot()) return;

    const auto view = _view.lock();
    if (!view) return;

    // A Yoga root can still be positioned by a non-Yoga superview, as happens
    // for a custom modal presentation. Yoga owns the root's size and its child
    // layout in that case, but the container owns the root's origin.
    applyLayoutPreservingOrigin(!view->superview().expired());
}

void YGLayout::applyLayoutPreservingOrigin(bool preserveOrigin) {
    calculateLayoutWithSize(_view.lock()->bounds().size);
    YGApplyLayoutToViewHierarchy(_view.lock(), preserveOrigin);
}

void YGLayout::YGApplyLayoutToViewHierarchy(const std::shared_ptr<UIView>&view, bool preserveOrigin) {
    auto yoga = view->_yoga;

    YGNodeRef node = yoga->_node;
    const NXPoint topLeft = {
      YGNodeLayoutGetLeft(node),
      YGNodeLayoutGetTop(node),
    };

    const NXPoint bottomRight = {
      topLeft.x + YGNodeLayoutGetWidth(node),
      topLeft.y + YGNodeLayoutGetHeight(node),
    };

    const NXPoint origin = preserveOrigin ? view->frame().origin : NXPoint();

    auto frame = NXRect(
      NXPoint (
        topLeft.x + origin.x,
        topLeft.y + origin.y
      ),
      NXSize (
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y
      )
    );

    // Apply measurements ignoring transformation matrix
    auto position = NXPoint(frame.origin.x + (frame.width() * view->layer()->anchorPoint().x),
                            frame.origin.y + (frame.height() * view->layer()->anchorPoint().y));

    if (view->layer()->position() != position) {
        view->layer()->setPosition(position);
        view->setNeedsDisplay();
        view->setNeedsUpdateSafeAreaInsets();
    }

    auto bounds = view->layer()->bounds();
    bounds.size = frame.size;

    // Keep transforms out of Yoga placement, but use UIView's bounds setter
    // so manual descendants, drawing and safe areas see this pass's size.
    if (view->bounds() != bounds) view->setBounds(bounds);

    if (!yoga->isLeaf()) {
        const auto children = view->subviews();
        for (const auto& i : children) {
            if (i->superview().lock() == view && i->_yoga->_isEnabled
                && i->_yoga->_isIncludedInLayout && YGNodeGetParent(i->_yoga->_node) == node)
                YGApplyLayoutToViewHierarchy(i, false);
        }
    }
}

NXSize YGLayout::calculateLayoutWithSize(NXSize size) {
    YGAttachNodesFromViewHierachy(_view.lock());

    const YGNodeRef node = _node;
    YGNodeCalculateLayout(
        node,
        size.width,
        size.height,
        YGNodeStyleGetDirection(node));

    return {
        YGNodeLayoutGetWidth(node),
        YGNodeLayoutGetHeight(node)
    };
}

void YGLayout::invalidateLayout() {
    auto view = _view.lock();
    if (!view) return;

    view->setNeedsLayout();
}

bool YGLayout::isLeaf() {
    if (_isEnabled) {
        for (auto& subview: _view.lock()->subviews()) {
            auto yoga = subview->_yoga;
            if (yoga->_isEnabled && yoga->_isIncludedInLayout) {
                return false;
            }
        }
    }

    return true;
}

bool YGLayout::isRoot() {
    if (_isEnabled) {
        return YGNodeGetParent(_node) == nullptr;
    }

    return false;
}

bool YGLayout::isDirty() {
    if (_isEnabled) {
        return YGNodeIsDirty(_node);
    }

    return false;
}

}
