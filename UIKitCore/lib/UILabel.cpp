#include <UILabel.h>
#include "TextRendering.hpp"
#include <tools/IBTools.h>
#include <include/core/SkBitmap.h>
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <cmath>
#include <stdexcept>
#include <vector>

using namespace NXKit;
using namespace skia::textlayout;

namespace {

struct ParagraphMetrics {
    NXFloat width = 0;
    NXFloat height = 0;
    size_t lines = 0;
};

ParagraphMetrics measureParagraph(const std::unique_ptr<Paragraph>& paragraph, NXFloat width, NXFloat lineHeight) {
    paragraph->layout(width);

    ParagraphMetrics result;
    result.width = std::max<NXFloat>(0, paragraph->getLongestLine());
    // Font-wide fTop/fBottom include extrema of unrelated glyphs. UIKit
    // centers the typographic line box, not that font bounding rectangle.
    std::vector<LineMetrics> lines;
    paragraph->getLineMetrics(lines);
    result.lines = lines.size();
    result.height = lineHeight > 0 ? lineHeight * result.lines : paragraph->getHeight();
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
    invalidateIntrinsicContentSize();
}

void UILabel::setTextColor(const UIColor& textColor) {
    if (_textColor == textColor) return;
    _textColor = textColor;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setFontSize(NXFloat fontSize) {
    if (!std::isfinite(fontSize) || fontSize <= 0)
        throw std::invalid_argument("UILabel font size must be finite and positive");
    if (_fontSize == fontSize) return;
    _fontSize = fontSize;
    setNeedsDisplay();
    invalidateIntrinsicContentSize();
}

void UILabel::setTextAlignment(NSTextAlignment textAlignment) {
    if (_textAlignment == textAlignment) return;
    _textAlignment = textAlignment;
    setNeedsDisplay();
    setNeedsLayout();
}

void UILabel::setFontWeight(NXFloat fontWeight) {
    if (!std::isfinite(fontWeight))
        throw std::invalid_argument("UILabel font weight must be finite");
    if (_fontWeight == fontWeight) return;
    _fontWeight = fontWeight;
    setNeedsDisplay();
    invalidateIntrinsicContentSize();
}

void UILabel::setScaleModifier(NXFloat scaleModifier) {
    if (layer()->scaleModifier() == scaleModifier) return;
    layer()->setScaleModifier(scaleModifier);
    setNeedsDisplay();
}

NXSize UILabel::sizeThatFits(NXSize size) {
    if (_text.empty()) return {};
    updateParagraph();
    auto metrics = measureParagraph(paragraph, size.width, _lineHeight);

    const auto traits = traitCollection();
    const auto scale = traits ? std::max<NXFloat>(1, traits->displayScale()) : 1;
    return { std::ceil(metrics.width * scale) / scale,
             std::ceil(metrics.height * scale) / scale };
}

void UILabel::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UIView::traitCollectionDidChange(previousTraitCollection);
    if (!previousTraitCollection || previousTraitCollection->displayScale() != traitCollection()->displayScale())
        invalidateIntrinsicContentSize();
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
    const auto bitmapWidth = scaledBitmapDimension(size.width, scale);
    const auto bitmapHeight = scaledBitmapDimension(size.height, scale);
#if defined(PLATFORM_SWITCH)
    // Dawn's GLES compatibility path has a portable RGBA8 upload format,
    // while Skia's native N32 color type is BGRA on this target.
    const auto bitmapInfo = SkImageInfo::Make(bitmapWidth, bitmapHeight,
                                              kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
#else
    const auto bitmapInfo = SkImageInfo::MakeN32Premul(bitmapWidth, bitmapHeight);
#endif
    bitmap.allocPixels(bitmapInfo);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
#ifdef __APPLE__
    const SkSurfaceProps textProps(0, kUnknown_SkPixelGeometry, 0, 1);
#else
    const SkSurfaceProps textProps;
#endif
    SkCanvas canvas(bitmap, textProps);

    canvas.scale(scale, scale);
    auto metrics = measureParagraph(paragraph, size.width, _lineHeight);
    // UILabel rounds its typographic block, centered origin and baselines to
    // display pixels independently. SkParagraph instead rounds baselines to
    // whole points, which is visibly different on Retina displays.
    const auto blockHeight = std::ceil(metrics.height * scale) / scale;
    auto yOffset = std::round((size.height - blockHeight) * scale / 2) / scale;
    std::vector<SkScalar> baselines;
    if (_lineHeight > 0) for (size_t i=0; i<metrics.lines; ++i)
        baselines.push_back(std::round((_ascender+i*_lineHeight) * scale) / scale);

    Internal::paintPlainText(*paragraph, &canvas, 0, yOffset, _textColor.raw(), baselines);

    layer()->setContents(new_shared<CGImage>(bitmap.asImage()));
    layer()->setContentsScale(scale);
}

void UILabel::updateParagraph() {
    SkFontStyle fontStyle(static_cast<int>(_fontWeight), SkFontStyle::kNormal_Width,
                          SkFontStyle::kUpright_Slant);
    auto skiaContext = SkiaCtx::main();
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(_textColor.raw());

    skia::textlayout::TextStyle style;
    style.setForegroundColor(paint);
    style.setFontSize(_fontSize);
//    style.setHeight(_fontSize);
    style.setFontStyle(fontStyle);

    auto fontCollection = Internal::plainTextFonts(skiaContext, fontStyle, _fontSize, style);
    _ascender = _lineHeight = 0;
    if (auto face = style.refTypeface()) {
        SkFontMetrics metrics;
        SkFont(face, _fontSize).getMetrics(&metrics);
        _ascender = -metrics.fAscent;
        _lineHeight = metrics.fDescent - metrics.fAscent + metrics.fLeading;
    }
    ParagraphStyle paraStyle;
    // SkParagraph's Flutter compatibility mode floors layout width to whole
    // points. A label fitted to a Retina-pixel width can then wrap despite
    // having enough room. Preserve fractional UIKit layout coordinates.
    paraStyle.setApplyRoundingHack(false);
    if (_lineHeight > 0) {
        // UILabel retains its primary font's line box when fallback glyphs
        // (such as emoji) have taller font-wide bounds.
        StrutStyle strut;
        strut.setFontFamilies(style.getFontFamilies());
        strut.setFontStyle(fontStyle);
        strut.setFontSize(_fontSize);
        strut.setStrutEnabled(true);
        strut.setForceStrutHeight(true);
        paraStyle.setStrutStyle(strut);
    }
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
    REGISTER_XIB_ATTRIBUTE(fontWeight, valueToFloat, setFontWeight)
    REGISTER_XIB_ATTRIBUTE(textAlignment, valueToTextAlignment, setTextAlignment)

//    if (name == "font") {
//        auto fontPath = valueToPath(value);
//        if (!fontPath.has_value()) return false;
//        setFont(new_shared<UIFont>(fontPath.value(), this->font()->pointSize));
//        return true;
//    }

    return false;
}
