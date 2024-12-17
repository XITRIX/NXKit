project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_library(UIKit
        lib/CABasicAnimation.cpp
        lib/CABasicAnimationPrototype.cpp
        lib/CABlurLayer.cpp
        lib/CASpringAnimation.cpp
        lib/CASpringAnimationPrototype.cpp
        lib/CAMediaTimingFunction.cpp
        lib/CATextLayer.cpp
        lib/CATransaction.cpp
        lib/ContentsGravityTransformation.cpp
        lib/platforms/SkiaCtx.cpp
        lib/Application.cpp
        lib/DispatchQueue.cpp
        lib/Timer.cpp
        lib/Geometry.cpp
        lib/CGImage.cpp
        lib/CADisplayLink.cpp
        lib/CALayer.cpp
        lib/UIApplication.cpp
        lib/UIApplicationDelegateImpl.cpp
        lib/UIApplicationMain.cpp
        lib/UIBlurView.cpp
        lib/UIColor.cpp
        lib/UIEvent.cpp
        lib/UIGestureRecognizer.cpp
        lib/UIImage.cpp
        lib/UIImageView.cpp
        lib/UIKey.cpp
        lib/UILabel.cpp
        lib/UIPress.cpp
        lib/UIPressesEvent.cpp
        lib/UIResponder.cpp
        lib/UITapGestureRecognizer.cpp
        lib/UITouch.cpp
        lib/UIView.cpp
        lib/UIViewAnimationGroup.cpp
        lib/UIViewAnimationOptions.cpp
        lib/UIViewController.cpp
        lib/UIWindow.cpp
        lib/NXAffineTransform.cpp
        lib/NXTransform3D.cpp
        lib/NXData.cpp
)

# APPLE
if (APPLE)
    target_sources(UIKit PRIVATE
            lib/platforms/SkiaCtx_sdlBase.cpp
    )

    # MAC
    if (PLATFORM_DESKTOP)
        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libwindow.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libicu.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskparagraph.a
        )

        target_sources(UIKit PRIVATE
                lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    # IOS
    elseif (PLATFORM_IOS)
        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libwindow.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libicu.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskparagraph.a
        )

        target_sources(UIKit PRIVATE
                lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
endif ()

target_include_directories(UIKit PUBLIC
        ${EXTERN_PATH}/SDL/include
        ${EXTERN_PATH}/skia
        ${EXTERN_PATH}/libromfs/lib/include
        include
        lib
)

if (PLATFORM_MAC)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/mac/Headers)
elseif (PLATFORM_IOS)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/ios/MetalANGLE.framework/Headers)
endif ()

if (APPLE)
    list(APPEND platform_libs "-framework Foundation" "-framework VideoToolbox" "-framework AVKit" "-framework MetalKit" "-framework CoreText")
endif ()

target_link_libraries(UIKit PRIVATE ${platform_libs})