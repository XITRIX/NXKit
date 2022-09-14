//
//  UIColor.hpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

#include <nanovg.h>
#include <Core/UITraitCollection/UITraitCollection.hpp>

#include <deque>

namespace NXKit {

struct UIColor {
public:
    UIColor(): UIColor(0, 0, 0, 0) {}
    UIColor(short r, short g, short b, short a);
    UIColor(short r, short g, short b, short a, short dr, short dg, short db, short da);
    UIColor(short r, short g, short b, short dr, short dg, short db): UIColor(r, g, b, 255, dr, dg, db, 255) {}
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

    void fillAnimationContext(std::deque<float>* context);
    static UIColor fromAnimationContext(std::deque<float>* context);

    static UIColor clear;
    static UIColor red;
    static UIColor green;
    static UIColor blue;
    static UIColor white;
    static UIColor black;
    static UIColor gray;

    static UIColor separator;
    static UIColor systemBackground;
    static UIColor secondarySystemBackground;
    static UIColor tetriarySystemBackground;
    static UIColor systemTint;

    static UIColor label;
    static UIColor invertLabel;
private:
    int value;
    int darkValue;
    bool darkInited = false;
};

}
