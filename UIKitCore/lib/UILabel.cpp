#include <UILabel.h>
#include <tools/IBTools.h>
#include <include/core/SkBitmap.h>
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <cmath>
#include <limits>
#include <vector>

using namespace NXKit;
using namespace skia::textlayout;

namespace {

struct ParagraphVisualBounds {
    NXFloat width = 0;
    NXFloat top = 0;
    NXFloat height = 0;
};

ParagraphVisualBounds measureParagraph(const std::unique_ptr<Paragraph>& paragraph, NXFloat width) {
    paragraph->layout(width);

    ParagraphVisualBounds result;
    result.width = paragraph->getLongestLine();

    std::vector<LineMetrics> lines;
    paragraph->getLineMetrics(lines);
    if (lines.empty()) {
        result.height = paragraph->getHeight();
        return result;
    }

    NXFloat top = std::numeric_limits<NXFloat>::infinity();
    NXFloat bottom = -std::numeric_limits<NXFloat>::infinity();

    for (const auto& line : lines) {
        NXFloat lineTop = static_cast<NXFloat>(line.fBaseline - line.fAscent);
        NXFloat lineBottom = static_cast<NXFloat>(line.fBaseline + line.fDescent);

        for (const auto& [_, styleMetrics] : line.fLineMetrics) {
            const auto& fontMetrics = styleMetrics.font_metrics;
            if (fontMetrics.hasBounds()) {
                lineTop = std::min(lineTop, static_cast<NXFloat>(line.fBaseline + fontMetrics.fTop));
                lineBottom = std::max(lineBottom, static_cast<NXFloat>(line.fBaseline + fontMetrics.fBottom));
            } else {
                lineTop = std::min(lineTop, static_cast<NXFloat>(line.fBaseline + fontMetrics.fAscent));
                lineBottom = std::max(lineBottom, static_cast<NXFloat>(line.fBaseline + fontMetrics.fDescent));
            }
        }

        top = std::min(top, lineTop);
        bottom = std::max(bottom, lineBottom);
    }

    result.top = std::floor(top);
    result.height = std::max<NXFloat>(0, std::ceil(bottom) - result.top);
    return result;
}

int scaledBitmapDimension(NXFloat logicalDimension, NXFloat scale) {
    return std::max(1, static_cast<int>(std::ceil(std::max<NXFloat>(0, logicalDimension) * scale)));
}

}

UILabel::UILabel(): UIView() {
    unicode = SkUnicodes::ICU::Make();
}

void UILabel::setText(const std::string& text) {
    if (_text == text) return;
    _text = text;
    setNeedsDisplay();
    setNeedsLayout();
    printf("Test set %s\n", text.c_str());
}

void UILabel::setTextColor(const UIColor& textColor) {
    if (_textColor == textColor) return;
    _textColor = textColor;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setFontSize(NXFloat fontSize) {
    if (_fontSize == fontSize) return;
    _fontSize = fontSize;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setTextAlignment(NSTextAlignment textAlignment) {
    if (_textAlignment == textAlignment) return;
    _textAlignment = textAlignment;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setFontWeight(NXFloat fontWeight) {
    if (_fontWeight == fontWeight) return;
    _fontWeight = fontWeight;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setScaleModifier(NXFloat scaleModifier) {
    if (layer()->scaleModifier() == scaleModifier) return;
    layer()->setScaleModifier(scaleModifier);
    setNeedsDisplay();
}

NXSize UILabel::sizeThatFits(NXSize size) {
    updateParagraph();
    auto metrics = measureParagraph(paragraph, size.width);

    // Adds extra 1, because of extra content scale could be not enough to fit text line
    auto rWidth = std::ceil(metrics.width) + 1;
    return { rWidth, metrics.height };
}

void UILabel::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UIView::traitCollectionDidChange(previousTraitCollection);
    setNeedsDisplay();
}

void UILabel::draw() {
    updateParagraph();

    SkBitmap bitmap;
    NXFloat scale;
    NXSize size;
    if (contentMode() == UIViewContentMode::redraw) {
        scale = traitCollection()->displayScale() * layer()->presentationOrSelf()->scaleModifier();
        size = layer()->presentationOrSelf()->bounds().size;
    } else {
        scale = traitCollection()->displayScale() * layer()->scaleModifier();
        size = bounds().size;
    }
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(scaledBitmapDimension(size.width, scale),
                                                  scaledBitmapDimension(size.height, scale)));
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bitmap);

    canvas.scale(scale, scale);
    auto metrics = measureParagraph(paragraph, size.width);
    auto yOffset = (size.height - metrics.height) / 2 - metrics.top;

    paragraph->paint(&canvas, 0, yOffset);

    layer()->setContents(new_shared<CGImage>(bitmap.asImage()));
    layer()->setContentsScale(scale);
}

void UILabel::updateParagraph() {
    SkFontStyle fontStyle;
    auto typeface = SkiaCtx::main()->getFontMgr()->matchFamilyStyle(nullptr, fontStyle);

    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(SkiaCtx::main()->getFontMgr());

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(_textColor.raw());

    skia::textlayout::TextStyle style;
    style.setForegroundColor(paint);
    style.setTypeface(typeface);
    style.setFontSize(_fontSize);
//    style.setHeight(_fontSize);
    style.setFontStyle(SkFontStyle((int) _fontWeight, SkFontStyle::kNormal_Width,
                                    SkFontStyle::kUpright_Slant));

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    switch (_textAlignment) {
        case NSTextAlignment::left:
            paraStyle.setTextAlign(TextAlign::kLeft);
            break;
        case NSTextAlignment::right:
            paraStyle.setTextAlign(TextAlign::kRight);
            break;
        case NSTextAlignment::center:
            paraStyle.setTextAlign(TextAlign::kCenter);
            break;
        case NSTextAlignment::justified:
            paraStyle.setTextAlign(TextAlign::kJustify);
            break;
        case NSTextAlignment::natural:
            paraStyle.setTextAlign(TextAlign::kStart);
            break;
    }

    auto paragraphBuilder = ParagraphBuilder::make(paraStyle, fontCollection, unicode);

    paragraphBuilder->addText(_text.c_str());
    paragraph = paragraphBuilder->Build();
}

bool UILabel::applyXMLAttribute(const std::string& name, const std::string& value) {
    if (UIView::applyXMLAttribute(name, value)) return true;

    REGISTER_XIB_ATTRIBUTE(text, valueToString, setText)
    REGISTER_XIB_ATTRIBUTE(textColor, valueToColor, setTextColor)
    REGISTER_XIB_ATTRIBUTE(fontSize, valueToFloat, setFontSize)
    REGISTER_XIB_ATTRIBUTE(textAlignment, valueToTextAlignment, setTextAlignment)

//    if (name == "font") {
//        auto fontPath = valueToPath(value);
//        if (!fontPath.has_value()) return false;
//        setFont(new_shared<UIFont>(fontPath.value(), this->font()->pointSize));
//        return true;
//    }

    return false;
}
