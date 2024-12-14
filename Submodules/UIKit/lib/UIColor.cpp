#include "UIColor.h"

using namespace NXKit;

UIColor UIColor::clear = UIColor(0, 0, 0, 0);
UIColor UIColor::red = UIColor(255, 0, 0);
UIColor UIColor::green = UIColor(0, 255, 0);
UIColor UIColor::blue = UIColor(0, 0, 255);
UIColor UIColor::orange = UIColor(255, 150, 0);
UIColor UIColor::cyan = UIColor(0, 150, 255);
UIColor UIColor::white = UIColor(255, 255, 255);
UIColor UIColor::black = UIColor(0, 0, 0);
UIColor UIColor::gray = UIColor(155, 155, 155);

UIColor UIColor::separator = UIColor(208, 208, 208);
UIColor UIColor::systemBackground = UIColor(235, 235, 235);
UIColor UIColor::secondarySystemBackground = UIColor(240, 240, 240);
UIColor UIColor::tetriarySystemBackground = UIColor(252, 255, 248);
UIColor UIColor::tint = UIColor(49, 79, 235);

UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    color = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}
UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b): UIColor(r, g, b, 255) {}
UIColor::UIColor(): UIColor(0, 0, 0) {}


unsigned char UIColor::r() {
    return static_cast<unsigned char>((color >> 16) & 0xff);
}

unsigned char UIColor::g() {
    return static_cast<unsigned char>((color >> 8) & 0xff);
}

unsigned char UIColor::b() {
    return static_cast<unsigned char>(color & 0xff);
}

unsigned char UIColor::a() {
    return static_cast<unsigned char>((color >> 24) & 0xff);
}

bool UIColor::operator==(const UIColor& rhs) const {
    return this->color == rhs.color;
}
