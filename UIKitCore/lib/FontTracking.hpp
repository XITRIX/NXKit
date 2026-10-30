#pragma once
#include "include/core/SkData.h"
#include "include/core/SkTypeface.h"
#include <cmath>
#include <cstdint>

namespace NXKit::Internal {

// Normal horizontal tracking, in points. Skia deliberately removes Core Text
// tracking from glyph advances, and HarfBuzz's paragraph path doesn't restore
// the font's AAT 'trak' table. Read the font's own size-dependent values.
// Format: https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6trak.html
inline float normalFontTracking(const uint8_t* bytes, size_t size, int em, float points) {
    if (!bytes || size < 12 || !std::isfinite(points) || points <= 0 || em <= 0) return 0;
    const auto u16 = [&](size_t p) { return uint16_t((uint16_t(bytes[p]) << 8) | bytes[p+1]); };
    const auto u32 = [&](size_t p) { return (uint32_t(u16(p)) << 16) | u16(p+2); };
    if (u32(0) != 0x10000 || u16(4) != 0) return 0;
    const size_t horizontal = u16(6);
    if (!horizontal || horizontal > size - 8) return 0;
    const size_t tracks = u16(horizontal), sizes = u16(horizontal+2), sizeOffset = u32(horizontal+4);
    if (!tracks || !sizes || tracks > (size-horizontal-8)/8 || sizeOffset > size || sizes > (size-sizeOffset)/4) return 0;
    size_t values = 0;
    for (size_t i=0; i<tracks; ++i) {
        const auto entry = horizontal+8+i*8;
        if (u32(entry) == 0) { values = u16(entry+6); break; }
    }
    if (!values || values > size || sizes > (size-values)/2) return 0;
    const auto pointAt = [&](size_t i) { return int32_t(u32(sizeOffset+i*4))/65536.0; };
    const auto valueAt = [&](size_t i) { return int16_t(u16(values+i*2)); };
    for (size_t i=0; i<sizes; ++i)
        if (pointAt(i) <= 0 || (i && pointAt(i) <= pointAt(i-1))) return 0;
    double units = valueAt(0);
    if (sizes > 1) {
        size_t upper = 1;
        while (upper+1 < sizes && pointAt(upper) < points) ++upper;
        const double fraction = (points-pointAt(upper-1))/(pointAt(upper)-pointAt(upper-1));
        units = valueAt(upper-1) + fraction*(valueAt(upper)-valueAt(upper-1));
    }
    return float(units*points/em);
}

inline float fontTracking(const SkTypeface& face, float points) {
    const auto data = face.copyTableData(SkSetFourByteTag('t','r','a','k'));
    return data ? normalFontTracking(data->bytes(), data->size(), face.getUnitsPerEm(), points) : 0;
}

}
