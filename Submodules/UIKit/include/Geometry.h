#pragma once

namespace NXKit {

typedef double NXFloat;

struct NXPoint {
    NXFloat x, y;

    NXPoint();
    NXPoint(NXFloat x, NXFloat y);

    bool operator==(const NXPoint& rhs) const;
    NXPoint operator+(const NXPoint& first) const;
    NXPoint operator-(const NXPoint& first) const;
    NXPoint& operator+=(const NXPoint& rhs);
    NXPoint& operator-=(const NXPoint& rhs);
    NXPoint operator/(const NXFloat& rhs);
    NXPoint operator*(const NXFloat& rhs);

//    Point applying(const NXAffineTransform& t) const;

    bool valid();
    NXFloat magnitude();
};

struct NXSize {
    NXFloat width, height;

    NXSize();
    NXSize(NXFloat width, NXFloat height);

    bool operator==(const NXSize &rhs) const;

    NXSize operator+(const NXSize &first) const;
    NXSize operator-(const NXSize &first) const;
    NXSize &operator+=(const NXSize &rhs);
    NXSize &operator-=(const NXSize &rhs);

    NXSize operator*(const NXFloat &first) const;
    NXSize operator/(const NXFloat &first) const;
    NXSize &operator*=(const NXFloat &rhs);
    NXSize &operator/=(const NXFloat &rhs);

    bool valid();
//    NXSize inset(UIEdgeInsets inset) const;
};

struct NXRect {
    NXPoint origin;
    NXSize size;

    NXRect();
    NXRect(NXPoint origin, NXSize size);
    NXRect(NXFloat x, NXFloat y, NXFloat width, NXFloat height);

    NXFloat width() const;
    NXFloat height() const;

    NXFloat minX() const;
    NXFloat midX() const;
    NXFloat maxX() const;

    NXFloat minY() const;
    NXFloat midY() const;
    NXFloat maxY() const;

    bool contains(NXPoint point);
    bool intersects(const NXRect& other) const;
    NXRect intersection(const NXRect& other) const;
    NXRect& offsetBy(const NXPoint& offset);
    NXRect& offsetBy(const NXFloat& offsetX, const NXFloat& offsetY);
//    NXRect& insetBy(const UIEdgeInsets& insets);
//    NXRect applying(const NXAffineTransform& t) const;
//    NXRect applying(const NXTransform3D& t) const;

    bool operator==(const NXRect& rhs);

    bool valid();

    static NXRect zero;
    static NXRect null;
};

}