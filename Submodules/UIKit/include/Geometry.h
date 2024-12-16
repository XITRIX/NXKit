#pragma once

namespace NXKit {

typedef double NXFloat;

struct NXAffineTransform;
struct NXTransform3D;

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

    NXPoint applying(const NXAffineTransform& t) const;

    bool valid();
    NXFloat magnitude();

    static NXPoint zero;
};

struct NXSize {
    NXFloat width, height;

    NXSize();
    NXSize(NXFloat width, NXFloat height);

    bool operator==(const NXSize &rhs) const;
    bool operator!=(const NXSize &rhs) const;

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

    void setWidth(NXFloat newValue);
    void setHeight(NXFloat newValue);

    void setMinX(NXFloat newValue);
    void setMidX(NXFloat newValue);
    void setMaxX(NXFloat newValue);

    void setMinY(NXFloat newValue);
    void setMidY(NXFloat newValue);
    void setMaxY(NXFloat newValue);

    bool contains(NXPoint point);
    bool intersects(const NXRect& other) const;
    NXRect& offsetBy(const NXPoint& offset);
    NXRect& offsetBy(const NXFloat& offsetX, const NXFloat& offsetY);

    bool operator==(const NXRect& rhs) const;
    NXRect operator+(const NXRect& rhs) const;
    NXRect operator-(const NXRect& rhs) const;
    NXRect operator*(const NXFloat& rhs) const;

    NXRect applying(NXAffineTransform transform);
    NXRect applying(NXTransform3D transform);

    NXRect intersection(NXRect other) const;

    bool isNull() const;
    static NXRect null;
};

}
