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

Size Size::inset(UIEdgeInsets inset) const {
    return Size(width + inset.left + inset.right, height + inset.top + inset.bottom);
}

// MARK: - RECT -
Rect::Rect(): origin(), size() { }
Rect::Rect(Point origin, Size size): origin(origin), size(size) { }
Rect::Rect(float x, float y, float width, float height): origin(x, y), size(width, height) { }

float Rect::width() const { return size.width; }
float Rect::height() const { return size.height; }

float Rect::minX() const { return origin.x; }
float Rect::midX() const { return origin.x + size.width / 2; }
float Rect::maxX() const { return origin.x + size.width; }

float Rect::minY() const { return origin.y; }
float Rect::midY() const { return origin.y + size.height / 2; }
float Rect::maxY() const { return origin.y + size.height; }

bool Rect::contains(Point point) {
    return
    (point.x >= minX()) && (point.x < maxX()) &&
    (point.y >= minY()) && (point.y < maxY());
}

bool Rect::intersects(const Rect& other) const {
    return !((minX() > other.maxX() || maxX() < other.minX()) || (minY() > other.maxY() || maxY() < other.minY()));
}

Rect& Rect::offsetBy(const Point& offset) {
    origin.x += offset.x;
    origin.y += offset.y;
    return *this;
}

Rect& Rect::offsetBy(const float& offsetX, const float& offsetY) {
    origin.x += offsetX;
    origin.y += offsetY;
    return *this;
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

// MARK: - INDEX PATH -
IndexPath::IndexPath(int row, int section) {
    a[0] = row;
    a[1] = section;
}

int IndexPath::row() const {
    return a[0];
}

int IndexPath::section() const {
    return a[1];
}

int IndexPath::item() const {
    return a[0];
}

IndexPath IndexPath::prev() const {
    IndexPath res = *this;

    if (res.row() <= 0)
        res.a[1]--;
    else
        res.a[0]--;

    return res;
}

IndexPath IndexPath::next(int rowLimit) const {
    IndexPath res = *this;

    if (res.row() >= rowLimit - 1)
        res.a[1]++;
    else
        res.a[0]++;
    return res;
}

bool IndexPath::operator<(const IndexPath &o) const {
    if (section() == o.section()) {
        return row() < o.row();
    }
    return section() < o.section();
}

bool IndexPath::operator==(const IndexPath &o) const {
    return section() == o.section() && row() == o.row();
}

}
