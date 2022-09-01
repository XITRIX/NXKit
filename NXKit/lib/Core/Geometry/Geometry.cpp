//
//  Geometry.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/Geometry/Geometry.hpp>

namespace NXKit {

// MARK: - POINT -
Point::Point(): x(0), y(0) { }
Point::Point(float x, float y): x(x), y(y) { }

bool Point::operator==(const Point& rhs) const {
    return this->x == rhs.x && this->y == rhs.y;
}

Point Point::operator+(const Point& first) const {
    return Point(x + first.x, y + first.y);
}

Point Point::operator-(const Point& first) const {
    return Point(x - first.x, y - first.y);
}

Point& Point::operator+=(const Point& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

Point& Point::operator-=(const Point& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

Point Point::operator/(const float& rhs) {
    auto res = *this;
    res.x /= rhs;
    res.y /= rhs;
    return res;
}

Point Point::operator*(const float& rhs) {
    auto res = *this;
    res.x *= rhs;
    res.y *= rhs;
    return res;
}

bool Point::valid() {
    return !isnan(this->x) && !isnan(this->y);
}

float Point::magnitude() {
    return sqrt(x * x + y * y);
}

// MARK: - SIZE -
Size::Size(): width(0), height(0) { }
Size::Size(float width, float height): width(width), height(height) { }

bool Size::operator==(const Size& rhs) {
    return this->width == rhs.width && this->height == rhs.height;
}

Size Size::operator+(const Size& first) const {
    return Size(width + first.width, height + first.height);
}

Size Size::operator-(const Size& first) const {
    return Size(width - first.width, height - first.height);
}

Size& Size::operator+=(const Size& rhs) {
    this->width += rhs.width;
    this->height += rhs.height;
    return *this;
}

Size& Size::operator-=(const Size& rhs) {
    this->width -= rhs.width;
    this->height -= rhs.height;
    return *this;
}

bool Size::valid() {
    return !isnan(this->width) && !isnan(this->height);
}

// MARK: - RECT -
Rect::Rect(): origin(), size() { }
Rect::Rect(Point origin, Size size): origin(origin), size(size) { }
Rect::Rect(float x, float y, float width, float height): origin(x, y), size(width, height) { }

float Rect::width() { return size.width; }
float Rect::height() { return size.height; }

float Rect::minX() { return origin.x; }
float Rect::midX() { return origin.x + size.width / 2; }
float Rect::maxX() { return origin.x + size.width; }

float Rect::minY() { return origin.y; }
float Rect::midY() { return origin.y + size.height / 2; }
float Rect::maxY() { return origin.y + size.height; }

bool Rect::contains(Point point) {
    return
    (point.x >= minX()) && (point.x < maxX()) &&
    (point.y >= minY()) && (point.y < maxY());
}

Rect& Rect::insetBy(const UIEdgeInsets& insets) {
    origin.x -= insets.left;
    origin.y -= insets.top;
    size.width += insets.left + insets.right;
    size.height += insets.top + insets.bottom;
    return *this;
}

bool Rect::operator==(const Rect& rhs) {
    return
    this->origin.x == rhs.origin.x && this->origin.y == rhs.origin.y &&
    this->size.width == rhs.size.width && this->size.height == rhs.size.height;
}

bool Rect::valid() {
    return this->origin.valid() && this->size.valid();
}

}
