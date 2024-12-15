#pragma once

#include <UIView.h>
#include <include/core/SkTypeface.h>

namespace NXKit {

class UILabel: public UIView {
public:
    UILabel();
private:
    int _numberOfLines = 1;
    std::string _text;
    UIColor _textColor = UIColor::black;
};

}
