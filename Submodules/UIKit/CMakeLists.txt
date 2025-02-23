project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_library(UIKit
        lib/tools/Logger.cpp
        lib/tools/IBTools.cpp
        lib/tools/Fatal.cpp
        lib/tools/Tools.cpp
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
        lib/IndexPath.cpp
        lib/CGImage.cpp
        lib/CADisplayLink.cpp
        lib/CALayer.cpp
        lib/YGLayout.cpp
        lib/UIAction.cpp
        lib/UIApplication.cpp
        lib/UIApplicationDelegateImpl.cpp
        lib/UIApplicationMain.cpp
        lib/UIBlurView.cpp
        lib/UIButton.cpp
        lib/UIColor.cpp
        lib/UIControl.cpp
        lib/UIControlGestureRecognizer.cpp
        lib/UIEdgeInsets.cpp
        lib/UIEvent.cpp
        lib/UIFocus.cpp
        lib/UIFocusAnimationCoordinator.cpp
        lib/UIFocusSystem.cpp
        lib/UIGestureRecognizer.cpp
        lib/UIImage.cpp
        lib/UIImageView.cpp
        lib/UIKey.cpp
        lib/UILabel.cpp
        lib/UINib.cpp
        lib/UIPanGestureRecognizer.cpp
        lib/UIPress.cpp
        lib/UIPressesEvent.cpp
        lib/UIResponder.cpp
        lib/UIScrollView.cpp
        lib/UITapGestureRecognizer.cpp
        lib/UITouch.cpp
        lib/UITraitCollection.cpp
        lib/UITraitEnvironment.cpp
        lib/UIView.cpp
        lib/UIViewAnimationGroup.cpp
        lib/UIViewAnimationOptions.cpp
        lib/UIViewController.cpp
        lib/UIWindow.cpp
        lib/NXAffineTransform.cpp
        lib/NXTransform3D.cpp
        lib/NXData.cpp
        lib/UIScrollViewExtensions/DecelerationTimingParameters.cpp
        lib/UIScrollViewExtensions/RubberBand.cpp
        lib/UIScrollViewExtensions/SpringTimingParameters.cpp
        lib/UIScrollViewExtensions/TimerAnimation.cpp
        lib/SkTools/EventTracingPriv.cpp
        lib/SkTools/SkDebugfTracer.cpp
)

list(APPEND platform_libs yogacore tinyxml2)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/yoga)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/tinyxml2)

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
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskparagraph.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libicu.a
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
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskparagraph.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskshaper.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_core.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskunicode_icu.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libicu.a
        )

        target_sources(UIKit PRIVATE
                lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
elseif (PLATFORM_SWITCH)
        target_sources(UIKit PUBLIC
                lib/platforms/SkiaCtx_sdlBase.cpp
                lib/platforms/switch/SkiaCtx_switch.cpp
                lib/platforms/switch/switch_wrapper.c
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

target_link_libraries(UIKit PUBLIC fmt ${platform_libs})