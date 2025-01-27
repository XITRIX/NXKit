#pragma once

#include "src/gpu/ganesh/gl/builders/GrGLProgramBuilder.h"

namespace NXKit {

typedef float NXFloat;

struct NXAffineTransform;
struct NXTransform3D;
struct UIEdgeInsets;

struct NXPoint {
    NXFloat x, y;

    NXPoint();
    NXPoint(NXFloat x, NXFloat y);

    bool operator==(const NXPoint& rhs) const;
    bool operator!=(const NXPoint& rhs) const;
    NXPoint operator+(const NXPoint& first) const;
    NXPoint operator-(const NXPoint& first) const;
    NXPoint& operator+=(const NXPoint& rhs);
    NXPoint& operator-=(const NXPoint& rhs);
    NXPoint operator/(const NXFloat& rhs) const;
    NXPoint operator*(const NXFloat& rhs) const;

    [[nodiscard]] NXPoint applying(const NXAffineTransform& t) const;
    [[nodiscard]] NXFloat distanceToSegment(NXPoint v, NXPoint w) const;

    [[nodiscard]] bool valid() const;
    [[nodiscard]] NXFloat magnitude() const;

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

    [[nodiscard]] bool valid() const;
//    NXSize inset(UIEdgeInsets inset) const;
};

struct NXRect {
    NXPoint origin;
    NXSize size;

    NXRect();
    NXRect(NXPoint origin, NXSize size);
    NXRect(NXFloat x, NXFloat y, NXFloat width, NXFloat height);

    [[nodiscard]] NXFloat width() const;
    [[nodiscard]] NXFloat height() const;

    [[nodiscard]] NXFloat minX() const;
    [[nodiscard]] NXFloat midX() const;
    [[nodiscard]] NXFloat maxX() const;

    [[nodiscard]] NXFloat minY() const;
    [[nodiscard]] NXFloat midY() const;
    [[nodiscard]] NXFloat maxY() const;

    void setWidth(NXFloat newValue);
    void setHeight(NXFloat newValue);

    void setMinX(NXFloat newValue);
    void setMidX(NXFloat newValue);
    void setMaxX(NXFloat newValue);

    void setMinY(NXFloat newValue);
    void setMidY(NXFloat newValue);
    void setMaxY(NXFloat newValue);

    [[nodiscard]] bool contains(NXPoint point) const;
    [[nodiscard]] bool intersects(const NXRect& other) const;
    [[nodiscard]] NXRect offsetBy(const NXPoint& offset) const;
    [[nodiscard]] NXRect offsetBy(const NXFloat& offsetX, const NXFloat& offsetY) const;
    [[nodiscard]] NXRect insetBy(const UIEdgeInsets& insets) const;

    bool operator==(const NXRect& rhs) const;
    NXRect operator+(const NXRect& rhs) const;
    NXRect operator-(const NXRect& rhs) const;
    NXRect operator*(const NXFloat& rhs) const;

    [[nodiscard]] NXRect applying(NXAffineTransform transform) const;
    [[nodiscard]] NXRect applying(NXTransform3D transform) const;

    [[nodiscard]] NXRect intersection(NXRect other) const;

    [[nodiscard]] bool isNull() const;
    static NXRect null;
};

struct Geometry {
    static NXFloat rubberBandClamp(NXFloat x, NXFloat coeff, NXFloat dim);
    static NXFloat rubberBandClamp(NXFloat x, NXFloat coeff, NXFloat dim, NXFloat limitStart, NXFloat limitEnd);
};

}
