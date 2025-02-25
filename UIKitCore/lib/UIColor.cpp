#include "UIColor.h"
#include <limits>
#include <cmath>
#include <utility>

using namespace NXKit;

UIColor UIColor::_currentTint = UIColor::systemBlue;

// Transparent
UIColor UIColor::clear = UIColor(0, 0, 0, 0);

// Fixed
UIColor UIColor::black = UIColor(0, 0, 0, "black");
UIColor UIColor::darkGray = UIColor(85, 85, 85, "darkGray");
UIColor UIColor::gray = UIColor(128, 128, 128, "gray");
UIColor UIColor::lightGray = UIColor(170, 170, 170, "lightGray");
UIColor UIColor::white = UIColor(255, 255, 255, "white");
UIColor UIColor::red = UIColor(255, 0, 0, "red");
UIColor UIColor::orange = UIColor(255, 128, 0, "orange");
UIColor UIColor::yellow = UIColor(255, 255, 0, "yellow");
UIColor UIColor::green = UIColor(0, 255, 0, "green");
UIColor UIColor::cyan = UIColor(0, 255, 255, "cyan");
UIColor UIColor::blue = UIColor(0, 0, 255, "blue");
UIColor UIColor::purple = UIColor(128, 0, 128, "purple");
UIColor UIColor::magenta = UIColor(255, 0, 255, "magenta");
UIColor UIColor::brown = UIColor(153, 102, 51, "brown");

// Adaptable
UIColor UIColor::systemRed = UIColorThemed(UIColor(255, 59, 48), UIColor(255, 69, 58), "systemRed");
UIColor UIColor::systemOrange = UIColorThemed(UIColor(255, 149, 0), UIColor(255, 159, 10), "systemOrange");
UIColor UIColor::systemYellow = UIColorThemed(UIColor(242, 204, 0), UIColor(255, 214, 10), "systemYellow");
UIColor UIColor::systemGreen = UIColorThemed(UIColor(52, 199, 89), UIColor(48, 209, 88), "systemGreen");
UIColor UIColor::systemMint = UIColorThemed(UIColor(0, 199, 190), UIColor(99, 230, 226), "systemMint");
UIColor UIColor::systemTeal = UIColorThemed(UIColor(48, 176, 199), UIColor(64, 200, 224), "systemTeal");
UIColor UIColor::systemCyan = UIColorThemed(UIColor(50, 173, 230), UIColor(100, 210, 255), "systemCyan");
UIColor UIColor::systemBlue = UIColorThemed(UIColor(0, 122, 255), UIColor(10, 132, 255), "systemBlue");
UIColor UIColor::systemIndigo = UIColorThemed(UIColor(88, 86, 214), UIColor(94, 92, 230), "systemIndigo");
UIColor UIColor::systemPurple = UIColorThemed(UIColor(175, 82, 222), UIColor(191, 90, 242), "systemPurple");
UIColor UIColor::systemPink = UIColorThemed(UIColor(255, 45, 85), UIColor(255, 55, 95), "systemPink");
UIColor UIColor::systemBrown = UIColorThemed(UIColor(162, 132, 94), UIColor(172, 142, 104), "systemBrown");

// Adoptable gray
UIColor UIColor::systemGray = UIColor(142, 142, 147, "systemGray");
UIColor UIColor::systemGray2 = UIColorThemed(UIColor(174, 174, 178), UIColor(99, 99, 102), "systemGray2");
UIColor UIColor::systemGray3 = UIColorThemed(UIColor(199, 199, 204), UIColor(72, 72, 74), "systemGray3");
UIColor UIColor::systemGray4 = UIColorThemed(UIColor(209, 209, 214), UIColor(58, 58, 60), "systemGray4");
UIColor UIColor::systemGray5 = UIColorThemed(UIColor(229, 229, 234), UIColor(44, 44, 46), "systemGray5");
UIColor UIColor::systemGray6 = UIColorThemed(UIColor(242, 242, 247), UIColor(28, 28, 30), "systemGray6");

// Label
UIColor UIColor::label = UIColorThemed(UIColor(0, 0, 0), UIColor(255, 255, 255), "label");
UIColor UIColor::secondaryLabel = UIColorThemed(UIColor(0x993c3c43), UIColor(0x99ebebf5), "secondaryLabel");
UIColor UIColor::tertiaryLabel = UIColorThemed(UIColor(0x4c3c3c43), UIColor(0x4cebebf5), "tertiaryLabel");
UIColor UIColor::quaternaryLabel = UIColorThemed(UIColor(0x2d3c3c43), UIColor(0x28ebebf5), "quaternaryLabel");

// Text
UIColor UIColor::placeholderText = UIColorThemed(UIColor(0x3c3c434c), UIColor(0xebebf54c), "placeholderText");

