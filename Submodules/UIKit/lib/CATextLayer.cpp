#include <CATextLayer.h>
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <tools/Tools.hpp>

using namespace NXKit;
using namespace skia::textlayout;

CATextLayer::CATextLayer(): CALayer() {
    SkFontStyle fontStyle;
    typeface = SkiaCtx::main()->getFontMgr()->matchFamilyStyle(nullptr, fontStyle);
    unicode = SkUnicodes::ICU::Make();

    updateParagraph();
}

CATextLayer::CATextLayer(CATextLayer* layer): CALayer(layer) {
    _fontSize = layer->_fontSize;
    _textColor = layer->_textColor;
    _text = layer->_text;

    typeface = layer->typeface;
    unicode = layer->unicode;
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
    onWillSet("textColor");
    _textColor = textColor;
    updateParagraph();
}

void CATextLayer::setFontSize(NXFloat fontSize) {
    if (_fontSize == fontSize) return;
    _fontSize = fontSize;
    updateParagraph();
}

void CATextLayer::updateParagraph() {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(SkiaCtx::main()->getFontMgr());

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(_textColor.raw());

    skia::textlayout::TextStyle style;
    style.setForegroundColor(paint);
    style.setTypeface(typeface);
    style.setFontSize(_fontSize);

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setTextAlign(TextAlign::kRight);

    auto paragraphBuilder = ParagraphBuilder::make(paraStyle, fontCollection, unicode);

    paragraphBuilder->addText(_text.c_str());
    paragraph = paragraphBuilder->Build();
}

std::optional<AnimatableProperty> CATextLayer::value(std::string forKeyPath) {
    if (forKeyPath == "textColor") return _textColor;
    return CALayer::value(forKeyPath);
}

void CATextLayer::update(std::shared_ptr<CALayer> presentation, std::shared_ptr<CABasicAnimation> animation, float progress) {
    if (!animation->keyPath.has_value() || !animation->fromValue.has_value()) return;

    auto keyPath = animation->keyPath.value();
    auto fromValue = animation->fromValue.value();

    if (keyPath == "textColor") {
        auto start = any_optional_cast<UIColor>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<UIColor>(animation->toValue);
        if (!end.has_value()) end = this->_textColor;
        if (!end.has_value()) end = UIColor::clear;

        std::static_pointer_cast<CATextLayer>(presentation)->setTextColor(start->interpolationTo(end.value(), progress));
    }

    CALayer::update(presentation, animation, progress);
}
