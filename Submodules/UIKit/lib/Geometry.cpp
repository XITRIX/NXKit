#include <Geometry.h>
#include <algorithm>

using namespace NXKit;

// MARK: - POINT -
NXPoint::NXPoint(): x(0), y(0) { }
NXPoint::NXPoint(NXFloat x, NXFloat y): x(x), y(y) { }

bool NXPoint::operator==(const NXPoint& rhs) const {
    return this->x == rhs.x && this->y == rhs.y;
}

NXPoint NXPoint::operator+(const NXPoint& first) const {
    return NXPoint(x + first.x, y + first.y);
}

NXPoint NXPoint::operator-(const NXPoint& first) const {
    return NXPoint(x - first.x, y - first.y);
}

NXPoint& NXPoint::operator+=(const NXPoint& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

NXPoint& NXPoint::operator-=(const NXPoint& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

NXPoint NXPoint::operator/(const NXFloat& rhs) {
    auto res = *this;
    res.x /= rhs;
    res.y /= rhs;
    return res;
}

NXPoint NXPoint::operator*(const NXFloat& rhs) {
    auto res = *this;
    res.x *= rhs;
    res.y *= rhs;
    return res;
}

//NXPoint NXPoint::applying(const NXAffineTransform& t) const {
//    return NXPoint(
//            x * t.m11 + y * t.m21 + t.tX,
//            x * t.m12 + y * t.m22 + t.tY
//    );
//}

bool NXPoint::valid() {
    return !isnan(this->x) && !isnan(this->y);
}

NXFloat NXPoint::magnitude() {
    return sqrt(x * x + y * y);
}

// MARK: - NXSize
NXSize::NXSize(): NXSize(0, 0) {}
NXSize::NXSize(NXFloat width, NXFloat height): width(width), height(height) {}

bool NXSize::operator==(const NXSize& rhs) const {
    return this->width == rhs.width && this->height == rhs.height;
}

NXSize NXSize::operator+(const NXSize& first) const {
    return NXSize(width + first.width, height + first.height);
}

NXSize NXSize::operator-(const NXSize& first) const {
    return NXSize(width - first.width, height - first.height);
}

NXSize& NXSize::operator+=(const NXSize& rhs) {
    this->width += rhs.width;
    this->height += rhs.height;
    return *this;
}

NXSize& NXSize::operator-=(const NXSize& rhs) {
    this->width -= rhs.width;
    this->height -= rhs.height;
    return *this;
}

NXSize NXSize::operator*(const NXFloat& first) const {
    return NXSize(width * first, height * first);
}

NXSize NXSize::operator/(const NXFloat& first) const {
    return NXSize(width / first, height / first);
}

NXSize& NXSize::operator*=(const NXFloat& rhs) {
    this->width *= rhs;
    this->height *= rhs;
    return *this;
}

NXSize& NXSize::operator/=(const NXFloat& rhs) {
    this->width /= rhs;
    this->height /= rhs;
    return *this;
}

bool NXSize::valid() {
    return !isnan(this->width) && !isnan(this->height);
}

// MARK: - RECT -
NXRect::NXRect(): origin(), size() { }
NXRect::NXRect(NXPoint origin, NXSize size): origin(origin), size(size) { }
NXRect::NXRect(NXFloat x, NXFloat y, NXFloat width, NXFloat height): origin(x, y), size(width, height) { }

NXFloat NXRect::width() const { return size.width; }
NXFloat NXRect::height() const { return size.height; }

NXFloat NXRect::minX() const { return origin.x; }
NXFloat NXRect::midX() const { return origin.x + size.width / 2; }
NXFloat NXRect::maxX() const { return origin.x + size.width; }

NXFloat NXRect::minY() const { return origin.y; }
NXFloat NXRect::midY() const { return origin.y + size.height / 2; }
NXFloat NXRect::maxY() const { return origin.y + size.height; }

bool NXRect::contains(NXPoint point) {
    return
            (point.x >= minX()) && (point.x < maxX()) &&
            (point.y >= minY()) && (point.y < maxY());
}

bool NXRect::intersects(const NXRect& other) const {
    return !((minX() > other.maxX() || maxX() < other.minX()) || (minY() > other.maxY() || maxY() < other.minY()));
}

NXRect NXRect::intersection(const NXRect& other) const {
    auto largestMinX = std::max(minX(), other.minX());
    auto largestMinY = std::max(minY(), other.minY());

    auto smallestMaxX = std::min(maxX(), other.maxX());
    auto smallestMaxY = std::min(maxY(), other.maxY());

    auto width = smallestMaxX - largestMinX;
    auto height = smallestMaxY - largestMinY;

    if (width > 0 && height > 0) {
        // The intersection rectangle has dimensions, i.e. there is an intersection:
        return NXRect(largestMinX, largestMinY, width, height);
    } else {
        return NXRect::null;
    }
}

NXRect& NXRect::offsetBy(const NXPoint& offset) {
    origin.x += offset.x;
    origin.y += offset.y;
    return *this;
}

NXRect& NXRect::offsetBy(const NXFloat& offsetX, const NXFloat& offsetY) {
    origin.x += offsetX;
    origin.y += offsetY;
    return *this;
}

//NXRect& NXRect::insetBy(const UIEdgeInsets& insets) {
//    origin.x -= insets.left;
//    origin.y -= insets.top;
//    size.width += insets.left + insets.right;
//    size.height += insets.top + insets.bottom;
//    return *this;
//}

//NXRect NXRect::applying(const NXAffineTransform& t) const {
//    if (t.isIdentity()) { return *this; }
//
//    auto newTopLeft = NXPoint(minX(), minY()).applying(t);
//    auto newTopRight = NXPoint(maxX(), minY()).applying(t);
//    auto newBottomLeft = NXPoint(minX(), maxY()).applying(t);
//    auto newBottomRight = NXPoint(maxX(), maxY()).applying(t);
//
//
//    auto newMinX = min(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);
//    auto newMaxX = max(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);
//
//    auto newMinY = min(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);
//    auto newMaxY = max(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);
//
//    // XXX: What happens if the point that was furthest left is now on the right (because of a rotation)?
//    // i.e. Should do we return a normalised rect or one with a negative width?
//    return NXRect(
//            newMinX,
//            newMinY,
//            newMaxX - newMinX,
//            newMaxY - newMinY);
//}

//NXRect NXRect::applying(const NXTransform3D& t) const {
//    if (t == NXTransform3DIdentity) { return *this; }
//
//    auto topLeft = t.transformingVector(minX(), minY(), 0);
//    auto topRight = t.transformingVector(maxX(), minY(), 0);
//    auto bottomLeft = t.transformingVector(minX(), maxY(), 0);
//    auto bottomRight = t.transformingVector(maxX(), maxY(), 0);
//
//    auto newMinX = min(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);
//    auto newMaxX = max(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);
//
//    auto newMinY = min(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);
//    auto newMaxY = max(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);
//
//    return NXRect(newMinX,
//                newMinY,
//                newMaxX - newMinX,
//                newMaxY - newMinY);
//}

bool NXRect::operator==(const NXRect& rhs) {
    return
            this->origin.x == rhs.origin.x && this->origin.y == rhs.origin.y &&
            this->size.width == rhs.size.width && this->size.height == rhs.size.height;
}

bool NXRect::valid() {
    return this->origin.valid() && this->size.valid();
}

NXRect NXRect::zero = NXRect();
NXRect NXRect::null = NXRect(std::numeric_limits<NXFloat>::infinity(), std::numeric_limits<NXFloat>::infinity(), 0, 0);
