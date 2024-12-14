#pragma once

namespace NXKit {

class UIColor {
public:
    UIColor();
    UIColor(unsigned char red, unsigned char green, unsigned char blue);
    UIColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();
    
    bool operator==(const UIColor& rhs) const;

    static UIColor clear;
    static UIColor red;
    static UIColor green;
    static UIColor blue;
    static UIColor orange;
    static UIColor cyan;
    static UIColor white;
    static UIColor black;
    static UIColor gray;

    static UIColor separator;
    static UIColor systemBackground;
    static UIColor secondarySystemBackground;
    static UIColor tetriarySystemBackground;
    static UIColor tint;

private:
    friend class CALayer;
    int color;
};

}
