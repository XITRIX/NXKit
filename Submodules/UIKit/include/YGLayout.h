//
//  YGLayout.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 10.03.2023.
//

#pragma once

#include <yoga/Yoga.h>
#include <Geometry.h>
#include <memory>
#include <vector>
#include <cassert>

namespace NXKit {

#define YG_PROPERTY(type, lowercased_name, capitalized_name)    \
type lowercased_name()                                          \
{                                                               \
    return YGNodeStyleGet##capitalized_name(_node);             \
}                                                               \
                                                                \
void set##capitalized_name(type lowercased_name)                \
{                                                               \
    YGNodeStyleSet##capitalized_name(_node, lowercased_name);   \
}

#define YG_VALUE_PROPERTY(lowercased_name, capitalized_name)                       \
YGValue lowercased_name()                                                          \
{                                                                                  \
  return YGNodeStyleGet##capitalized_name(_node);                                  \
}

#define YG_AUTO_VALUE_PROPERTY(lowercased_name, capitalized_name)                  \
YGValue lowercased_name()                                                          \
{                                                                                  \
  return YGNodeStyleGet##capitalized_name(_node);                                  \
}                                                                                  \
                                                                                   \
void set##capitalized_name(YGValue lowercased_name)                                \
{                                                                                  \
  switch (lowercased_name.unit) {                                                  \
    case YGUnitPoint:                                                              \
      YGNodeStyleSet##capitalized_name(_node, lowercased_name.value);              \
      break;                                                                       \
    case YGUnitPercent:                                                            \
      YGNodeStyleSet##capitalized_name##Percent(_node, lowercased_name.value);     \
      break;                                                                       \
    case YGUnitAuto:                                                               \
      YGNodeStyleSet##capitalized_name##Auto(_node);                               \
      break;                                                                       \
    default:                                                                       \
      assert(false && "Not implemented");                                          \
  }                                                                                \
}

#define YG_EDGE_PROPERTY_GETTER(type, lowercased_name, capitalized_name, property, edge) \
type lowercased_name()                                                                   \
{                                                                                        \
    return YGNodeStyleGet##property(_node, edge);                                        \
}

#define YG_EDGE_PROPERTY_SETTER(lowercased_name, capitalized_name, property, edge) \
void set##capitalized_name(float lowercased_name)                                  \
{                                                                                  \
    YGNodeStyleSet##property(_node, edge, lowercased_name);                        \
}

#define YG_EDGE_PROPERTY(lowercased_name, capitalized_name, property, edge)         \
YG_EDGE_PROPERTY_GETTER(float, lowercased_name, capitalized_name, property, edge) \
YG_EDGE_PROPERTY_SETTER(lowercased_name, capitalized_name, property, edge)

#define YG_VALUE_EDGE_PROPERTY_SETTER(objc_lowercased_name, objc_capitalized_name, c_name, edge) \
void set##objc_capitalized_name(YGValue objc_lowercased_name)                                    \
{                                                                                                \
  switch (objc_lowercased_name.unit) {                                                           \
    case YGUnitUndefined:                                                                        \
      YGNodeStyleSet##c_name(_node, edge, objc_lowercased_name.value);                           \
      break;                                                                                     \
    case YGUnitPoint:                                                                            \
      YGNodeStyleSet##c_name(_node, edge, objc_lowercased_name.value);                           \
      break;                                                                                     \
    case YGUnitPercent:                                                                          \
      YGNodeStyleSet##c_name##Percent(_node, edge, objc_lowercased_name.value);                  \
      break;                                                                                     \
    default:                                                                                     \
      assert(false && "Not implemented");                                                        \
  }                                                                                              \
}

#define YG_VALUE_EDGE_PROPERTY(lowercased_name, capitalized_name, property, edge)   \
YG_EDGE_PROPERTY_GETTER(YGValue, lowercased_name, capitalized_name, property, edge) \
YG_VALUE_EDGE_PROPERTY_SETTER(lowercased_name, capitalized_name, property, edge)

#define YG_VALUE_EDGES_PROPERTIES(lowercased_name, capitalized_name)                                                  \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Left, capitalized_name##Left, capitalized_name, YGEdgeLeft)                   \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Top, capitalized_name##Top, capitalized_name, YGEdgeTop)                      \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Right, capitalized_name##Right, capitalized_name, YGEdgeRight)                \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Bottom, capitalized_name##Bottom, capitalized_name, YGEdgeBottom)             \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Start, capitalized_name##Start, capitalized_name, YGEdgeStart)                \
YG_VALUE_EDGE_PROPERTY(lowercased_name##End, capitalized_name##End, capitalized_name, YGEdgeEnd)                      \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Horizontal, capitalized_name##Horizontal, capitalized_name, YGEdgeHorizontal) \
YG_VALUE_EDGE_PROPERTY(lowercased_name##Vertical, capitalized_name##Vertical, capitalized_name, YGEdgeVertical)       \
YG_VALUE_EDGE_PROPERTY(lowercased_name, capitalized_name, capitalized_name, YGEdgeAll)

#define YG_GUTTER_PROPERTY_GETTER(type, lowercased_name, capitalized_name, property, gutter) \
type lowercased_name()                                                                   \
{                                                                                        \
    return YGNodeStyleGet##property(_node, gutter);                                        \
}

#define YG_VALUE_GUTTER_PROPERTY_SETTER(objc_lowercased_name, objc_capitalized_name, c_name, gutter) \
void set##objc_capitalized_name(float objc_lowercased_name)                                    \
{                                                                                                \
    YGNodeStyleSet##c_name(_node, gutter, objc_lowercased_name);                                 \
}

#define YG_VALUE_GUTTER_PROPERTY(lowercased_name, capitalized_name, property, gutter)   \
YG_GUTTER_PROPERTY_GETTER(YGValue, lowercased_name, capitalized_name, property, gutter) \
YG_VALUE_GUTTER_PROPERTY_SETTER(lowercased_name, capitalized_name, property, gutter)

class UIView;
class YGLayout {
public:
    bool isEnabled() { return _isEnabled; }
    void setEnabled(bool enabled) {
        _isEnabled = enabled;
    }

    bool isIncludedInLayout() { return _isIncludedInLayout; }
    void setIncludedInLayout(bool isIncludedInLayout) { _isIncludedInLayout = isIncludedInLayout; }

    YG_PROPERTY(YGDirection, direction, Direction)
    YG_PROPERTY(YGFlexDirection, flexDirection, FlexDirection)
    YG_PROPERTY(YGJustify, justifyContent, JustifyContent)
    YG_PROPERTY(YGAlign, alignContent, AlignContent)
    YG_PROPERTY(YGAlign, alignItems, AlignItems)
    YG_PROPERTY(YGAlign, alignSelf, AlignSelf)
    YG_PROPERTY(YGPositionType, position, PositionType)
    YG_PROPERTY(YGWrap, flexWrap, FlexWrap)
    YG_PROPERTY(YGOverflow, overflow, Overflow)
    YG_PROPERTY(YGDisplay, display, Display)

    YG_PROPERTY(float, flex, Flex)
    YG_PROPERTY(float, flexGrow, FlexGrow)
    YG_PROPERTY(float, flexShrink, FlexShrink)
    YG_AUTO_VALUE_PROPERTY(flexBasis, FlexBasis)

    YG_VALUE_GUTTER_PROPERTY(columnGap, ColumnGap, Gap, YGGutterColumn)
    YG_VALUE_GUTTER_PROPERTY(rowGap, RowGap, Gap, YGGutterRow)
    YG_VALUE_GUTTER_PROPERTY(allGap, AllGap, Gap, YGGutterAll)
    
    YG_VALUE_EDGE_PROPERTY(left, Left, Position, YGEdgeLeft)
    YG_VALUE_EDGE_PROPERTY(top, Top, Position, YGEdgeTop)
    YG_VALUE_EDGE_PROPERTY(right, Right, Position, YGEdgeRight)
    YG_VALUE_EDGE_PROPERTY(bottom, Bottom, Position, YGEdgeBottom)
    YG_VALUE_EDGE_PROPERTY(start, Start, Position, YGEdgeStart)
    YG_VALUE_EDGE_PROPERTY(end, End, Position, YGEdgeEnd)
    YG_VALUE_EDGES_PROPERTIES(margin, Margin)
    YG_VALUE_EDGES_PROPERTIES(padding, Padding)

    YG_EDGE_PROPERTY(borderLeftWidth, BorderLeftWidth, Border, YGEdgeLeft)
    YG_EDGE_PROPERTY(borderTopWidth, BorderTopWidth, Border, YGEdgeTop)
    YG_EDGE_PROPERTY(borderRightWidth, BorderRightWidth, Border, YGEdgeRight)
    YG_EDGE_PROPERTY(borderBottomWidth, BorderBottomWidth, Border, YGEdgeBottom)
    YG_EDGE_PROPERTY(borderStartWidth, BorderStartWidth, Border, YGEdgeStart)
    YG_EDGE_PROPERTY(borderEndWidth, BorderEndWidth, Border, YGEdgeEnd)
    YG_EDGE_PROPERTY(borderWidth, BorderWidth, Border, YGEdgeAll)

    YG_AUTO_VALUE_PROPERTY(width, Width)
    YG_AUTO_VALUE_PROPERTY(height, Height)
    YG_VALUE_PROPERTY(minWidth, MinWidth)
    YG_VALUE_PROPERTY(minHeight, MinHeight)
    YG_VALUE_PROPERTY(maxWidth, MaxWidth)
    YG_VALUE_PROPERTY(maxHeight, MaxHeight)
    YG_PROPERTY(float, aspectRatio, AspectRatio)

    void setPosition(NXPoint position) {
        setLeft(YGValue{static_cast<float>(position.x), YGUnitPoint});
        setTop(YGValue{static_cast<float>(position.y), YGUnitPoint});
    }

    void setSize(NXSize size) {
        setWidth(YGValue{static_cast<float>(size.width), YGUnitPoint});
        setHeight(YGValue{static_cast<float>(size.height), YGUnitPoint});
    }

    YGLayout(std::shared_ptr<UIView> view);
    ~YGLayout();

    void layoutIfNeeded();
    void applyLayoutPreservingOrigin(bool preserveOrigin);
    NXSize calculateLayoutWithSize(NXSize size);

    bool isLeaf();
    bool isRoot();

    bool isDirty();
    void markDirty();

private:
    YGNodeRef _node = nullptr;
    std::weak_ptr<UIView> _view;
    bool _isEnabled = false;
    bool _isIncludedInLayout = true;

    bool isUIView;
    
    static void YGAttachNodesFromViewHierachy(std::shared_ptr<UIView> view);
    static void YGRemoveAllChildren(const YGNodeRef node);
    static YGSize YGMeasureView(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode);
    static float YGSanitizeMeasurement(float constrainedSize, float measuredSize, YGMeasureMode measureMode);
    static bool YGNodeHasExactSameChildren(const YGNodeRef node, std::vector<std::shared_ptr<UIView>> subviews);
    static void YGApplyLayoutToViewHierarchy(std::shared_ptr<UIView>view, bool preserveOrigin);
    static float YGRoundPixelValue(float value);

    friend class UIView;
};

namespace yoga {
namespace literals {

inline YGValue operator"" _pt(long double value) {
  return YGValue{static_cast<float>(value), YGUnitPoint};
}
inline YGValue operator"" _pt(unsigned long long value) {
  return operator"" _pt(static_cast<long double>(value));
}

inline YGValue operator"" _percent(long double value) {
  return YGValue{static_cast<float>(value), YGUnitPercent};
}
inline YGValue operator"" _percent(unsigned long long value) {
  return operator"" _percent(static_cast<long double>(value));
}

} // namespace literals
} // namespace yoga

}
