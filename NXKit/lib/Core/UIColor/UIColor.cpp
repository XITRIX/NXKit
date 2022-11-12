//
//  UIColor.cpp
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#include <Core/UIColor/UIColor.hpp>
#include <Core/Utils/Tools/Tools.hpp>

namespace NXKit {

UIColor UIColor::clear = UIColor(0, 0, 0, 0);
UIColor UIColor::red = UIColor(255, 0, 0);
UIColor UIColor::green = UIColor(0, 255, 0);
UIColor UIColor::blue = UIColor(0, 0, 255);
UIColor UIColor::orange = UIColor(255, 150, 0);
UIColor UIColor::cyan = UIColor(0, 150, 255);
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

UIColor UIColor::random() {
    return UIColor(rand() % 255, rand() % 255, rand() % 255);
}

UIColor::UIColor(short r, short g, short b, short a) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
    darkValue = value;
}

UIColor::UIColor(short r, short g, short b, short a, short dr, short dg, short db, short da) {
    value = (a & 0xff) << 24 | (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
    darkValue = (da & 0xff) << 24 | (dr & 0xff) << 16 | (dg & 0xff) << 8 | (db & 0xff);
}

UIColor UIColor::f(float r, float g, float b, float a) {
    return UIColor(r * 255, g * 255, b * 255, a * 255);
}

int UIColor::colorByCurrentStyle() {
    if (UITraitCollection::current.userInterfaceStyle == UIUserInterfaceStyle::dark)
        return darkValue;
    return value;
}

unsigned char UIColor::r() {
    return static_cast<unsigned char>((colorByCurrentStyle() >> 16) & 0xff);
}

unsigned char UIColor::g() {
    return static_cast<unsigned char>((colorByCurrentStyle() >> 8) & 0xff);
}

unsigned char UIColor::b() {
    return static_cast<unsigned char>(colorByCurrentStyle() & 0xff);
}

unsigned char UIColor::a() {
    return static_cast<unsigned char>((colorByCurrentStyle() >> 24) & 0xff);
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

void UIColor::fillAnimationContext(std::deque<float>* context) {
    auto cache = UITraitCollection::current;

    UITraitCollection::current.userInterfaceStyle = UIUserInterfaceStyle::light;

    context->push_back(r());
    context->push_back(g());
    context->push_back(b());
    context->push_back(a());

    UITraitCollection::current.userInterfaceStyle = UIUserInterfaceStyle::dark;

    context->push_back(r());
    context->push_back(g());
    context->push_back(b());
    context->push_back(a());

    UITraitCollection::current = cache;
}

float getColor(float val, float alt) {
    if (!isnan(val))
        return minmax(0.0f, val, 255.0f);
    return alt;
}

void UIColor::apply(std::deque<float>* context) {
    auto cache = UITraitCollection::current;

    UITraitCollection::current.userInterfaceStyle = UIUserInterfaceStyle::light;

    auto rc = getColor(pop(context), r());
    auto gc = getColor(pop(context), g());
    auto bc = getColor(pop(context), b());
    auto ac = getColor(pop(context), a());

    UITraitCollection::current.userInterfaceStyle = UIUserInterfaceStyle::dark;

    auto drc = getColor(pop(context), r());
    auto dgc = getColor(pop(context), g());
    auto dbc = getColor(pop(context), b());
    auto dac = getColor(pop(context), a());

    UITraitCollection::current = cache;

    auto color = UIColor(rc, gc, bc, ac, drc, dgc, dbc, dac);

    value = color.value;
    darkValue = color.darkValue;
}

UIColor UIColor::fromAnimationContext(std::deque<float>* context) {
    auto r = minmax(0.0f, pop(context), 255.0f);
    auto g = minmax(0.0f, pop(context), 255.0f);
    auto b = minmax(0.0f, pop(context), 255.0f);
    auto a = minmax(0.0f, pop(context), 255.0f);

    auto dr = minmax(0.0f, pop(context), 255.0f);
    auto dg = minmax(0.0f, pop(context), 255.0f);
    auto db = minmax(0.0f, pop(context), 255.0f);
    auto da = minmax(0.0f, pop(context), 255.0f);

    return UIColor(r, g, b, a, dr, dg, db, da);
}

}
