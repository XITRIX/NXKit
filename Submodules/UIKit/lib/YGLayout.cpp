//
//  YGLayout.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 10.03.2023.
//

#include <YGLayout.h>
#include <UIView.h>

namespace NXKit {

YGSize YGLayout::YGMeasureView(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
    const float constrainedWidth = (widthMode == YGMeasureModeUndefined) ? MAXFLOAT : width;
    const float constrainedHeight = (heightMode == YGMeasureModeUndefined) ? MAXFLOAT: height;

    UIView* view = (UIView*) YGNodeGetContext(node);
    NXSize sizeThatFits = NXSize();

    // The default implementation of sizeThatFits: returns the existing size of
    // the view. That means that if we want to layout an empty UIView, which
    // already has got a frame set, its measured size should be CGSizeZero, but
    // UIKit returns the existing size.
    //
    // See https://github.com/facebook/yoga/issues/606 for more information.
    if (!view->_yoga->isUIView || view->subviews().size() > 0) {
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

void YGLayout::YGAttachNodesFromViewHierachy(std::shared_ptr<UIView> view)
{
  auto yoga = view->_yoga;
  const YGNodeRef node = yoga->_node;

  // Only leaf nodes should have a measure function
  if (yoga->isLeaf()) {
    YGRemoveAllChildren(node);
    YGNodeSetMeasureFunc(node, YGMeasureView);
  } else {
    YGNodeSetMeasureFunc(node, NULL);

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
    if (node == NULL) {
        return;
    }

    YGNodeRemoveAllChildren(node);
}

YGLayout::YGLayout(std::shared_ptr<UIView> view):
    _view(view), _node(YGNodeNew())
{
    YGNodeSetContext(_node, view.get());
    _isEnabled = false;

    // In case we've changed UIView's sizeToFit implementation, there is no need in this check anymore
//    isUIView = strcmp(typeid(view.get()).name(), typeid(UIView*).name()) == 0;
    isUIView = false;
}

YGLayout::~YGLayout() {
    YGNodeFree(_node);
}

void YGLayout::layoutIfNeeded() {
    if (!isEnabled() || !isRoot()) return;
    applyLayoutPreservingOrigin(false);
}

void YGLayout::applyLayoutPreservingOrigin(bool preserveOrigin) {
    calculateLayoutWithSize(_view.lock()->bounds().size);
    YGApplyLayoutToViewHierarchy(_view.lock(), preserveOrigin);
}

void YGLayout::YGApplyLayoutToViewHierarchy(std::shared_ptr<UIView>view, bool preserveOrigin) {
    auto yoga = view->_yoga;

    // We don't need it cause if it is not included in layout
    // it still could contains it's own layout that needs to be calculated
//    if (!yoga->_isIncludedInLayout) { return; }

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
    view->setFrame( NXRect(
      NXPoint (
        YGRoundPixelValue(topLeft.x + origin.x),
        YGRoundPixelValue(topLeft.y + origin.y)
      ),
      NXSize (
        YGRoundPixelValue(bottomRight.x) - YGRoundPixelValue(topLeft.x),
        YGRoundPixelValue(bottomRight.y) - YGRoundPixelValue(topLeft.y)
      )
    ));

    if (!yoga->isLeaf()) {
        for (int i = 0; i < view->subviews().size(); i++) {
            YGApplyLayoutToViewHierarchy(view->subviews()[i], false);
        }
    }
}

float YGLayout::YGRoundPixelValue(float value) {
    static float scale = UITraitCollection::current()->displayScale(); // UIRenderer::main()->scale();
    return roundf(value * scale) / scale;
}

NXSize YGLayout::calculateLayoutWithSize(NXSize size) {
    YGAttachNodesFromViewHierachy(_view.lock());

    const YGNodeRef node = _node;
    YGNodeCalculateLayout(
        node,
        size.width,
        size.height,
        YGNodeStyleGetDirection(node));

    return NXSize(
        YGNodeLayoutGetWidth(node),
        YGNodeLayoutGetHeight(node)
    );
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

}
