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

UIColor UIColor::separator = UIColor(208, 208, 208, 81, 81, 81);
UIColor UIColor::systemBackground = UIColor(235, 235, 235, 45, 45, 45);
UIColor UIColor::secondarySystemBackground = UIColor(240, 240, 240, 50, 50, 50);
UIColor UIColor::tetriarySystemBackground = UIColor(252, 255, 248, 31, 34, 39);
UIColor UIColor::systemTint = UIColor(49, 79, 235, 0, 255, 204);

UIColor UIColor::label = UIColor(45, 45, 45, 255, 255, 255);
UIColor UIColor::invertLabel = UIColor(255, 255, 255, 52, 41, 55);

UIColor::UIColor(short r, short g, short b, short a) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}

UIColor::UIColor(short r, short g, short b, short a, short dr, short dg, short db, short da) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
    darkValue = (da & 0xff) << 24 | (dr & 0xff) << 16 | (dg & 0xff) << 8 | (db & 0xff);
    darkInited = true;
}

UIColor UIColor::f(float r, float g, float b, float a) {
    return UIColor(r * 255, g * 255, b * 255, a * 255);
}

bool currentStyleIsDark() {
    return UITraitCollection::current.userInterfaceStyle == UIUserInterfaceStyle::dark;
}

unsigned char UIColor::r() {
    if (darkInited && currentStyleIsDark()) {
        return static_cast<unsigned char>((darkValue >> 16) & 0xff);
    }
    return static_cast<unsigned char>((value >> 16) & 0xff);
}

unsigned char UIColor::g() {
    if (darkInited && currentStyleIsDark()) {
        return static_cast<unsigned char>((darkValue >> 8) & 0xff);
    }
    return static_cast<unsigned char>((value >> 8) & 0xff);
}

unsigned char UIColor::b() {
    if (darkInited && currentStyleIsDark()) {
        return static_cast<unsigned char>(darkValue & 0xff);
    }
    return static_cast<unsigned char>(value & 0xff);
}

unsigned char UIColor::a() {
    if (darkInited && currentStyleIsDark()) {
        return static_cast<unsigned char>((darkValue >> 24) & 0xff);
    }
    return static_cast<unsigned char>((value >> 24) & 0xff);
}

UIColor UIColor::withAlphaComponent(float alpha) {
//    UIColor color = *this;
//    color.value |= (int)(static_cast<unsigned char>((value >> 24) & 0xff) * alpha);
//    color.darkValue |= (int)(static_cast<unsigned char>((darkValue >> 24) & 0xff) * alpha);
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
