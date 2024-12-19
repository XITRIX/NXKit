#pragma once

#include <UITraitCollection.h>
#include <Geometry.h>
#include <stdint.h>
#include <functional>
#include <optional>

namespace NXKit {

#define UIColorThemed(lightColor, darkColor)                               \
UIColor([](auto collection) {\
if (collection->userInterfaceStyle() == UIUserInterfaceStyle::dark) \
    return darkColor;                                               \
else                                                                \
    return lightColor;\
})

class UIColor {
public:
    UIColor();
    UIColor(std::function<UIColor(std::shared_ptr<UITraitCollection>)> dynamicProvider);
    UIColor(int rawValue);
    UIColor(unsigned char red, unsigned char green, unsigned char blue);
    UIColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

    unsigned char r();
    unsigned char g();
    unsigned char b();
    unsigned char a();
    
    bool operator==(const UIColor& rhs) const;

    UIColor interpolationTo(UIColor endResult, NXFloat progress);
    uint32_t raw() const;

    // Transparent
    static UIColor clear;

    // Fixed
    static UIColor black;
    static UIColor darkGray;
    static UIColor gray;
    static UIColor lightGray;
    static UIColor white;
    static UIColor red;
    static UIColor orange;
    static UIColor yellow;
    static UIColor green;
    static UIColor cyan;
    static UIColor blue;
    static UIColor purple;
    static UIColor magenta;
    static UIColor brown;

    // Adaptable
    static UIColor systemRed;
    static UIColor systemOrange;
    static UIColor systemYellow;
    static UIColor systemGreen;
    static UIColor systemMint;
    static UIColor systemTeal;
    static UIColor systemCyan;
    static UIColor systemBlue;
    static UIColor systemIndigo;
    static UIColor systemPurple;
    static UIColor systemPink;
    static UIColor systemBrown;

    // Label
    static UIColor label;
    static UIColor secondaryLabel;
    static UIColor tertiaryLabel;
    static UIColor quaternaryLabel;

    // Separator
    static UIColor separator;
    
    // Standard content background
    static UIColor systemBackground;
    static UIColor secondarySystemBackground;
    static UIColor tetriarySystemBackground;

    static UIColor tint;

private:
    std::optional<std::function<UIColor(std::shared_ptr<UITraitCollection>)>> dynamicProvider;
    uint32_t color = 0xFF600060;
};

}
