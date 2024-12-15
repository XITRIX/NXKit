#pragma once

#include <CALayer.h>
#include <SkiaCtx.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>

namespace NXKit {

class CATextLayer: public CALayer {
public:
    CATextLayer();
    CATextLayer(CATextLayer* layer);
    virtual ~CATextLayer() = default;

    void draw(SkCanvas* context) override;
    std::shared_ptr<CALayer> copy() override;

    void setText(std::string text);
    [[nodiscard]] std::string text() const { return _text; }

    void setTextColor(UIColor textColor);
    [[nodiscard]] UIColor textColor() const { return _textColor; }
private:
    std::string _text = "Furthermore, العربية نص جميل. द क्विक ब्राउन फ़ॉक्स jumps over the lazy 🐕.";
    UIColor _textColor = UIColor::black;

    // Skia
    sk_sp<SkTypeface> typeface;
    skia::textlayout::ParagraphStyle paraStyle;
    sk_sp<skia::textlayout::FontCollection> fontCollection;
    sk_sp<SkUnicode> unicode;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> paragraphBuilder;
    std::unique_ptr<skia::textlayout::Paragraph> paragraph;

    void updateParagraph();
};

}
