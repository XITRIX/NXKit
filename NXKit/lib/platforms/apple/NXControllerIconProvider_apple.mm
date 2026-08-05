#include "NXControllerIconProvider.h"

#include <CGImage.h>
#include <UIImage.h>

#include <include/utils/mac/SkCGUtils.h>

#include <GameController/GameController.h>
#if defined(PLATFORM_IOS)
#include <UIKit/UIKit.h>
#else
#include <AppKit/AppKit.h>
#endif

#include <algorithm>
#include <cmath>

namespace NXKit {

namespace {

#if __has_feature(objc_arc)
#define NX_AUTORELEASE(value) (value)
#else
#define NX_AUTORELEASE(value) [(value) autorelease]
#endif

NXControllerType controllerTypeForGCController(GCController* controller) {
    if (!controller) {
        return NXControllerType::generic;
    }

    NSString* identity = [NSString stringWithFormat:
        @"%@ %@",
        controller.productCategory ?: @"",
        controller.vendorName ?: @""
    ];
    identity = identity.lowercaseString;

    if ([identity containsString:@"xbox"]
        || [identity containsString:@"microsoft"]) {
        return NXControllerType::xbox;
    }
    if ([identity containsString:@"dualshock"]
        || [identity containsString:@"dualsense"]
        || [identity containsString:@"playstation"]
        || [identity containsString:@"sony"]) {
        return NXControllerType::playStation;
    }
    if ([identity containsString:@"nintendo"]
        || [identity containsString:@"switch"]
        || [identity containsString:@"joy-con"]
        || [identity containsString:@"gamecube"]) {
        return NXControllerType::nintendoSwitch;
    }
    return NXControllerType::generic;
}

GCController* controllerForType(NXControllerType controllerType) {
    NSArray<GCController*>* controllers = GCController.controllers;
    if (controllerType == NXControllerType::generic) {
        return nil;
    }

    for (GCController* controller in controllers) {
        if (controllerTypeForGCController(controller) == controllerType) {
            return controller;
        }
    }

    // SDL may know a device more precisely than GameController's public
    // product strings. A sole controller is still the best symbol authority.
    return controllers.count == 1 ? controllers.firstObject : nil;
}

GCControllerElement* elementForButton(
    GCController* controller,
    NXActionButton button
) {
    GCExtendedGamepad* gamepad = controller.extendedGamepad;
    if (!gamepad) {
        return nil;
    }

    switch (button) {
        case NXActionButton::a: return gamepad.buttonA;
        case NXActionButton::b: return gamepad.buttonB;
        case NXActionButton::x: return gamepad.buttonX;
        case NXActionButton::y: return gamepad.buttonY;
        case NXActionButton::plus: return gamepad.buttonMenu;
        case NXActionButton::minus: return gamepad.buttonOptions;
        case NXActionButton::home: return gamepad.buttonHome;
        case NXActionButton::leftThumbstick:
            return gamepad.leftThumbstickButton;
        case NXActionButton::rightThumbstick:
            return gamepad.rightThumbstickButton;
        case NXActionButton::leftShoulder: return gamepad.leftShoulder;
        case NXActionButton::rightShoulder: return gamepad.rightShoulder;
        case NXActionButton::leftTrigger: return gamepad.leftTrigger;
        case NXActionButton::rightTrigger: return gamepad.rightTrigger;
        case NXActionButton::dpadUp: return gamepad.dpad.up;
        case NXActionButton::dpadDown: return gamepad.dpad.down;
        case NXActionButton::dpadLeft: return gamepad.dpad.left;
        case NXActionButton::dpadRight: return gamepad.dpad.right;
        case NXActionButton::misc1:
            // SDL maps the PS5 microphone button to misc1, while Apple's share
            // element represents a different PlayStation control.
            if (controllerTypeForGCController(controller)
                == NXControllerType::playStation) {
                return nil;
            }
            if (@available(macOS 12.0, iOS 15.0, *)) {
                return gamepad[GCInputButtonShare];
            }
            return nil;
        case NXActionButton::rightPaddle1:
            if (@available(macOS 14.4, iOS 17.4, *)) {
                return gamepad[GCInputBackRightButton(0)];
            }
            return gamepad[GCInputXboxPaddleOne];
        case NXActionButton::leftPaddle1:
            if (@available(macOS 14.4, iOS 17.4, *)) {
                return gamepad[GCInputBackLeftButton(0)];
            }
            return gamepad[GCInputXboxPaddleThree];
        case NXActionButton::rightPaddle2:
            if (@available(macOS 14.4, iOS 17.4, *)) {
                return gamepad[GCInputBackRightButton(1)];
            }
            return gamepad[GCInputXboxPaddleTwo];
        case NXActionButton::leftPaddle2:
            if (@available(macOS 14.4, iOS 17.4, *)) {
                return gamepad[GCInputBackLeftButton(1)];
            }
            return gamepad[GCInputXboxPaddleFour];
        case NXActionButton::touchpad:
            return gamepad[GCInputDualShockTouchpadButton];
        case NXActionButton::misc2:
        case NXActionButton::misc3:
        case NXActionButton::misc4:
        case NXActionButton::misc5:
        case NXActionButton::misc6:
            return nil;
    }
    return nil;
}

NSString* fallbackSymbolName(
    NXActionButton button,
    NXControllerType controllerType
) {
    if (controllerType == NXControllerType::playStation) {
        switch (button) {
            case NXActionButton::a: return @"xmark.circle";
            case NXActionButton::b: return @"circle.circle";
            case NXActionButton::x: return @"square.circle";
            case NXActionButton::y: return @"triangle.circle";
            default: break;
        }
    }

    if (controllerType == NXControllerType::nintendoSwitch) {
        switch (button) {
            case NXActionButton::a: return @"b.circle";
            case NXActionButton::b: return @"a.circle";
            case NXActionButton::x: return @"y.circle";
            case NXActionButton::y: return @"x.circle";
            default: break;
        }
    }

    switch (button) {
        case NXActionButton::a: return @"a.circle";
        case NXActionButton::b: return @"b.circle";
        case NXActionButton::x: return @"x.circle";
        case NXActionButton::y: return @"y.circle";
        case NXActionButton::plus:
            return controllerType == NXControllerType::nintendoSwitch
                ? @"plus.circle" : @"line.3.horizontal.circle";
        case NXActionButton::minus:
            return controllerType == NXControllerType::nintendoSwitch
                ? @"minus.circle" : @"rectangle.on.rectangle.circle";
        case NXActionButton::home:
            if (controllerType == NXControllerType::xbox) {
                return @"xbox.logo";
            }
            if (controllerType == NXControllerType::playStation) {
                return @"playstation.logo";
            }
            return @"house.circle";
        case NXActionButton::leftThumbstick:
            return @"l.joystick.press.down";
        case NXActionButton::rightThumbstick:
            return @"r.joystick.press.down";
        case NXActionButton::leftShoulder:
            if (controllerType == NXControllerType::xbox) {
                return @"lb.button.roundedbottom.horizontal";
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return @"l.button.roundedbottom.horizontal";
            }
            return @"l1.button.roundedbottom.horizontal";
        case NXActionButton::rightShoulder:
            if (controllerType == NXControllerType::xbox) {
                return @"rb.button.roundedbottom.horizontal";
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return @"r.button.roundedbottom.horizontal";
            }
            return @"r1.button.roundedbottom.horizontal";
        case NXActionButton::leftTrigger:
            if (controllerType == NXControllerType::xbox) {
                return @"lt.button.roundedtop.horizontal";
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return @"zl.button.roundedtop.horizontal";
            }
            return @"l2.button.roundedtop.horizontal";
        case NXActionButton::rightTrigger:
            if (controllerType == NXControllerType::xbox) {
                return @"rt.button.roundedtop.horizontal";
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return @"zr.button.roundedtop.horizontal";
            }
            return @"r2.button.roundedtop.horizontal";
        case NXActionButton::dpadUp: return @"dpad.up.filled";
        case NXActionButton::dpadDown: return @"dpad.down.filled";
        case NXActionButton::dpadLeft: return @"dpad.left.filled";
        case NXActionButton::dpadRight: return @"dpad.right.filled";
        case NXActionButton::misc1:
            if (controllerType == NXControllerType::playStation) {
                return @"mic.circle";
            }
            if (controllerType == NXControllerType::nintendoSwitch) {
                return @"viewfinder.circle";
            }
            return @"square.and.arrow.up.circle";
        case NXActionButton::rightPaddle1: return @"p1.button.horizontal";
        case NXActionButton::leftPaddle1: return @"p3.button.horizontal";
        case NXActionButton::rightPaddle2: return @"p2.button.horizontal";
        case NXActionButton::leftPaddle2: return @"p4.button.horizontal";
        case NXActionButton::touchpad: return @"rectangle.inset.filled";
        case NXActionButton::misc2: return @"2.circle";
        case NXActionButton::misc3: return @"3.circle";
        case NXActionButton::misc4: return @"4.circle";
        case NXActionButton::misc5: return @"5.circle";
        case NXActionButton::misc6: return @"6.circle";
    }
    return nil;
}

std::shared_ptr<UIImage> imageFromCGImage(CGImageRef image, NXFloat scale) {
    if (!image) {
        return nullptr;
    }
    auto skiaImage = SkMakeImageFromCGImage(image);
    if (!skiaImage) {
        return nullptr;
    }

    auto result = std::make_shared<UIImage>(
        std::make_shared<CGImage>(std::move(skiaImage)),
        scale
    );
    result->setRenderModeAsTemplate(true);
    return result;
}

#if defined(PLATFORM_IOS)
std::shared_ptr<UIImage> renderSystemSymbol(
    NSString* name,
    NXFloat pointSize,
    NXFloat scale
) {
    UIImageSymbolConfiguration* configuration =
        [UIImageSymbolConfiguration configurationWithPointSize:pointSize
                                                        weight:UIImageSymbolWeightSemibold
                                                         scale:UIImageSymbolScaleMedium];
    ::UIImage* symbol = [::UIImage systemImageNamed:name
                                  withConfiguration:configuration];
    if (!symbol) {
        return nullptr;
    }

    const CGSize canvasSize = CGSizeMake(pointSize, pointSize);
    UIGraphicsBeginImageContextWithOptions(canvasSize, NO, scale);
    const CGSize symbolSize = symbol.size;
    const auto fitScale = std::min(
        pointSize / std::max<CGFloat>(symbolSize.width, 1),
        pointSize / std::max<CGFloat>(symbolSize.height, 1)
    );
    const CGSize fittedSize = CGSizeMake(
        symbolSize.width * fitScale,
        symbolSize.height * fitScale
    );
    const CGRect destination = CGRectMake(
        (pointSize - fittedSize.width) / 2,
        (pointSize - fittedSize.height) / 2,
        fittedSize.width,
        fittedSize.height
    );
    [symbol drawInRect:destination];
    ::UIImage* rendered = UIGraphicsGetImageFromCurrentImageContext();
    CGImageRef renderedImage = rendered.CGImage;
    auto result = imageFromCGImage(renderedImage, scale);
    UIGraphicsEndImageContext();
    return result;
}
#else
std::shared_ptr<UIImage> renderSystemSymbol(
    NSString* name,
    NXFloat pointSize,
    NXFloat scale
) {
    NSImage* symbol = [NSImage imageWithSystemSymbolName:name
                               accessibilityDescription:nil];
    if (!symbol) {
        return nullptr;
    }
    NSImageSymbolConfiguration* configuration =
        [NSImageSymbolConfiguration configurationWithPointSize:pointSize
                                                        weight:NSFontWeightSemibold
                                                         scale:NSImageSymbolScaleMedium];
    symbol = [symbol imageWithSymbolConfiguration:configuration];
    if (!symbol) {
        return nullptr;
    }

    const auto pixelSize = std::max(
        1,
        static_cast<int>(std::ceil(pointSize * scale))
    );
    NSBitmapImageRep* representation = NX_AUTORELEASE(
        [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:nil
                          pixelsWide:pixelSize
                          pixelsHigh:pixelSize
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSDeviceRGBColorSpace
                         bytesPerRow:0
                        bitsPerPixel:0]
    );
    if (!representation) {
        return nullptr;
    }
    representation.size = NSMakeSize(pointSize, pointSize);

    NSGraphicsContext* context =
        [NSGraphicsContext graphicsContextWithBitmapImageRep:representation];
    if (!context) {
        return nullptr;
    }

    [NSGraphicsContext saveGraphicsState];
    NSGraphicsContext.currentContext = context;
    CGContextClearRect(context.CGContext, CGRectMake(0, 0, pointSize, pointSize));

    const NSSize symbolSize = symbol.size;
    const auto fitScale = std::min(
        pointSize / std::max<CGFloat>(symbolSize.width, 1),
        pointSize / std::max<CGFloat>(symbolSize.height, 1)
    );
    const NSSize fittedSize = NSMakeSize(
        symbolSize.width * fitScale,
        symbolSize.height * fitScale
    );
    const NSRect destination = NSMakeRect(
        (pointSize - fittedSize.width) / 2,
        (pointSize - fittedSize.height) / 2,
        fittedSize.width,
        fittedSize.height
    );
    [symbol drawInRect:destination
              fromRect:NSZeroRect
             operation:NSCompositingOperationSourceOver
              fraction:1
        respectFlipped:YES
                 hints:nil];
    [context flushGraphics];
    [NSGraphicsContext restoreGraphicsState];

    return imageFromCGImage(representation.CGImage, scale);
}
#endif

class AppleControllerIconProvider final : public NXControllerIconProvider {
public:
    std::shared_ptr<UIImage> iconForButton(
        NXActionButton button,
        NXControllerType controllerType,
        NXFloat pointSize,
        NXFloat scale
    ) const override {
        @autoreleasepool {
            NSString* symbolName = nil;
            if (@available(macOS 11.0, iOS 14.0, *)) {
                GCController* controller = controllerForType(controllerType);
                GCControllerElement* element = elementForButton(controller, button);
                symbolName = element.sfSymbolsName
                    ?: element.unmappedSfSymbolsName;
            }
            symbolName = symbolName
                ?: fallbackSymbolName(button, controllerType);
            return renderSystemSymbol(symbolName, pointSize, scale);
        }
    }
};

} // namespace

std::unique_ptr<NXControllerIconProvider>
NXMakePlatformControllerIconProvider() {
    return std::make_unique<AppleControllerIconProvider>();
}

} // namespace NXKit
