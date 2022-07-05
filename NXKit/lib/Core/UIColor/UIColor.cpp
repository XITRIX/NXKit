//
//  UIColor.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UIColor.hpp"

UIColor::UIColor(short r, short g, short b, short a) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}

short UIColor::r() {
    return (value >> 16) & 0xff;
}

short UIColor::g() {
    return (value >> 8) & 0xff;
}

short UIColor::b() {
    return value & 0xff;
}

short UIColor::a() {
    return (value >> 24) & 0xff;
}

NVGcolor UIColor::raw() {
    return nvgRGBA(r(), g(), b(), a());
}
