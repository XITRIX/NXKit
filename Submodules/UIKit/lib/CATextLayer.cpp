#include <CATextLayer.h>
#include <modules/skunicode/include/SkUnicode_icu.h>

using namespace NXKit;
using namespace skia::textlayout;

CATextLayer::CATextLayer(): CALayer() {
    auto fontMgr = SkiaCtx::main()->getFontMgr();

    SkFontStyle fontStyle;
    typeface = SkiaCtx::main()->getFontMgr()->matchFamilyStyle(nullptr, fontStyle);

    fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(SkiaCtx::main()->getFontMgr());

    unicode = SkUnicodes::ICU::Make();

    paragraphBuilder = ParagraphBuilder::make(paraStyle, fontCollection, unicode);
    updateParagraph();
}

CATextLayer::CATextLayer(CATextLayer* layer): CALayer(layer) {
    typeface = layer->typeface;
    paraStyle = layer->paraStyle;
    fontCollection = layer->fontCollection;
    unicode = layer->unicode;

    paragraphBuilder = ParagraphBuilder::make(paraStyle, fontCollection, unicode);
    updateParagraph();
}

std::shared_ptr<CALayer> CATextLayer::copy() {
    return new_shared<CATextLayer>(this);
}

void CATextLayer::draw(SkCanvas* context) {
    paragraph->layout(bounds().size.width);
    paragraph->paint(context, 0, 0);
}

void CATextLayer::setText(std::string text) {
    if (_text == text) return;
    _text = text;
    updateParagraph();
}

void CATextLayer::setTextColor(UIColor textColor) {
    if (_textColor == textColor) return;
    _textColor = textColor;
    updateParagraph();
}

void CATextLayer::updateParagraph() {
    paragraphBuilder->Reset();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(_textColor.raw());

    skia::textlayout::TextStyle style;
    style.setForegroundColor(paint);
    style.setTypeface(typeface);
    style.setFontSize(17);

    paraStyle.setTextStyle(style);
    paraStyle.setTextAlign(TextAlign::kRight);

    paragraphBuilder->addText(_text.c_str());

    paragraph = paragraphBuilder->Build();
}
