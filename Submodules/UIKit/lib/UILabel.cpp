#include <UILabel.h>
#include <tools/IBTools.h>
#include <include/core/SkBitmap.h>
#include <modules/skunicode/include/SkUnicode_icu.h>

using namespace NXKit;
using namespace skia::textlayout;

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
    paragraph->layout(size.width);
    auto height = paragraph->getHeight();
    auto width = paragraph->getMaxIntrinsicWidth();

    // Adds extra 1, because of extra content scale could be not enough to fit text line
    auto rWidth = std::ceil(width) + 1;
    return { rWidth, height };
}

void UILabel::traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) {
    UIView::traitCollectionDidChange(previousTraitCollection);
    setNeedsLayout();
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
    auto bitmapSize = size * scale;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul((int) bitmapSize.width, (int) bitmapSize.height));
    SkCanvas canvas(bitmap);

    canvas.scale(scale, scale);
    paragraph->layout(size.width);

    auto height = paragraph->getHeight();
    auto yOffset = (size.height - height) / 2;

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
