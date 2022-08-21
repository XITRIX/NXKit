//
//  UILabel.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UILabel/UILabel.hpp>
#include <Core/Application/Application.hpp>
#include <Core/Utils/FontManager/FontManager.hpp>

namespace NXKit {

#define ELLIPSIS "\u2026"

void UILabel::computeLabelHeight(UILabel* label, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode, YGSize* size, float* originalBounds) {
    label->isWrapping = false;

    float requiredHeight = originalBounds[3] - originalBounds[1] + 5;
    if (heightMode == YGMeasureModeUndefined || heightMode == YGMeasureModeAtMost)
    {
        // Grow the label vertically as much as possible
        if (heightMode == YGMeasureModeAtMost)
            size->height = std::min(requiredHeight, height);
        else
            size->height = requiredHeight;
    }
    else if (heightMode == YGMeasureModeExactly)
    {
        size->height = height;
    }
    else
    {
        //        fatal("Unsupported Label height measure mode: " + std::to_string(heightMode));
    }
}

YGSize UILabel::labelMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
    NVGcontext* vg       = Application::shared()->getVideoContext()->getNVGContext();
    UILabel* label         = (UILabel*)YGNodeGetContext(node);
    std::string fullText = label->getText();

    YGSize size = {
        .width  = width,
        .height = height,
    };

    if (fullText == "")
        return size;

    // XXX: workaround for a Yoga bug
    if (widthMode == YGMeasureModeAtMost && (width == 0 || std::isnan(width)))
    {
        widthMode = YGMeasureModeUndefined;
        width     = NAN;
    }

    // Setup nvg state for the measurements
    nvgFontSize(vg, label->font.fontSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, label->font.fontId);
    nvgTextLineHeight(vg, label->font.lineHeight);

    // Measure the needed width for the ellipsis
    float bounds[4]; // width = xmax - xmin + some padding because nvgTextBounds isn't super precise
    nvgTextBounds(vg, 0, 0, ELLIPSIS, nullptr, bounds);
    label->ellipsisWidth = bounds[2] - bounds[0];

    // Measure the needed width for the fullText
    nvgTextBounds(vg, 0, 0, fullText.c_str(), nullptr, bounds);
    float requiredWidth = bounds[2] - bounds[0];
    label->requiredWidth = requiredWidth;

    // XXX: This is an approximation since the given width here may not match the actual final width of the view
    float availableWidth = std::isnan(width) ? std::numeric_limits<float>::max() : width;

    // Width
    if (widthMode == YGMeasureModeUndefined || widthMode == YGMeasureModeAtMost)
    {
        // Grow the label horizontally as much as possible
        if (widthMode == YGMeasureModeAtMost)
            size.width = std::min(requiredWidth, availableWidth);
        else
            size.width = requiredWidth;
    }
    else if (widthMode == YGMeasureModeExactly)
    {
        size.width = width;
    }
    else
    {
        //        fatal("Unsupported Label width measure mode: " + std::to_string(widthMode));
    }

    // Height
    // Measure the required height, with wrapping

    // Is wrapping necessary and allowed ?
    if ((availableWidth < requiredWidth || fullText.find("\n") != std::string::npos) && !label->isSingleLine())
    {
        float boxBounds[4];
        nvgTextBoxBounds(vg, 0, 0, availableWidth, fullText.c_str(), nullptr, boxBounds);

        float requiredHeight = boxBounds[3] - boxBounds[1];

        // Undefined height mode, always wrap
        if (heightMode == YGMeasureModeUndefined)
        {
            label->isWrapping = true;
            size.height = requiredHeight;
        }
        // At most height mode, see if we have enough space
        else if (heightMode == YGMeasureModeAtMost)
        {
            if (height >= requiredHeight)
            {
                label->isWrapping = true;
                size.height = requiredHeight;
            }
            else
            {
                size.height = height;
                //                computeLabelHeight(label, width, widthMode, height, heightMode, &size, bounds);
            }
        }
        // Exactly mode, see if we have enough space
        else if (heightMode == YGMeasureModeExactly)
        {
            if (height >= requiredHeight)
            {
                label->isWrapping = true;
                size.height = height;
            }
            else
            {
                computeLabelHeight(label, width, widthMode, height, heightMode, &size, bounds);
            }
        }
        else
        {
            //            fatal("Unsupported Label height measure mode: " + std::to_string(heightMode));
        }
    }
    // No wrapping necessary or allowed, return the normal height
    else
    {
        computeLabelHeight(label, width, widthMode, height, heightMode, &size, bounds);
    }

    return size;
}

