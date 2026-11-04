#include "AppleGlassPreference.hpp"
#import <Foundation/Foundation.h>
#include <cmath>
#include <iostream>
#include <limits>

int main() { @autoreleasepool {
    if (@available(macOS 27.0, *)) {
        auto defaults=NSUserDefaults.standardUserDefaults;
        // NSArgumentDomain is volatile and process-local. Never write or
        // synchronize the user's global/application preference domains.
        NSDictionary* original=[[defaults volatileDomainForName:NSArgumentDomain] copy];
        int failures=0;
        const auto check=[&](id value,std::optional<float> expected) {
            NSMutableDictionary* domain=original ? [original mutableCopy] : [[NSMutableDictionary alloc] init];
            domain[@"NSGlassTintAmount"]=value;
            [defaults setVolatileDomain:domain forName:NSArgumentDomain];
            const auto actual=NXKit::Internal::appleGlassTintAmount();
            if(actual.has_value()!=expected.has_value() || (actual && std::abs(*actual-*expected)>.000001f)) {
                ++failures;std::cerr<<"Native glass preference bridge returned an invalid value\n";
            }
#if !__has_feature(objc_arc)
            [domain release];
#endif
        };
        for(float amount : {0.f,.25f,.5f,.75f,1.f}) check(@(amount),amount);
        check(@"0.5",std::nullopt);
        check(@(-.1),std::nullopt);
        check(@(1.1),std::nullopt);
        check(@(std::numeric_limits<double>::infinity()),std::nullopt);
        check(@(std::numeric_limits<double>::quiet_NaN()),std::nullopt);
        if(original) [defaults setVolatileDomain:original forName:NSArgumentDomain];
        else [defaults removeVolatileDomainForName:NSArgumentDomain];
#if !__has_feature(objc_arc)
        [original release];
#endif
        if(failures) return 1;
        std::cout<<"Read-only Apple glass preference tests passed\n";
    }
    return 0;
}}
