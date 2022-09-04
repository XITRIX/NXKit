//
//  UIColor.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UIColor/UIColor.hpp>

namespace NXKit {

UIColor UIColor::clear = UIColor(0, 0, 0, 0);
UIColor UIColor::red = UIColor(255, 0, 0);
UIColor UIColor::green = UIColor(0, 255, 0);
UIColor UIColor::blue = UIColor(0, 0, 255);
UIColor UIColor::white = UIColor(255, 255, 255);
UIColor UIColor::black = UIColor(0, 0, 0);
UIColor UIColor::gray = UIColor(155, 155, 155);
UIColor UIColor::separator = UIColor(208, 208, 208);

UIColor UIColor::systemBlue = UIColor(49, 79, 235);

UIColor::UIColor(short r, short g, short b, short a) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}

UIColor UIColor::f(float r, float g, float b, float a) {
    return UIColor(r * 255, g * 255, b * 255, a * 255);
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

UIColor UIColor::withAlphaComponent(float alpha) {
    return UIColor(r(), g(), b(), a() * alpha);
}

UIColor UIColor::withAlphaComponent(short alpha) {
    return UIColor(r(), g(), b(), a() * (alpha / 255));
}

NVGcolor UIColor::raw() {
    return nvgRGBA(r(), g(), b(), a());
}

bool UIColor::operator==(const UIColor& rhs) {
    return value == rhs.value;
}

}
