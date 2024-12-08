#include "NXColor.h"

using namespace NXKit;

NXColor::NXColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    color = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}
NXColor::NXColor(unsigned char r, unsigned char g, unsigned char b): NXColor(r, g, b, 255) {}
NXColor::NXColor(): NXColor(0, 0, 0) {}


unsigned char NXColor::r() {
    return static_cast<unsigned char>((color >> 16) & 0xff);
}

unsigned char NXColor::g() {
    return static_cast<unsigned char>((color >> 8) & 0xff);
}

unsigned char NXColor::b() {
    return static_cast<unsigned char>(color & 0xff);
}

unsigned char NXColor::a() {
    return static_cast<unsigned char>((color >> 24) & 0xff);
}