project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_library(UIKit
        UIKitCore/lib/tools/Logger.cpp
        UIKitCore/lib/tools/IBTools.cpp
        UIKitCore/lib/tools/Fatal.cpp
        UIKitCore/lib/tools/Tools.cpp
        UIKitCore/lib/CABasicAnimation.cpp
        UIKitCore/lib/CABasicAnimationPrototype.cpp
        UIKitCore/lib/CABlurLayer.cpp
        UIKitCore/lib/CASpringAnimation.cpp
        UIKitCore/lib/CASpringAnimationPrototype.cpp
        UIKitCore/lib/CAMediaTimingFunction.cpp
        UIKitCore/lib/CATextLayer.cpp
        UIKitCore/lib/CATransaction.cpp
        UIKitCore/lib/ContentsGravityTransformation.cpp
        UIKitCore/lib/platforms/SkiaCtx.cpp
        UIKitCore/lib/Application.cpp
        UIKitCore/lib/DispatchQueue.cpp
        UIKitCore/lib/Timer.cpp
        UIKitCore/lib/Geometry.cpp
        UIKitCore/lib/IndexPath.cpp
        UIKitCore/lib/CGImage.cpp
        UIKitCore/lib/CADisplayLink.cpp
        UIKitCore/lib/CALayer.cpp
        UIKitCore/lib/YGLayout.cpp
        UIKitCore/lib/UIAction.cpp
        UIKitCore/lib/UIApplication.cpp
        UIKitCore/lib/UIApplicationDelegateImpl.cpp
        UIKitCore/lib/UIApplicationMain.cpp
        UIKitCore/lib/UIBlurView.cpp
        UIKitCore/lib/UIButton.cpp
        UIKitCore/lib/UIColor.cpp
        UIKitCore/lib/UIControl.cpp
        UIKitCore/lib/UIControlGestureRecognizer.cpp
        UIKitCore/lib/UIEdgeInsets.cpp
        UIKitCore/lib/UIEvent.cpp
        UIKitCore/lib/UIFocus.cpp
        UIKitCore/lib/UIFocusAnimationCoordinator.cpp
        UIKitCore/lib/UIFocusSystem.cpp
        UIKitCore/lib/UIGestureRecognizer.cpp
        UIKitCore/lib/UIImage.cpp
        UIKitCore/lib/UIImageView.cpp
        UIKitCore/lib/UIKey.cpp
        UIKitCore/lib/UILabel.cpp
        UIKitCore/lib/UINib.cpp
        UIKitCore/lib/UIPanGestureRecognizer.cpp
        UIKitCore/lib/UIPress.cpp
        UIKitCore/lib/UIPressesEvent.cpp
        UIKitCore/lib/UIResponder.cpp
        UIKitCore/lib/UIScrollView.cpp
        UIKitCore/lib/UITapGestureRecognizer.cpp
        UIKitCore/lib/UITouch.cpp
        UIKitCore/lib/UITraitCollection.cpp
        UIKitCore/lib/UITraitEnvironment.cpp
        UIKitCore/lib/UIView.cpp
        UIKitCore/lib/UIViewAnimationGroup.cpp
        UIKitCore/lib/UIViewAnimationOptions.cpp
        UIKitCore/lib/UIViewController.cpp
        UIKitCore/lib/UIWindow.cpp
        UIKitCore/lib/NXAffineTransform.cpp
        UIKitCore/lib/NXTransform3D.cpp
        UIKitCore/lib/NXData.cpp
        UIKitCore/lib/UIScrollViewExtensions/DecelerationTimingParameters.cpp
        UIKitCore/lib/UIScrollViewExtensions/RubberBand.cpp
        UIKitCore/lib/UIScrollViewExtensions/SpringTimingParameters.cpp
        UIKitCore/lib/UIScrollViewExtensions/TimerAnimation.cpp
        UIKitCore/lib/SkTools/EventTracingPriv.cpp
        UIKitCore/lib/SkTools/SkDebugfTracer.cpp
)

list(APPEND platform_libs yogacore tinyxml2)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/yoga)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/tinyxml2)

# APPLE
if (APPLE)
    target_sources(UIKit PRIVATE
            UIKitCore/lib/platforms/SkiaCtx_sdlBase.cpp
    )

    # MAC
    if (PLATFORM_DESKTOP)
        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskparagraph.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libicu.a
        )

        target_sources(UIKit PRIVATE
                UIKitCore/lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    # IOS
    elseif (PLATFORM_IOS)
        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskparagraph.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libicu.a
        )

        target_sources(UIKit PRIVATE
                UIKitCore/lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
elseif (PLATFORM_SWITCH)
        target_sources(UIKit PUBLIC
                UIKitCore/lib/platforms/SkiaCtx_sdlBase.cpp
                UIKitCore/lib/platforms/switch/SkiaCtx_switch.cpp
                UIKitCore/lib/platforms/switch/switch_wrapper.c
        )

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        message(STATUS "SDL2 Path: \"${SDL2_INCLUDE_DIRS}\"")
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

        # GPU
        target_link_libraries(UIKit PUBLIC EGL glapi GLESv2 drm_nouveau nx)


        target_link_libraries(UIKit PUBLIC 
                ${EXTERN_PATH}/skia/out/horizon/libskia.a
                ${EXTERN_PATH}/skia/out/horizon/libskparagraph.a
                ${EXTERN_PATH}/skia/out/horizon/libskshaper.a
                ${EXTERN_PATH}/skia/out/horizon/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/horizon/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/horizon/libicu.a
                ${EXTERN_PATH}/skia/out/horizon/libskcms.a
                freetype
                expat
                jpeg
                png
                bz2
                z


        )
endif ()

target_include_directories(UIKit PUBLIC
        ${EXTERN_PATH}/SDL/include
        ${EXTERN_PATH}/skia
        ${EXTERN_PATH}/libromfs/lib/include
        UIKitCore/include
        UIKitCore/lib
)

if (PLATFORM_MAC)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/mac/Headers)
elseif (PLATFORM_IOS)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/ios/MetalANGLE.framework/Headers)
endif ()

if (APPLE)
    list(APPEND platform_libs "-framework Foundation" "-framework VideoToolbox" "-framework AVKit" "-framework MetalKit" "-framework CoreText")
endif ()

target_link_libraries(UIKit PUBLIC fmt ${platform_libs})