//
//  Tools.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#include <vector>
#include <deque>
#include <math.h>

namespace NXKit {

#define IFNNULL(prop, val)  \
{                           \
    float v = val;          \
    if (!isnan(v))          \
        prop = v;           \
}

#define IFNNULLOR(prop, val, orr)  \
{                           \
    float v = val;          \
    if (!isnan(v))          \
        prop = v;           \
    else                    \
        prop = orr;         \
}

template <typename T>
T pop(std::vector<T>* vector) {
    T val = vector->back();
    vector->pop_back();
    return val;
}

template <typename T>
T pop(std::deque<T>* vector) {
    T val = vector->front();
    vector->pop_front();
    return val;
}

float ntz(float value);

bool floatSame(float a, float b, float epsilon = 1);

float minmax(float min, float val, float max);

float isEqual(float val1, float val2);

}
