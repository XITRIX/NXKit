#include "platforms/switch/SwitchCacheFormat.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace NXKit::Internal::SwitchCacheFormat;

static void expect(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    const std::string abi(64, 'a');
    const std::array<uint8_t, 5> bytes{0, 1, 127, 128, 255};
    const auto encoded = encode(bytes, pipelineMagic, abi);
    const auto decoded = decode(encoded, pipelineMagic, abi);
    expect(std::equal(bytes.begin(), bytes.end(), decoded.begin(), decoded.end()), "cache round trip");
    expect(decode(encoded, keysMagic, abi).empty(), "reject wrong cache kind");
    expect(decode(encoded, pipelineMagic, std::string(64, 'b')).empty(), "reject stale shader ABI");
    for (size_t size = 0; size < encoded.size(); ++size)
        expect(decode(std::span(encoded).first(size), pipelineMagic, abi).empty(), "reject truncation");
    for (size_t index = 0; index < encoded.size(); ++index) {
        auto corrupt = encoded;
        corrupt[index] ^= 0x80;
        expect(decode(corrupt, pipelineMagic, abi).empty(), "reject corruption");
    }
    auto trailing = encoded;
    trailing.push_back(0);
    expect(decode(trailing, pipelineMagic, abi).empty(), "reject trailing data");
    expect(encode({}, pipelineMagic, abi).empty(), "reject empty payload");
    expect(encode(bytes, pipelineMagic, "short").empty(), "reject malformed ABI");
    auto hugeLength = encoded;
    for (size_t index = 72; index < 80; ++index) hugeLength[index] = 255;
    expect(decode(hugeLength, pipelineMagic, abi).empty(), "reject overflowing declared size");
    std::cout << "PASS: cache round trip, kind/ABI, truncation, corruption and size bounds\n";
}
