#pragma once

#include <UIView.h>
#include <CATextLayer.h>
#include <include/core/SkTypeface.h>

namespace NXKit {

class UILabel: public UIView {
public:
    UILabel();

    void setTextColor(UIColor textColor);
    [[nodiscard]] UIColor textColor();

private:
    int _numberOfLines = 1;
    std::string _text;

    std::shared_ptr<CATextLayer> _textLayer();
};

}
