#include <CATextLayer.h>
#include <modules/skunicode/include/SkUnicode_icu.h>
#include <tools/Tools.hpp>

using namespace NXKit;
using namespace skia::textlayout;

CATextLayer::CATextLayer(): CALayer() {
    unicode = SkUnicodes::ICU::Make();

    updateParagraph();
}

CATextLayer::CATextLayer(CATextLayer* layer): CALayer(layer) {
    _fontSize = layer->_fontSize;
    _textColor = layer->_textColor;
    _fontWeight = layer->_fontWeight;
    _textAlignment = layer->_textAlignment;
    _text = layer->_text;

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
    onWillSet("fontSize");
    _fontSize = fontSize;
    updateParagraph();
}

void CATextLayer::setFontWeight(NXFloat fontWidth) {
    if (_fontWeight == fontWidth) return;
    onWillSet("fontWidth");
    _fontWeight = fontWidth;
    updateParagraph();
}

void CATextLayer::setTextAlignment(NSTextAlignment textAlignment) {
    if (_textAlignment == textAlignment) return;
    _textAlignment = textAlignment;
    updateParagraph();
}

void CATextLayer::updateParagraph() {
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
    style.setFontStyle(SkFontStyle(_fontWeight, SkFontStyle::kNormal_Width,
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

std::optional<AnimatableProperty> CATextLayer::value(std::string forKeyPath) {
    if (forKeyPath == "textColor") return _textColor;
    if (forKeyPath == "fontSize") return _fontSize;
    if (forKeyPath == "fontWidth") return _fontWeight;
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

    if (keyPath == "fontSize") {
        auto start = any_optional_cast<NXFloat>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXFloat>(animation->toValue);
        if (!end.has_value()) end = this->_fontSize;

        std::static_pointer_cast<CATextLayer>(presentation)->setFontSize(start.value() + (end.value() - start.value()) * progress);
    }

    if (keyPath == "fontWidth") {
        auto start = any_optional_cast<NXFloat>(fromValue);
        if (!start.has_value()) { return; }

        auto end = any_optional_cast<NXFloat>(animation->toValue);
        if (!end.has_value()) end = this->_fontWeight;

        std::static_pointer_cast<CATextLayer>(presentation)->setFontWeight(start.value() + (end.value() - start.value()) * progress);
    }

    CALayer::update(presentation, animation, progress);
}
