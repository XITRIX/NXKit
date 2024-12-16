#pragma once

#include <UIView.h>
#include <CATextLayer.h>
#include <include/core/SkTypeface.h>

namespace NXKit {

class UILabel: public UIView {
public:
    UILabel();

    void setText(std::string text) { _textLayer()->setText(text); }
    [[nodiscard]] std::string text() const { return _textLayer()->text(); }

    void setTextColor(UIColor textColor) { _textLayer()->setTextColor(textColor); }
    [[nodiscard]] UIColor textColor() const { return _textLayer()->textColor(); }

    void setFontSize(NXFloat fontSize) { _textLayer()->setFontSize(fontSize); }
    [[nodiscard]] NXFloat fontSize() const { return _textLayer()->fontSize(); }

    void setTextAlignment(NSTextAlignment textAlignment) { _textLayer()->setTextAlignment(textAlignment); }
    [[nodiscard]] NSTextAlignment textAlignment() const { return _textLayer()->textAlignment(); }

    void setFontWeight(NXFloat fontWeight) { _textLayer()->setFontWeight(fontWeight); }
    [[nodiscard]] NXFloat fontWeight() const { return _textLayer()->fontWeight(); }

    NXSize sizeThatFits(NXSize size) override;

private:
    int _numberOfLines = 1;
    std::shared_ptr<CATextLayer> _textLayer() const;
};

}
