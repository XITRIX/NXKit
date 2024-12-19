#include "UIColor.h"
#include <limits>
#include <math.h>

using namespace NXKit;

// Transparent
UIColor UIColor::clear = UIColor(0, 0, 0, 0);

// Fixed
UIColor UIColor::black = UIColor(0, 0, 0);
UIColor UIColor::darkGray = UIColor(85, 85, 85);
UIColor UIColor::gray = UIColor(128, 128, 128);
UIColor UIColor::lightGray = UIColor(170, 170, 170);
UIColor UIColor::white = UIColor(255, 255, 255);
UIColor UIColor::red = UIColor(255, 0, 0);
UIColor UIColor::orange = UIColor(255, 128, 0);
UIColor UIColor::yellow = UIColor(255, 255, 0);
UIColor UIColor::green = UIColor(0, 255, 0);
UIColor UIColor::cyan = UIColor(0, 255, 255);
UIColor UIColor::blue = UIColor(0, 0, 255);
UIColor UIColor::purple = UIColor(128, 0, 128);
UIColor UIColor::magenta = UIColor(255, 0, 255);
UIColor UIColor::brown = UIColor(153, 102, 51);

// Adaptable
UIColor UIColor::systemRed = UIColorThemed(UIColor(255, 59, 48), UIColor(255, 69, 58));
UIColor UIColor::systemOrange = UIColorThemed(UIColor(255, 149, 0), UIColor(255, 159, 10));
UIColor UIColor::systemYellow = UIColorThemed(UIColor(242, 204, 0), UIColor(255, 214, 10));
UIColor UIColor::systemGreen = UIColorThemed(UIColor(52, 199, 89), UIColor(48, 209, 88));
UIColor UIColor::systemMint = UIColorThemed(UIColor(0, 199, 190), UIColor(99, 230, 226));
UIColor UIColor::systemTeal = UIColorThemed(UIColor(48, 176, 199), UIColor(64, 200, 224));
UIColor UIColor::systemCyan = UIColorThemed(UIColor(50, 173, 230), UIColor(100, 210, 255));
UIColor UIColor::systemBlue = UIColorThemed(UIColor(0, 122, 255), UIColor(10, 132, 255));
UIColor UIColor::systemIndigo = UIColorThemed(UIColor(88, 86, 214), UIColor(94, 92, 230));
UIColor UIColor::systemPurple = UIColorThemed(UIColor(175, 82, 222), UIColor(191, 90, 242));
UIColor UIColor::systemPink = UIColorThemed(UIColor(255, 45, 85), UIColor(255, 55, 95));
UIColor UIColor::systemBrown = UIColorThemed(UIColor(162, 132, 94), UIColor(172, 142, 104));

// Label
UIColor UIColor::label = UIColorThemed(UIColor(0, 0, 0), UIColor(255, 255, 255));
UIColor UIColor::secondaryLabel = UIColorThemed(UIColor(0x993c3c43), UIColor(0x99ebebf5));
UIColor UIColor::tertiaryLabel = UIColorThemed(UIColor(0x4c3c3c43), UIColor(0x4cebebf5));
UIColor UIColor::quaternaryLabel = UIColorThemed(UIColor(0x2d3c3c43), UIColor(0x28ebebf5));

// Standard content background
UIColor UIColor::systemBackground = UIColorThemed(UIColor(255, 255, 255), UIColor(0, 0, 0));
UIColor UIColor::secondarySystemBackground = UIColorThemed(UIColor(242, 242, 242), UIColor(28, 28, 30));
UIColor UIColor::tetriarySystemBackground = UIColorThemed(UIColor(255, 255, 255), UIColor(44, 44, 46));

// Separator
UIColor UIColor::separator = UIColorThemed(UIColor(0x4a3c3c43), UIColor(0x99545458));

UIColor UIColor::tint = systemBlue;

UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    color = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}
UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b): UIColor(r, g, b, 255) {}
UIColor::UIColor(): UIColor(0, 0, 0) {}

UIColor::UIColor(int rawValue): color(rawValue) {}


UIColor::UIColor(std::function<UIColor(std::shared_ptr<UITraitCollection>)> dynamicProvider) {
    this->dynamicProvider = dynamicProvider;
}

uint32_t UIColor::raw() const {
    if (dynamicProvider == std::nullopt) {
        return color;
    }

    return dynamicProvider.value()(UITraitCollection::current()).raw();
}

unsigned char UIColor::r() {
    return static_cast<unsigned char>((raw() >> 16) & 0xff);
}

unsigned char UIColor::g() {
    return static_cast<unsigned char>((raw() >> 8) & 0xff);
}

unsigned char UIColor::b() {
    return static_cast<unsigned char>(raw() & 0xff);
}

unsigned char UIColor::a() {
    return static_cast<unsigned char>((raw() >> 24) & 0xff);
}

bool UIColor::operator==(const UIColor& rhs) const {
    return this->raw() == rhs.raw();
}

UIColor UIColor::interpolationTo(UIColor endResult, NXFloat progress) {
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
