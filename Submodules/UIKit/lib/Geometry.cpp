#include <Geometry.h>
#include <cmath>

#include "NXAffineTransform.h"
#include "NXTransform3D.h"

using namespace NXKit;

// MARK: - PRIVATE -
NXFloat min(NXFloat a, NXFloat b, NXFloat c, NXFloat d) {
    auto minValue = (a < b) ? a : b;
    minValue = (minValue < c) ? minValue : c;
    minValue = (minValue < d) ? minValue : d;
    return minValue;
}

NXFloat max(NXFloat a, NXFloat b, NXFloat c, NXFloat d) {
    auto maxValue = (a > b) ? a : b;
    maxValue = (maxValue > c) ? maxValue : c;
    maxValue = (maxValue > d) ? maxValue : d;
    return maxValue;
}

bool isEqual(NXFloat val1, NXFloat val2) {
    if (std::isnan(val1) && std::isnan(val2))
        return true;
    return val1 == val2;
}

// MARK: - POINT -
NXPoint NXPoint::zero = NXPoint();

NXPoint::NXPoint(): x(0), y(0) { }
NXPoint::NXPoint(NXFloat x, NXFloat y): x(x), y(y) { }

bool NXPoint::operator==(const NXPoint& rhs) const {
    return isEqual(this->x, rhs.x) && isEqual(this->y, rhs.y);
}

bool NXPoint::operator!=(const NXPoint& rhs) const {
    return !(*this == rhs);
}

NXPoint NXPoint::operator+(const NXPoint& first) const {
    return {x + first.x, y + first.y};
}

