//
//  UIStackView.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 05.07.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>

namespace NXKit {

enum class Axis {
    HORIZONTAL,
    VERTICAL,
};

enum class JustifyContent
{
    FLEX_START,
    CENTER,
    FLEX_END,
    SPACE_BETWEEN,
    SPACE_AROUND,
    SPACE_EVENLY,
};

enum class AlignItems
{
    AUTO,
    FLEX_START,
    CENTER,
    FLEX_END,
    STRETCH,
    BASELINE,
    SPACE_BETWEEN,
    SPACE_AROUND,
};

class UIStackView: public UIView {
public:
    UIStackView(Axis axis);
    UIStackView(Rect frame);
    UIStackView(): UIStackView(Axis::VERTICAL) {}

    void addSubview(std::shared_ptr<UIView> view) override;
    std::shared_ptr<UIView> getNextFocus(NavigationDirection direction) override;
    std::shared_ptr<UIView> getDefaultFocus() override;

    std::shared_ptr<UIView> hitTest(Point point, UIEvent *withEvent) override;

    void subviewFocusDidChange(std::shared_ptr<UIView> focusedView, std::shared_ptr<UIView> notifiedView) override;

    void setAxis(Axis axis);
    Axis getAxis() { return axis; }

    void setJustifyContent(JustifyContent justify);
    void setAlignItems(AlignItems alignment);

    void setPadding(float top, float right, float bottom, float left);
    void setPaddingTop(float top);
    void setPaddingLeft(float left);
    void setPaddingRight(float right);
    void setPaddingBottom(float bottom);

    bool passthroughTouches = false;
private:
    friend class UISelectorViewController;

    Axis axis = Axis::VERTICAL;
    int currentFocus = 0;
};

}
