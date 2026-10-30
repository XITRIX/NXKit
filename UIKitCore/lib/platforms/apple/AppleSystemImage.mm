#include <UIImage.h>
#include "include/utils/mac/SkCGUtils.h"
#include <TargetConditionals.h>
#include <algorithm>
#include <cmath>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

namespace NXKit::Internal {
std::shared_ptr<UIImage> appleSystemImage(const std::string& name,NXFloat pointSize,NXFloat weight,NXFloat scale) {
    @autoreleasepool {
        constexpr CGFloat weights[]={-.8,-.6,-.4,0,.23,.3,.4,.56,.62};
        const auto position=std::clamp((weight-100)/100.0,0.0,8.0);
        const int lower=std::min(7,int(position));
        const auto nativeWeight=weights[lower]+(weights[lower+1]-weights[lower])*(position-lower);
        NSString* symbolName=[NSString stringWithUTF8String:name.c_str()];
        if(!symbolName) return nullptr;
        auto convert=[&](CGImageRef image)->std::shared_ptr<UIImage> {
            auto pixels=image?SkMakeImageFromCGImage(image):nullptr;
            if(!pixels) return nullptr;
            auto result=std::make_shared<UIImage>(std::make_shared<CGImage>(std::move(pixels)),scale);
            result->setRenderModeAsTemplate(true); return result;
        };
#if TARGET_OS_IPHONE
        if(@available(iOS 13.0,*)) {
            const auto symbolWeight=static_cast<UIImageSymbolWeight>(int(std::round(position))+1);
            auto config=[UIImageSymbolConfiguration configurationWithPointSize:pointSize
                weight:symbolWeight scale:UIImageSymbolScaleMedium];
            auto symbol=[::UIImage systemImageNamed:symbolName withConfiguration:config];
            if(!symbol) return nullptr;
            UIGraphicsImageRendererFormat* format=[UIGraphicsImageRendererFormat preferredFormat];
            format.scale=scale; format.opaque=NO;
            auto renderer=[[UIGraphicsImageRenderer alloc] initWithSize:symbol.size format:format];
            auto image=[renderer imageWithActions:^(UIGraphicsImageRendererContext*) { [symbol drawAtPoint:CGPointZero]; }];
            auto result=convert(image.CGImage);
#if !__has_feature(objc_arc)
            [renderer release];
#endif
            return result;
        }
#else
        if(@available(macOS 11.0,*)) {
            auto symbol=[NSImage imageWithSystemSymbolName:symbolName accessibilityDescription:nil];
            auto config=[NSImageSymbolConfiguration configurationWithPointSize:pointSize weight:nativeWeight scale:NSImageSymbolScaleMedium];
            symbol=[symbol imageWithSymbolConfiguration:config];
            if(!symbol) return nullptr;
            auto rep=[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil
                pixelsWide:std::max(1,int(std::ceil(symbol.size.width*scale)))
                pixelsHigh:std::max(1,int(std::ceil(symbol.size.height*scale)))
                bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO
                colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];
            if(!rep) return nullptr;
            rep.size=symbol.size;
            auto context=[NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
            if(!context) {
#if !__has_feature(objc_arc)
                [rep release];
#endif
                return nullptr;
            }
            [NSGraphicsContext saveGraphicsState];
            NSGraphicsContext.currentContext=context;
            CGContextClearRect(context.CGContext,CGRectMake(0,0,symbol.size.width,symbol.size.height));
            [symbol drawInRect:NSMakeRect(0,0,symbol.size.width,symbol.size.height) fromRect:NSZeroRect
                operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];
            [context flushGraphics]; [NSGraphicsContext restoreGraphicsState];
            auto result=convert(rep.CGImage);
#if !__has_feature(objc_arc)
            [rep release];
#endif
            return result;
        }
#endif
        return nullptr;
    }
}
}
