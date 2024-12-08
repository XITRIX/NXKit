#pragma once

namespace NXKit {

class NXColor {
public:
    NXColor();
    NXColor(unsigned char red, unsigned char green, unsigned char blue);
    NXColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();

private:
    friend class NXLayer;
    int color;
};

}