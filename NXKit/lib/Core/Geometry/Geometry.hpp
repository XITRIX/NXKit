//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once
#include <math.h>
#include <Core/UIEdgeInsets/UIEdgeInsets.hpp>
#include <Core/NXTransform3D/NXTransform3D.hpp>

namespace NXKit {

struct Point {
    float x, y;

    Point();
    Point(float x, float y);

    bool operator==(const Point& rhs) const;
    Point operator+(const Point& first) const;
    Point operator-(const Point& first) const;
    Point& operator+=(const Point& rhs);
    Point& operator-=(const Point& rhs);
    Point operator/(const float& rhs);
    Point operator*(const float& rhs);

    Point applying(const NXAffineTransform& t) const;

    bool valid();
    float magnitude();
};

struct Size {
    float width, height;

    Size();
    Size(float width, float height);

    bool operator==(const Size& rhs) const;
    Size operator+(const Size& first) const;
    Size operator-(const Size& first) const;
    Size& operator+=(const Size& rhs);
    Size& operator-=(const Size& rhs);

    bool valid();
    Size inset(UIEdgeInsets inset) const;
};

struct Rect {
    Point origin;
    Size size;

    Rect();
    Rect(Point origin, Size size);
    Rect(float x, float y, float width, float height);

    float width() const;
    float height() const;

    float minX() const;
    float midX() const;
    float maxX() const;

    float minY() const;
    float midY() const;
    float maxY() const;

    bool contains(Point point);
    bool intersects(const Rect& other) const;
    Rect intersection(const Rect& other) const;
    Rect& offsetBy(const Point& offset);
    Rect& offsetBy(const float& offsetX, const float& offsetY);
    Rect& insetBy(const UIEdgeInsets& insets);
    Rect applying(const NXAffineTransform& t) const;
    Rect applying(const NXTransform3D& t) const;

    bool operator==(const Rect& rhs) const;

    bool valid();

    static Rect zero;
    static Rect null;
};

struct IndexPath {
    IndexPath(int row, int section);
    IndexPath(): IndexPath(0, 0) {}

    int row() const;
    int section() const;
    int item() const;

    IndexPath prev() const;
    IndexPath next(int rowLimit) const;

    bool operator<(const IndexPath &o) const;
    bool operator==(const IndexPath &o) const;
private:
    int a[2];
};

}
