project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_subdirectory(UIKitCore)
add_subdirectory(NXKit)

# The checked-in Skia archives are built with is_debug=false. Skia's public
# inline ref-counting code changes with SK_DEBUG/SK_RELEASE, so every consumer
# of those headers must use the same mode or an app-defined SkRefCnt subclass
# can cross the static-library boundary with incompatible disposal semantics.
target_compile_definitions(XITRIXUIKit PUBLIC SK_RELEASE)

list(APPEND platform_libs yogacore tinyxml2)
target_include_directories(XITRIXUIKit PUBLIC ${EXTERN_PATH}/yoga)
target_include_directories(XITRIXUIKit PUBLIC ${EXTERN_PATH}/tinyxml2)

# APPLE
if (APPLE)
    # MAC
    if (PLATFORM_DESKTOP)
        set(SKIA_MAC_OUT_DIR ${EXTERN_PATH}/skia/out/mac-arm64)
        if (NOT EXISTS "${SKIA_MAC_OUT_DIR}/libskia.a")
            message(FATAL_ERROR "Graphite/Metal Skia archive is missing: ${SKIA_MAC_OUT_DIR}/libskia.a")
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
            message(FATAL_ERROR "Graphite/Metal Skia archive is missing: ${SKIA_IOS_OUT_DIR}/libskia.a")
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
        set(SKIA_SWITCH_OUT_DIR ${EXTERN_PATH}/skia/out/horizon)
        if (NOT EXISTS "${SKIA_SWITCH_OUT_DIR}/libskia.a")
            message(FATAL_ERROR "Graphite/Dawn Skia archive is missing: ${SKIA_SWITCH_OUT_DIR}/libskia.a")
        endif ()
        if (NOT EXISTS "${SKIA_SWITCH_OUT_DIR}/libdawn_combined.a")
            message(FATAL_ERROR "Dawn archive is missing: ${SKIA_SWITCH_OUT_DIR}/libdawn_combined.a")
        endif ()

        list(APPEND platform_libs SDL3::SDL3)

        target_include_directories(XITRIXUIKit PRIVATE
                ${EXTERN_PATH}/skia/third_party/externals/dawn
                ${EXTERN_PATH}/skia/third_party/externals/dawn/include
                ${SKIA_SWITCH_OUT_DIR}/gen/third_party/dawn/include
        )

        target_link_libraries(XITRIXUIKit PUBLIC
                ${SKIA_SWITCH_OUT_DIR}/libskia.a
                ${SKIA_SWITCH_OUT_DIR}/libskparagraph.a
                ${SKIA_SWITCH_OUT_DIR}/libskshaper.a
                ${SKIA_SWITCH_OUT_DIR}/libskunicode_core.a
                ${SKIA_SWITCH_OUT_DIR}/libskunicode_icu.a
                ${SKIA_SWITCH_OUT_DIR}/libicu.a
                ${SKIA_SWITCH_OUT_DIR}/libskcms.a
                ${SKIA_SWITCH_OUT_DIR}/libdawn_combined.a
                freetype
                harfbuzz
                expat
                jpeg
                png
                bz2
                z
                EGL
                glapi
                GLESv2
                drm_nouveau
                nx
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
