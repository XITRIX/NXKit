//
//  Vector.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 28.08.2022.
//

#pragma once

#include <vector>

namespace NXKit {

template <typename T>
T pop(std::vector<T>* vector) {
    float val = vector->back();
    vector->pop_back();
    return val;
}

}