// Fill
UIColor UIColor::systemFill = UIColorThemed(UIColor(0x33787880), UIColor(0x5c787880), "systemFill");
UIColor UIColor::secondarySystemFill = UIColorThemed(UIColor(0x29787880), UIColor(0x52787880), "secondarySystemFill");
UIColor UIColor::tertiarySystemFill = UIColorThemed(UIColor(0x1f767680), UIColor(0x3d767680), "tertiarySystemFill");
UIColor UIColor::quaternarySystemFill = UIColorThemed(UIColor(0x14747480), UIColor(0x2e767680), "quaternarySystemFill");

// Standard content background
UIColor UIColor::systemBackground = UIColorThemed(UIColor(255, 255, 255), UIColor(0, 0, 0), "systemBackground");
UIColor UIColor::secondarySystemBackground = UIColorThemed(UIColor(242, 242, 242), UIColor(28, 28, 30), "secondarySystemBackground");
UIColor UIColor::tertiarySystemBackground = UIColorThemed(UIColor(255, 255, 255), UIColor(44, 44, 46), "tertiarySystemBackground");

// Grouped content background
UIColor UIColor::systemGroupedBackground = UIColorThemed(UIColor(242, 242, 242), UIColor(0, 0, 0), "systemGroupedBackground");
UIColor UIColor::secondarySystemGroupedBackground = UIColorThemed(UIColor(255, 255, 255), UIColor(28, 28, 30), "secondarySystemGroupedBackground");
UIColor UIColor::tertiarySystemGroupedBackground = UIColorThemed(UIColor(242, 242, 242), UIColor(44, 44, 46), "tertiarySystemGroupedBackground");

// Separator
UIColor UIColor::separator = UIColorThemed(UIColor(0x4a3c3c43), UIColor(0x99545458), "separator");
UIColor UIColor::opaqueSeparator = UIColorThemed(UIColor(198, 198, 200), UIColor(56, 56, 58), "opaqueSeparator");

// Tint
UIColor UIColor::tint = UIColor([](auto) {
    return UIColor::_currentTint;
}, "tint");

UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a, std::string id): _id(std::move(id)) {
    color = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}
UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b, std::string id): UIColor(r, g, b, 255, std::move(id)) {}
UIColor::UIColor(): UIColor(0, 0, 0) {}

UIColor::UIColor(int rawValue, std::string id): color(rawValue), _id(std::move(id)) {}

UIColor::UIColor(UIColor const &color) = default;

UIColor::UIColor(const std::function<UIColor(std::shared_ptr<UITraitCollection>)>& dynamicProvider, std::string id): _id(std::move(id)) {
    this->dynamicProvider = dynamicProvider;
}

uint32_t UIColor::raw() const {
    if (dynamicProvider == std::nullopt) {
        return color;
    }

    return dynamicProvider.value()(UITraitCollection::current()).raw();
}

unsigned char UIColor::r() const {
    return static_cast<unsigned char>((raw() >> 16) & 0xff);
}

unsigned char UIColor::g() const {
    return static_cast<unsigned char>((raw() >> 8) & 0xff);
}

unsigned char UIColor::b() const {
    return static_cast<unsigned char>(raw() & 0xff);
}

unsigned char UIColor::a() const {
    return static_cast<unsigned char>((raw() >> 24) & 0xff);
}

bool UIColor::operator==(const UIColor& rhs) const {
    if (!_id.empty() || !rhs._id.empty())
        return _id == rhs._id;

    return this->raw() == rhs.raw();
}

bool UIColor::operator!=(const UIColor& rhs) const {
    return !(*this == rhs);
}

UIColor UIColor::withAlphaComponent(NXFloat alpha) {
    UIColor copy(*this);
    auto mixMaxAlpha = std::max(0.0f, std::min(alpha, 1.0f));

    return UIColor([copy, mixMaxAlpha](auto) {
        return UIColor(copy.r(), copy.g(), copy.b(), (unsigned char) (255 * mixMaxAlpha));
    });
}

UIColor UIColor::interpolationTo(const UIColor& endResult, NXFloat progress) const {
    auto startR = r();
    auto startG = g();
    auto startB = b();
    auto startA = a();

    auto endR = endResult.r();
    auto endG = endResult.g();
    auto endB = endResult.b();
    auto endA = endResult.a();

    auto currentProgress = progress * 255;
    auto maxProgress = UINT8_MAX;

    auto resultR = startR + (endR - startR) * currentProgress / maxProgress;
    auto resultG = startG + (endG - startG) * currentProgress / maxProgress;
    auto resultB = startB + (endB - startB) * currentProgress / maxProgress;
    auto resultA = startA + (endA - startA) * currentProgress / maxProgress;

#define res(x) fmaxf(0, fminf(255, abs(x)))

    return UIColor(
        res(resultR),
        res(resultG),
        res(resultB),
        res(resultA)
    );
}
