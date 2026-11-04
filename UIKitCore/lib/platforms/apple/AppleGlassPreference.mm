#include "AppleGlassPreference.hpp"
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>
#include <cmath>
#include <dlfcn.h>

std::optional<float> NXKit::Internal::appleGlassTintAmount() {
    NSNumber* value=nil;
#if TARGET_OS_OSX
    if (@available(macOS 27.0, *)) {
        // AppKit's systemLegibility reads this inherited defaults key. This
        // reads Foundation's cache; never synchronize preferences in a frame.
        id candidate=[NSUserDefaults.standardUserDefaults objectForKey:@"NSGlassTintAmount"];
        if ([candidate isKindOfClass:NSNumber.class]) value=candidate;
    }
#elif TARGET_OS_IOS
    if (@available(iOS 27.0, *)) {
        // Private exported bridge; no hard link or guessed preferences domain.
        // The return type is NSNumber*, not a scalar. UIKit maintains this
        // cached value and refreshes it on its glass-preference notification.
        using Getter=NSNumber* (*)(void);
        static const auto getter=reinterpret_cast<Getter>(dlsym(RTLD_DEFAULT,"UIViewGlassGetTintAmount"));
        if (getter) {
            const auto candidate=getter();
            if ([candidate isKindOfClass:NSNumber.class]) value=candidate;
        }
    }
#endif
    if (!value) return std::nullopt;
    const auto amount=value.doubleValue;
    if (!std::isfinite(amount) || amount<0 || amount>1) return std::nullopt;
    return static_cast<float>(amount);
}
