project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_subdirectory(UIKitCore)
add_subdirectory(NXKit)

list(APPEND platform_libs yogacore tinyxml2)
target_include_directories(XITRIXUIKit PUBLIC ${EXTERN_PATH}/yoga)
target_include_directories(XITRIXUIKit PUBLIC ${EXTERN_PATH}/tinyxml2)

# APPLE
if (APPLE)
    # MAC
    if (PLATFORM_DESKTOP)
        set(SKIA_MAC_OUT_DIR ${EXTERN_PATH}/skia/out/mac-arm64)
        if (NOT EXISTS "${SKIA_MAC_OUT_DIR}/libskia.a")
            message(FATAL_ERROR "Native Metal Skia archive is missing: ${SKIA_MAC_OUT_DIR}/libskia.a")
        endif ()

        list(APPEND platform_libs
                SDL3::SDL3
                ${SKIA_MAC_OUT_DIR}/libskia.a
                ${SKIA_MAC_OUT_DIR}/libskparagraph.a
                ${SKIA_MAC_OUT_DIR}/libskshaper.a
                ${SKIA_MAC_OUT_DIR}/libskunicode_core.a
                ${SKIA_MAC_OUT_DIR}/libskunicode_icu.a
                ${SKIA_MAC_OUT_DIR}/libicu.a
        )
    # IOS
    elseif (PLATFORM_IOS)
        set(SKIA_IOS_OUT_DIR ${EXTERN_PATH}/skia/out/ios-arm64)
        if (NOT EXISTS "${SKIA_IOS_OUT_DIR}/libskia.a")
            message(FATAL_ERROR "Native Metal Skia archive is missing: ${SKIA_IOS_OUT_DIR}/libskia.a")
        endif ()

        list(APPEND platform_libs
                SDL3::SDL3
                ${SKIA_IOS_OUT_DIR}/libskia.a
                ${SKIA_IOS_OUT_DIR}/libskparagraph.a
                ${SKIA_IOS_OUT_DIR}/libskshaper.a
                ${SKIA_IOS_OUT_DIR}/libskunicode_core.a
                ${SKIA_IOS_OUT_DIR}/libskunicode_icu.a
                ${SKIA_IOS_OUT_DIR}/libicu.a
        )
    endif ()
elseif (PLATFORM_SWITCH)
        list(APPEND platform_libs SDL3::SDL3)

        # GPU
        target_link_libraries(XITRIXUIKit PUBLIC EGL glapi GLESv2 drm_nouveau nx)

        target_link_libraries(XITRIXUIKit PUBLIC
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

target_include_directories(XITRIXUIKit PUBLIC
        ${EXTERN_PATH}/skia
        ${EXTERN_PATH}/libromfs/lib/include
)

if (APPLE)
    list(APPEND platform_libs
            "-framework Foundation"
            "-framework VideoToolbox"
            "-framework AVKit"
            "-framework Metal"
            "-framework MetalKit"
            "-framework QuartzCore"
            "-framework CoreText")
endif ()

target_link_libraries(XITRIXUIKit PUBLIC fmt ${platform_libs})
