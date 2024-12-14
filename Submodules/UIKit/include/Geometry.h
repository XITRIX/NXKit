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
    NXRect(float x, float y, float width, float height);

    float width() const;
    float height() const;

    float minX() const;
    float midX() const;
    float maxX() const;

    float minY() const;
    float midY() const;
    float maxY() const;

    void setWidth(float newValue);
    void setHeight(float newValue);

    void setMinX(float newValue);
    void setMidX(float newValue);
    void setMaxX(float newValue);

    void setMinY(float newValue);
    void setMidY(float newValue);
    void setMaxY(float newValue);

    bool contains(NXPoint point);
    bool intersects(const NXRect& other) const;
    NXRect& offsetBy(const NXPoint& offset);
    NXRect& offsetBy(const float& offsetX, const float& offsetY);

    bool operator==(const NXRect& rhs) const;
    NXRect operator+(const NXRect& rhs) const;
    NXRect operator-(const NXRect& rhs) const;
    NXRect operator*(const float& rhs) const;

    NXRect applying(NXAffineTransform transform);
    NXRect applying(NXTransform3D transform);

    NXRect intersection(NXRect other) const;

    bool isNull() const;
    static NXRect null;
};

}
