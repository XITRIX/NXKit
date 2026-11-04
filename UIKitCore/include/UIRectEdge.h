#pragma once

#include <cstdint>

namespace NXKit {
enum class UIRectEdge : std::uint8_t {
    none = 0, top = 1 << 0, left = 1 << 1,
    bottom = 1 << 2, right = 1 << 3, all = 15,
};
constexpr UIRectEdge operator|(UIRectEdge a, UIRectEdge b) {
    return static_cast<UIRectEdge>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
constexpr UIRectEdge operator&(UIRectEdge a, UIRectEdge b) {
    return static_cast<UIRectEdge>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}
constexpr UIRectEdge& operator|=(UIRectEdge& a, UIRectEdge b) { return a = a | b; }
}
