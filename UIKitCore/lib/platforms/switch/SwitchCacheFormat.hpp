#pragma once

#include <algorithm>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace NXKit::Internal::SwitchCacheFormat {

inline constexpr size_t headerSize = 88;
inline constexpr size_t maximumSize = 32 * 1024 * 1024;
inline constexpr std::string_view pipelineMagic = "NXVKP001";
inline constexpr std::string_view keysMagic = "NXVKK001";

inline uint64_t checksum(std::span<const uint8_t> bytes) {
    uint64_t hash = 14695981039346656037ull;
    for (uint8_t byte : bytes) hash = (hash ^ byte) * 1099511628211ull;
    return hash;
}
inline void append64(std::vector<uint8_t>& bytes, uint64_t value) {
    for (int i = 0; i < 8; ++i) bytes.push_back(static_cast<uint8_t>(value >> (i * 8)));
}
inline uint64_t read64(std::span<const uint8_t> bytes) {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) value |= static_cast<uint64_t>(bytes[i]) << (i * 8);
    return value;
}
inline std::vector<uint8_t> encode(std::span<const uint8_t> payload,
                                  std::string_view magic, std::string_view abi) {
    if (payload.empty() || payload.size() > maximumSize || magic.size() != 8 || abi.size() != 64)
        return {};
    std::vector<uint8_t> result;
    result.reserve(headerSize + payload.size());
    result.insert(result.end(), magic.begin(), magic.end());
    result.insert(result.end(), abi.begin(), abi.end());
    append64(result, payload.size());
    append64(result, checksum(payload));
    result.insert(result.end(), payload.begin(), payload.end());
    return result;
}
inline std::span<const uint8_t> decode(std::span<const uint8_t> bytes,
                                     std::string_view magic, std::string_view abi) {
    if (magic.size() != 8 || abi.size() != 64 || bytes.size() < headerSize ||
        bytes.size() > headerSize + maximumSize ||
        !std::equal(magic.begin(), magic.end(), bytes.begin()) ||
        !std::equal(abi.begin(), abi.end(), bytes.begin() + 8)) return {};
    const auto payload = bytes.subspan(headerSize);
    if (payload.empty() || read64(bytes.subspan(72, 8)) != payload.size() ||
        read64(bytes.subspan(80, 8)) != checksum(payload)) return {};
    return payload;
}

} // namespace NXKit::Internal::SwitchCacheFormat
