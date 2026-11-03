#pragma once
#include <cstdint>
namespace NXKit {
enum class UIRectCorner : std::uint8_t {
    none = 0, topLeft = 1 << 0, topRight = 1 << 1,
    bottomLeft = 1 << 2, bottomRight = 1 << 3, allCorners = 15
};
constexpr UIRectCorner operator|(UIRectCorner a, UIRectCorner b) {
    return static_cast<UIRectCorner>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
}
