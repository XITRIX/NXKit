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

}