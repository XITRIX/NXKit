//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once

struct Point {
    float x, y;

    Point(): x(0), y(0) { }
    Point(float x, float y): x(x), y(y) { }

    Point operator+(const Point& first) const {
        return Point(x + first.x, y + first.y);
    }

    Point operator-(const Point& first) const {
        return Point(x - first.x, y - first.y);
    }

    Point& operator+=(const Point& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    Point& operator-=(const Point& rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }
};

struct Size {
    float width, height;

    Size(): width(0), height(0) { }
    Size(float width, float height): width(width), height(height) { }

    Size operator+(const Size& first) const {
        return Size(width + first.width, height + first.height);
    }

    Size operator-(const Size& first) const {
        return Size(width - first.width, height - first.height);
    }

    Size& operator+=(const Size& rhs) {
        this->width += rhs.width;
        this->height += rhs.height;
        return *this;
    }

    Size& operator-=(const Size& rhs) {
        this->width -= rhs.width;
        this->height -= rhs.height;
        return *this;
    }
};

struct Rect {
    Point origin;
    Size size;

    Rect(): origin(), size() { }
    Rect(Point origin, Size size): origin(origin), size(size) { }
    Rect(float x, float y, float width, float height): origin(x, y), size(width, height) { }
};
