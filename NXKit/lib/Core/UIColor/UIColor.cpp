//
//  UIColor.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include "UIColor.hpp"

namespace NXKit {

UIColor UIColor::clear = UIColor(0, 0, 0, 0);
UIColor UIColor::red = UIColor(255, 0, 0);
UIColor UIColor::green = UIColor(0, 255, 0);
UIColor UIColor::blue = UIColor(0, 0, 255);
UIColor UIColor::white = UIColor(255, 255, 255);
UIColor UIColor::black = UIColor(0, 0, 0);

UIColor::UIColor(short r, short g, short b, short a) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}

unsigned char UIColor::r() {
    return static_cast<unsigned char>((value >> 16) & 0xff);
}

unsigned char UIColor::g() {
    return static_cast<unsigned char>((value >> 8) & 0xff);
}

unsigned char UIColor::b() {
    return static_cast<unsigned char>(value & 0xff);
}

unsigned char UIColor::a() {
    return static_cast<unsigned char>((value >> 24) & 0xff);
}

NVGcolor UIColor::raw() {
    return nvgRGBA(r(), g(), b(), a());
}

}
