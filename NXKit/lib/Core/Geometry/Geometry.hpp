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

    float width();
    float height();

    float minX();
    float midX();
    float maxX();

    float minY();
    float midY();
    float maxY();

    bool contains(Point point);
    Rect& insetBy(const UIEdgeInsets& insets);

    bool operator==(const Rect& rhs);

    bool valid();
};

}
