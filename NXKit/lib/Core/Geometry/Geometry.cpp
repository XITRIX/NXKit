//
//  Geometry.cpp
//  NXKit
//
//  Created by Даниил Виноградов on 01.09.2022.
//

#include <Core/Geometry/Geometry.hpp>

namespace NXKit {

// MARK: - PRIVATE -
float min(float a, float b, float c, float d) {
    auto minValue = (a < b) ? a : b;
    minValue = (minValue < c) ? minValue : c;
    minValue = (minValue < d) ? minValue : d;
    return minValue;
}

float max(float a, float b, float c, float d) {
    auto maxValue = (a > b) ? a : b;
    maxValue = (maxValue > c) ? maxValue : c;
    maxValue = (maxValue > d) ? maxValue : d;
    return maxValue;
}

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

Point Point::applying(const NXAffineTransform& t) const {
    return Point(
        x * t.m11 + y * t.m21 + t.tX,
        x * t.m12 + y * t.m22 + t.tY
    );
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

Rect Rect::intersection(const Rect& other) const {
    auto largestMinX = std::max(minX(), other.minX());
    auto largestMinY = std::max(minY(), other.minY());

    auto smallestMaxX = std::min(maxX(), other.maxX());
    auto smallestMaxY = std::min(maxY(), other.maxY());

    auto width = smallestMaxX - largestMinX;
    auto height = smallestMaxY - largestMinY;

    if (width > 0 && height > 0) {
        // The intersection rectangle has dimensions, i.e. there is an intersection:
        return Rect(largestMinX, largestMinY, width, height);
    } else {
        return Rect::null;
    }
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

Rect Rect::applying(const NXAffineTransform& t) const {
    if (t.isIdentity()) { return *this; }

    auto newTopLeft = Point(minX(), minY()).applying(t);
    auto newTopRight = Point(maxX(), minY()).applying(t);
    auto newBottomLeft = Point(minX(), maxY()).applying(t);
    auto newBottomRight = Point(maxX(), maxY()).applying(t);


    auto newMinX = min(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);
    auto newMaxX = max(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);

    auto newMinY = min(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);
    auto newMaxY = max(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);

    // XXX: What happens if the point that was furthest left is now on the right (because of a rotation)?
    // i.e. Should do we return a normalised rect or one with a negative width?
    return Rect(
        newMinX,
        newMinY,
        newMaxX - newMinX,
        newMaxY - newMinY);
}

Rect Rect::applying(const NXTransform3D& t) const {
    if (t == NXTransform3DIdentity) { return *this; }

    auto topLeft = t.transformingVector(minX(), minY(), 0);
    auto topRight = t.transformingVector(maxX(), minY(), 0);
    auto bottomLeft = t.transformingVector(minX(), maxY(), 0);
    auto bottomRight = t.transformingVector(maxX(), maxY(), 0);

    auto newMinX = min(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);
    auto newMaxX = max(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);

    auto newMinY = min(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);
    auto newMaxY = max(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);

    return Rect(newMinX,
                newMinY,
                newMaxX - newMinX,
                newMaxY - newMinY);
}

bool Rect::operator==(const Rect& rhs) {
    return
    this->origin.x == rhs.origin.x && this->origin.y == rhs.origin.y &&
    this->size.width == rhs.size.width && this->size.height == rhs.size.height;
}

bool Rect::valid() {
    return this->origin.valid() && this->size.valid();
}

Rect Rect::zero = Rect();
Rect Rect::null = Rect(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), 0, 0);

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
