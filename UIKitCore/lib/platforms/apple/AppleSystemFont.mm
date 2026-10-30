#include "AppleSystemFont.hpp"
#include "include/ports/SkTypeface_mac.h"
#include <TargetConditionals.h>
#include <algorithm>
#include <cmath>
#if TARGET_OS_IPHONE
#import <UIKit/UIFont.h>
#else
#import <AppKit/NSFont.h>
#endif

namespace NXKit::Internal {
namespace {
// CSS/Skia weights to UIFontWeight/NSFontWeight. Preserve intermediate weights.
CGFloat nativeWeight(int value) {
    constexpr CGFloat weights[] = {-.8, -.6, -.4, 0, .23, .3, .4, .56, .62};
    const auto position = std::clamp((value - 100) / 100.0, 0.0, 8.0);
    const int lower = std::min(7, int(position));
    return weights[lower] + (weights[lower + 1] - weights[lower]) * (position - lower);
}
}
sk_sp<SkTypeface> appleSystemTypeface(const SkFontStyle& style, float pointSize) {
    if (!std::isfinite(pointSize) || pointSize <= 0) return nullptr;
    @autoreleasepool {
#if TARGET_OS_IPHONE
    auto font = [UIFont systemFontOfSize:pointSize weight:nativeWeight(style.weight())];
#else
    auto font = [NSFont systemFontOfSize:pointSize weight:nativeWeight(style.weight())];
#endif
    CTFontSymbolicTraits traits = 0;
    if (style.slant() != SkFontStyle::kUpright_Slant) traits |= kCTFontItalicTrait;
    if (style.width() < SkFontStyle::kNormal_Width) traits |= kCTFontCondensedTrait;
    if (style.width() > SkFontStyle::kNormal_Width) traits |= kCTFontExpandedTrait;
    if (traits) {
        auto variant = CTFontCreateCopyWithSymbolicTraits((__bridge CTFontRef)font,
            pointSize, nullptr, traits, kCTFontItalicTrait | kCTFontCondensedTrait | kCTFontExpandedTrait);
        if (variant) {
            auto face = SkMakeTypefaceFromCTFont(variant);
            CFRelease(variant);
            return face;
        }
    }
    return SkMakeTypefaceFromCTFont((__bridge CTFontRef)font);
    }
}
}
