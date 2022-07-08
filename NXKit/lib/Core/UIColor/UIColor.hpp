//
//  UIColor.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include "nanovg.h"

struct UIColor {
public:
    UIColor(short r, short g, short b, short a);
    UIColor(short r, short g, short b): UIColor(r, g, b, 255) {}

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();

    NVGcolor raw();
private:
    int value;
};
