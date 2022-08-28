//
//  Geometry.h
//  NanoVG
//
//  Created by Даниил Виноградов on 09.06.2022.
//

#pragma once
#include <math.h>

namespace NXKit {

struct Point {
    float x, y;

    Point(): x(0), y(0) { }
    Point(float x, float y): x(x), y(y) { }

    bool operator==(const Point& rhs) {
        return this->x == rhs.x && this->y == rhs.y;
    }

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

    Point operator/(const float& rhs) {
        auto res = *this;
        res.x /= rhs;
        res.y /= rhs;
        return res;
    }

    Point operator*(const float& rhs) {
        auto res = *this;
        res.x *= rhs;
        res.y *= rhs;
        return res;
    }

    bool valid() {
        return !isnan(this->x) && !isnan(this->y);
    }

    float magnitude() {
        return sqrt(x * x + y * y);
    }
};

struct Size {
    float width, height;

    Size(): width(0), height(0) { }
    Size(float width, float height): width(width), height(height) { }

    bool operator==(const Size& rhs) {
        return this->width == rhs.width && this->height == rhs.height;
    }

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

    bool valid() {
        return !isnan(this->width) && !isnan(this->height);
    }
};

struct Rect {
    Point origin;
    Size size;

    Rect(): origin(), size() { }
    Rect(Point origin, Size size): origin(origin), size(size) { }
    Rect(float x, float y, float width, float height): origin(x, y), size(width, height) { }

    float width() { return size.width; }
    float height() { return size.height; }

    float minX() { return origin.x; }
    float midX() { return origin.x + size.width / 2; }
    float maxX() { return origin.x + size.width; }

    float minY() { return origin.y; }
    float midY() { return origin.y + size.height / 2; }
    float maxY() { return origin.y + size.height; }

    bool contains(Point point) {
        return
        (point.x >= minX()) && (point.x < maxX()) &&
        (point.y >= minY()) && (point.y < maxY());
    }

    bool operator==(const Rect& rhs) {
        return
        this->origin.x == rhs.origin.x && this->origin.y == rhs.origin.y &&
        this->size.width == rhs.size.width && this->size.height == rhs.size.height;
    }

    bool valid() {
        return this->origin.valid() && this->size.valid();
    }
};

}
