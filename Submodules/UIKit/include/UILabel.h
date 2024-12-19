#pragma once

#include <UIView.h>
#include <CATextLayer.h>
#include <include/core/SkTypeface.h>

namespace NXKit {

class UILabel: public UIView {
public:
    UILabel();

    void draw() override;

    void setText(std::string text);
    [[nodiscard]] std::string text() const { return _text; }

    void setTextColor(UIColor textColor);
    [[nodiscard]] UIColor textColor() const { return _textColor; }

    void setFontSize(NXFloat fontSize);
    [[nodiscard]] NXFloat fontSize() const { return _fontSize; }

    void setTextAlignment(NSTextAlignment textAlignment);
    [[nodiscard]] NSTextAlignment textAlignment() const { return _textAlignment; }

    void setFontWeight(NXFloat fontWeight);
    [[nodiscard]] NXFloat fontWeight() const { return _fontWeight; }

    NXSize sizeThatFits(NXSize size) override;
    void traitCollectionDidChange(std::shared_ptr<UITraitCollection> previousTraitCollection) override;

private:
    int _numberOfLines = 1;
    NXFloat _fontSize = 17;
    NXFloat _fontWeight = SkFontStyle::kNormal_Weight;
    NSTextAlignment _textAlignment = NSTextAlignment::left;
    std::string _text = "Furthermore, العربية نص جميل. द क्विक ब्राउन फ़ॉक्स jumps over the lazy 🐕.";
    UIColor _textColor = UIColor::label;

    sk_sp<SkUnicode> unicode;
    std::unique_ptr<skia::textlayout::Paragraph> paragraph;

    void updateParagraph();
};

}
