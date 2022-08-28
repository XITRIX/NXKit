//
//  UILabel.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include <Core/UIView/UIView.hpp>
#include <string>

namespace NXKit {

enum class HorizontalAlign {
    LEFT,
    CENTER,
    RIGHT,
};

enum class VerticalAlign
{
    BASELINE,
    TOP,
    CENTER,
    BOTTOM,
};

struct UIFont {
    int fontId;
    float fontSize;
    float lineHeight;
};

class UILabel: public UIView {
public:
    UILabel(std::string text);
    UILabel(): UILabel("") {}

    UIColor textColor;


    float shadowRadius = 0;
    UIColor shadowColor;

    HorizontalAlign horizontalAlign = HorizontalAlign::LEFT;
    VerticalAlign verticalAlign     = VerticalAlign::TOP;

    void draw(NVGcontext *vgContext) override;
    void layoutSubviews() override;

    std::string getText();
    void setText(std::string text);

    UIFont* getFont();
    void setFont(UIFont font);

    bool isSingleLine();
    void setSingleLine(bool singleLine);
private:
    UIFont font;
    std::string text = "";
    std::string truncatedText = "";
    bool singleLine = false;
    bool isWrapping = false;
    float ellipsisWidth = 0;
    float requiredWidth = 0;

    enum NVGalign getNVGHorizontalAlign();
    enum NVGalign getNVGVerticalAlign();

    static void computeLabelHeight(UILabel* label, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode, YGSize* size, float* originalBounds);
    static YGSize labelMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode);
};

}
