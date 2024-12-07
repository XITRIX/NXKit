project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

# APPLE
if (APPLE)
    list(APPEND platform_sources
            lib/platforms/SkiaCtx_sdlBase.cpp
    )

    # MAC
    if (PLATFORM_DESKTOP)
        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libwindow.a
        )

        list(APPEND platform_sources
                lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    # IOS
    elseif (PLATFORM_IOS)
        list(APPEND platform_sources
                lib/platforms/apple/ios/SkiaCtx_ios.mm
        )

        find_library(SDL2 NAMES SDL2-static SDL2main)

        list(APPEND platform_libs
                SDL2-static SDL2main
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libwindow.a
        )

        list(APPEND platform_sources
            lib/platforms/apple/ios/SkiaCtx_ios.mm
        )
    endif ()
endif ()

add_library(UIKit
        lib/platforms/SkiaCtx.cpp
        lib/Application.cpp
        lib/NXSize.cpp
        ${platform_sources}
)

target_include_directories(UIKit PUBLIC
        ${EXTERN_PATH}/SDL/include
        ${EXTERN_PATH}/skia
        include
        lib
)

if (PLATFORM_MAC)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/mac/Headers)
elseif (PLATFORM_IOS)
    target_include_directories(SDL2-static PUBLIC ${EXTERN_PATH}/angle/ios/MetalANGLE.framework/Headers)
endif ()

if (APPLE)
    list(APPEND platform_libs "-framework Foundation" "-framework VideoToolbox" "-framework AVKit" "-framework MetalKit")
endif ()

target_link_libraries(UIKit PRIVATE ${platform_libs})