#pragma once

#include <UIView.h>

namespace NXKit {

class UILabel: public UIView {
public:
private:
    int _numberOfLines = 1;
    std::string _text;
    UIColor _textColor = UIColor::black;
};

}