static std::string trim(const std::string& str)
{
    std::string result = "";
    size_t endIndex    = str.size();
    while (endIndex > 0 && std::isblank(str[endIndex - 1]))
        endIndex -= 1;
    for (size_t i = 0; i < endIndex; i += 1)
    {
        char ch = str[i];
        if (!isblank(ch) || result.size() > 0)
            result += ch;
    }
    return result;
}

UILabel::UILabel(std::string text) : UIView(),
text(text),
textColor(0, 0, 0),
shadowColor(0, 0, 0)
{
    font = UIFont {
        .fontId = FontManager::shared()->getPrimaryFont(),
        .fontSize = 17,
        .lineHeight = 1.65f
    };
    
    YGNodeSetMeasureFunc(this->ygNode, labelMeasureFunc);
}

void UILabel::draw(NVGcontext *vg) {
    int x = 0;
    int y = 0;
    int width = getFrame().size.width;
    int height = getFrame().size.height;


    if (width == 0)
        return;

    enum NVGalign horizAlign = this->getNVGHorizontalAlign();
    enum NVGalign vertAlign  = this->getNVGVerticalAlign();

    nvgFontSize(vg, this->font.fontSize);
    nvgTextAlign(vg, horizAlign | vertAlign);
    nvgFontFaceId(vg, this->font.fontId);
    nvgTextLineHeight(vg, this->font.lineHeight);
    nvgFillColor(vg, this->textColor.raw());

    // Wrapped text
    if (this->isWrapping) {
        nvgTextAlign(vg, horizAlign | NVG_ALIGN_TOP);
        nvgTextBox(vg, x, y, width, this->text.c_str(), nullptr);
    }
    // Truncated text
    else {
        float textX = x;
        float textY = y;

        if (horizAlign == NVG_ALIGN_CENTER)
            textX += width / 2;
        else if (horizAlign == NVG_ALIGN_RIGHT)
            textX += width;

        if (vertAlign == NVG_ALIGN_MIDDLE || vertAlign == NVG_ALIGN_BASELINE)
            textY += height / 2.0f;
        else if (vertAlign == NVG_ALIGN_BOTTOM)
            textY += height;

        nvgText(vg, textX, textY, this->truncatedText.c_str(), nullptr);
    }
}

void UILabel::layoutSubviews() {
    UIView::layoutSubviews();

    float width = getFrame().size.width;

    if (width == 0)
    {
        //        this->resetScrollingAnimation();
        return;
    }

    // Prebake clipping
    if (width < this->requiredWidth && !this->isWrapping)
    {
        // Compute the position of the ellipsis (in chars), should the string be truncated
        // Use an approximation based on the text width and ellipsis width
        // Cannot do it in the measure function because the margins are not applied yet there
        float toRemove      = std::min(this->requiredWidth, this->requiredWidth - width + this->ellipsisWidth * 1.5f); // little bit more than ellipsis width to make sure it doesn't overflow
        float toRemoveRatio = toRemove / requiredWidth;

        size_t ellipsisPosition = this->text.size() - roundf((float)this->text.size() * toRemoveRatio);
        this->truncatedText     = trim(this->text.substr(0, ellipsisPosition)) + ELLIPSIS;
    }
    else
    {
        this->truncatedText = this->text;
    }
}

void UILabel::setText(std::string text) {
    this->text = text;
    setNeedsLayout();
}

std::string UILabel::getText() {
    return text;
}

void UILabel::setFont(UIFont font) {
    font = font;
    setNeedsLayout();
}

UIFont* UILabel::getFont() {
    return &font;
}

void UILabel::setSingleLine(bool singleLine) {
    this->singleLine = singleLine;
    setNeedsLayout();
}

bool UILabel::isSingleLine() {
    return singleLine;
}

enum NVGalign UILabel::getNVGHorizontalAlign()
{
    switch (this->horizontalAlign) {
        case HorizontalAlign::LEFT:
            return NVG_ALIGN_LEFT;
        case HorizontalAlign::CENTER:
            return NVG_ALIGN_CENTER;
        case HorizontalAlign::RIGHT:
            return NVG_ALIGN_RIGHT;
    }
}

enum NVGalign UILabel::getNVGVerticalAlign()
{
    switch (this->verticalAlign)
    {
        default:
        case VerticalAlign::BASELINE:
            return NVG_ALIGN_BASELINE;
        case VerticalAlign::TOP:
            return NVG_ALIGN_TOP;
        case VerticalAlign::CENTER:
            return NVG_ALIGN_MIDDLE;
        case VerticalAlign::BOTTOM:
            return NVG_ALIGN_BOTTOM;
    }
}

}
