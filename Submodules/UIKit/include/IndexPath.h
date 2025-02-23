//
// Created by Даниил Виноградов on 23.02.2025.
//

#pragma once

#include <Geometry.h>

namespace NXKit {

struct IndexPath {
    IndexPath(int item, int section);
    IndexPath(): IndexPath(0, 0) {}

    int section() { return _indexes[0]; }
    int item() { return _indexes[1]; }
private:
    std::vector<int> _indexes;
};

}