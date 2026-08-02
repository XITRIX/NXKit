//
// Created by Даниил Виноградов on 23.02.2025.
//

#pragma once

#include <Geometry.h>
#include <vector>

namespace NXKit {

struct IndexPath {
    IndexPath(int item, int section);
    IndexPath(): IndexPath(0, 0) {}

    [[nodiscard]] int section() const { return _indexes[0]; }
    [[nodiscard]] int item() const { return _indexes[1]; }

    friend bool operator==(const IndexPath& lhs, const IndexPath& rhs) {
        return lhs._indexes == rhs._indexes;
    }

    friend bool operator!=(const IndexPath& lhs, const IndexPath& rhs) {
        return !(lhs == rhs);
    }
private:
    std::vector<int> _indexes;
};

}