NXPoint NXPoint::operator-(const NXPoint& first) const {
    return {x - first.x, y - first.y};
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

NXPoint NXPoint::operator/(const NXFloat& rhs) const {
    auto res = *this;
    res.x /= rhs;
    res.y /= rhs;
    return res;
}

NXPoint NXPoint::operator*(const NXFloat& rhs) const {
    auto res = *this;
    res.x *= rhs;
    res.y *= rhs;
    return res;
}

NXPoint NXPoint::applying(const NXAffineTransform& t) const {
    return {
            x * t.m11 + y * t.m21 + t.tX,
            x * t.m12 + y * t.m22 + t.tY
    };
}

float NXPoint::distanceToSegment(NXPoint v, NXPoint w) const {
    auto pv_dx = x - v.x;
    auto pv_dy = y - v.y;
    auto wv_dx = w.x - v.x;
    auto wv_dy = w.y - v.y;

    auto dot = pv_dx * wv_dx + pv_dy * wv_dy;
    auto len_sq = wv_dx * wv_dx + wv_dy * wv_dy;
    auto param = dot / len_sq;

    float int_x, int_y; /* intersection of normal to vw that goes through p */

    if (param < 0 || (v.x == w.x && v.y == w.y)) {
        int_x = v.x;
        int_y = v.y;
    } else if (param > 1) {
        int_x = w.x;
        int_y = w.y;
    } else {
        int_x = v.x + param * wv_dx;
        int_y = v.y + param * wv_dy;
    }

    /* Components of normal */
    auto dx = x - int_x;
    auto dy = y - int_y;

    return sqrt(dx * dx + dy * dy);
}

bool NXPoint::valid() const {
    return !std::isnan(this->x) && !std::isnan(this->y);
}

NXFloat NXPoint::magnitude() const {
    return sqrt(x * x + y * y);
}

// MARK: - NXSize
NXSize::NXSize(): NXSize(0, 0) {}
NXSize::NXSize(NXFloat width, NXFloat height): width(width), height(height) {}

bool NXSize::operator==(const NXSize& rhs) const {
    return isEqual(this->width, rhs.width) && isEqual(this->height, rhs.height);
}

bool NXSize::operator!=(const NXSize& rhs) const {
    return !isEqual(this->width, rhs.width) || !isEqual(this->height, rhs.height);
}

NXSize NXSize::operator+(const NXSize& first) const {
    return {width + first.width, height + first.height};
}

NXSize NXSize::operator-(const NXSize& first) const {
    return {width - first.width, height - first.height};
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
    return {width * first, height * first};
}

NXSize NXSize::operator/(const NXFloat& first) const {
    return {width / first, height / first};
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

bool NXSize::valid() const {
    return !std::isnan(this->width) && !std::isnan(this->height);
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

void NXRect::setWidth(NXFloat newValue) { size.width = newValue; }
void NXRect::setHeight(NXFloat newValue) { size.height = newValue; }

void NXRect::setMinX(NXFloat newValue) { origin.x = newValue; }
void NXRect::setMidX(NXFloat newValue) { origin.x = newValue - (size.width / 2); }
void NXRect::setMaxX(NXFloat newValue) { origin.x = newValue - size.width; }

void NXRect::setMinY(NXFloat newValue) { origin.y = newValue; }
void NXRect::setMidY(NXFloat newValue) { origin.y = newValue - (size.height / 2); }
void NXRect::setMaxY(NXFloat newValue) { origin.y = newValue - size.height; }

bool NXRect::contains(NXPoint point) const {
    return
    (point.x >= minX()) && (point.x < maxX()) &&
    (point.y >= minY()) && (point.y < maxY());
}

bool NXRect::intersects(const NXRect& other) const {
    return !((minX() > other.maxX() || maxX() < other.minX()) || (minY() > other.maxY() || maxY() < other.minY()));
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

bool NXRect::operator==(const NXRect& rhs) const {
    return this->origin == rhs.origin && this->size == rhs.size;
}

NXRect NXRect::operator+(const NXRect& rhs) const {
    return {
        this->origin.x + rhs.origin.x,
        this->origin.y + rhs.origin.y,
        this->size.width + rhs.size.width,
        this->size.height + rhs.size.height
    };
}

NXRect NXRect::operator-(const NXRect& rhs) const {
    return {
        this->origin.x - rhs.origin.x,
        this->origin.y - rhs.origin.y,
        this->size.width - rhs.size.width,
        this->size.height - rhs.size.height
    };
}

NXRect NXRect::operator*(const NXFloat& rhs) const {
    return {
        this->origin.x * rhs,
        this->origin.y * rhs,
        this->size.width * rhs,
        this->size.height * rhs
    };
}

NXRect NXRect::applying(NXAffineTransform t) {
    if (t.isIdentity()) { return *this; }

    auto newTopLeft = NXPoint(minX(), minY()).applying(t);
    auto newTopRight = NXPoint(maxX(), minY()).applying(t);
    auto newBottomLeft = NXPoint(minX(), maxY()).applying(t);
    auto newBottomRight = NXPoint(maxX(), maxY()).applying(t);

    auto newMinX = min(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);
    auto newMaxX = max(newTopLeft.x, newTopRight.x, newBottomLeft.x, newBottomRight.x);

    auto newMinY = min(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);
    auto newMaxY = max(newTopLeft.y, newTopRight.y, newBottomLeft.y, newBottomRight.y);

    // XXX: What happens if the point that was furthest left is now on the right (because of a rotation)?
    // i.e. Should do we return a normalised rect or one with a negative width?
    return {
            newMinX,
            newMinY,
            newMaxX - newMinX,
            newMaxY - newMinY
    };
}

NXRect NXRect::applying(NXTransform3D t) {
    if (t == NXTransform3DIdentity) { return *this; }

    auto topLeft = t.transformingVector(minX(), minY(), 0);
    auto topRight = t.transformingVector(maxX(), minY(), 0);
    auto bottomLeft = t.transformingVector(minX(), maxY(), 0);
    auto bottomRight = t.transformingVector(maxX(), maxY(), 0);

    auto newMinX = min(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);
    auto newMaxX = max(topLeft.x, topRight.x, bottomLeft.x, bottomRight.x);

    auto newMinY = min(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);
    auto newMaxY = max(topLeft.y, topRight.y, bottomLeft.y, bottomRight.y);

    return {newMinX, newMinY, newMaxX - newMinX, newMaxY - newMinY};
}

NXRect NXRect::intersection(NXRect other) const {
    auto largestMinX = fmaxf(minX(), other.minX());
    auto largestMinY = fmaxf(minY(), other.minY());

    auto smallestMaxX = fmaxf(maxX(), other.maxX());
    auto smallestMaxY = fmaxf(maxY(), other.maxY());

    auto width = smallestMaxX - largestMinX;
    auto height = smallestMaxY - largestMinY;

    if (width > 0 && height > 0) {
        // The intersection rectangle has dimensions, i.e. there is an intersection:
        return {largestMinX, largestMinY, width, height};
    } else {
        return null;
    }
}

bool NXRect::isNull() const {
    return *this == null;
}

NXRect NXRect::null = NXRect(INFINITY, INFINITY, 0, 0);

NXFloat Geometry::rubberBandClamp(NXFloat x, NXFloat coeff, NXFloat dim) {
    return (1.0f - (1.0f / ((x * coeff / dim) + 1.0f))) * dim;
}

NXFloat Geometry::rubberBandClamp(NXFloat x, NXFloat coeff, NXFloat dim, NXFloat limitStart, NXFloat limitEnd) {
    auto clampedX = fminf(fmaxf(x, limitStart), limitEnd);
    auto diff = abs(x - clampedX);
    float sign = clampedX > x ? -1 : 1;

    return clampedX + sign * rubberBandClamp(diff, coeff, dim);
}
