//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once
#include <math.h>
#include <Core/UIEdgeInsets/UIEdgeInsets.hpp>

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

    bool valid();
    float magnitude();
};

struct Size {
    float width, height;

    Size();
    Size(float width, float height);

    bool operator==(const Size& rhs);
    Size operator+(const Size& first) const;
    Size operator-(const Size& first) const;
    Size& operator+=(const Size& rhs);
    Size& operator-=(const Size& rhs);

    bool valid();
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
    Rect& offsetBy(const Point& offet);
    Rect& offsetBy(const float& offetX, const float& offetY);
    Rect& insetBy(const UIEdgeInsets& insets);

    bool operator==(const Rect& rhs);

    bool valid();
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
