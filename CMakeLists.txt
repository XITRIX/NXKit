project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_subdirectory(UIKitCore)
add_subdirectory(NXKit)

list(APPEND platform_libs yogacore tinyxml2)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/yoga)
target_include_directories(UIKit PUBLIC ${EXTERN_PATH}/tinyxml2)

# APPLE
if (APPLE)
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
    endif ()
elseif (PLATFORM_SWITCH)
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