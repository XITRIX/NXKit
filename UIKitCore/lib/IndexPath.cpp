//
// Created by Даниил Виноградов on 23.02.2025.
//

#include <IndexPath.h>

using namespace NXKit;

IndexPath::IndexPath(int item, int section) {
    _indexes.push_back(section);
    _indexes.push_back(item);
}
