#pragma once

#include <cstdint>

namespace NXKit {

/** Directional edge option set, with the values from Apple's NSDirectionalRectEdge. */
enum class NSDirectionalRectEdge : std::uint8_t {
    none = 0,
    top = 1 << 0,
    leading = 1 << 1,
    bottom = 1 << 2,
    trailing = 1 << 3,
    all = top | leading | bottom | trailing,
};

constexpr NSDirectionalRectEdge operator|(NSDirectionalRectEdge a, NSDirectionalRectEdge b) {
    return static_cast<NSDirectionalRectEdge>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
constexpr NSDirectionalRectEdge operator&(NSDirectionalRectEdge a, NSDirectionalRectEdge b) {
    return static_cast<NSDirectionalRectEdge>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}
constexpr NSDirectionalRectEdge& operator|=(NSDirectionalRectEdge& a, NSDirectionalRectEdge b) {
    return a = a | b;
}

}
