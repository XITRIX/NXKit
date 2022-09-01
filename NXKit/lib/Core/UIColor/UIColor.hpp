//
//  UIColor.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include <nanovg.h>

namespace NXKit {

struct UIColor {
public:
    UIColor(): UIColor(0, 0, 0, 0) {}
    UIColor(short r, short g, short b, short a);
    UIColor(short r, short g, short b): UIColor(r, g, b, 255) {}

    static UIColor f(float r, float g, float b, float a = 1);

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();

    UIColor withAlphaComponent(float alpha);
    UIColor withAlphaComponent(short alpha);

    NVGcolor raw();

    bool operator==(const UIColor& rhs);

    static UIColor clear;
    static UIColor red;
    static UIColor green;
    static UIColor blue;
    static UIColor white;
    static UIColor black;
    static UIColor gray;
    static UIColor separator;
private:
    int value;
};

}
