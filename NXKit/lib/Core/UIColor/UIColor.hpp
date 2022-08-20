//
//  UIColor.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "nanovg.h"

namespace NXKit {

struct UIColor {
public:
    UIColor(short r, short g, short b, short a);
    UIColor(short r, short g, short b): UIColor(r, g, b, 255) {}

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();

    NVGcolor raw();

    bool operator==(const UIColor& rhs);

    static UIColor clear;
    static UIColor red;
    static UIColor green;
    static UIColor blue;
    static UIColor white;
    static UIColor black;
private:
    int value;
};

}
