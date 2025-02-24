#pragma once

#include <UITraitCollection.h>
#include <Geometry.h>
#include <cstdint>
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
    UIColor(UIColor const &color);
    explicit UIColor(const std::function<UIColor(std::shared_ptr<UITraitCollection>)>& dynamicProvider);
    explicit UIColor(int rawValue);
    UIColor(unsigned char red, unsigned char green, unsigned char blue);
    UIColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

    [[nodiscard]] unsigned char r() const;
    [[nodiscard]] unsigned char g() const;
    [[nodiscard]] unsigned char b() const;
    [[nodiscard]] unsigned char a() const;
    
    bool operator==(const UIColor& rhs) const;
    bool operator!=(const UIColor& rhs) const;

    UIColor withAlphaComponent(NXFloat alpha);

    [[nodiscard]] UIColor interpolationTo(const UIColor& endResult, NXFloat progress) const;
    [[nodiscard]] uint32_t raw() const;

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

    // Label
    static UIColor label;
    static UIColor secondaryLabel;
    static UIColor tertiaryLabel;
    static UIColor quaternaryLabel;

    // Text
    static UIColor placeholderText;

    // Fill
    static UIColor systemFill;
    static UIColor secondarySystemFill;
    static UIColor tertiarySystemFill;
    static UIColor quaternarySystemFill;

    // Separator
    static UIColor separator;
    static UIColor opaqueSeparator;
    
    // Standard content background
    static UIColor systemBackground;
    static UIColor secondarySystemBackground;
    static UIColor tertiarySystemBackground;

    // Grouped content background
    static UIColor systemGroupedBackground;
    static UIColor secondarySystemGroupedBackground;
    static UIColor tertiarySystemGroupedBackground;

    // Tint
    static UIColor tint;

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

    // Adoptable gray
    static UIColor systemGray;
    static UIColor systemGray2;
    static UIColor systemGray3;
    static UIColor systemGray4;
    static UIColor systemGray5;
    static UIColor systemGray6;

private:
    friend class UIView;

    static UIColor _currentTint;
    std::optional<std::function<UIColor(std::shared_ptr<UITraitCollection>)>> dynamicProvider;
    uint32_t color = 0xFF600060;
};

}
