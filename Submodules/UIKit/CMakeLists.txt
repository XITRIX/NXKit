project(UIKit)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

# APPLE
if (APPLE)
    list(APPEND platform_sources
            lib/platforms/apple/SkiaInit.mm
    )

    # MAC
    if (PLATFORM_DESKTOP)
        list(APPEND google_angle_libs
                ${EXTERN_PATH}/angle/mac/libEGL.dylib
                ${EXTERN_PATH}/angle/mac/libGLESv2.dylib
        )

        list(APPEND platform_libs
                ${google_angle_libs}
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/mac-arm64-angle/libwindow.a
        )
    # IOS
    elseif (PLATFORM_IOS)
        list(APPEND FRAMEWORKS ${EXTERN_PATH}/angle/ios/MetalANGLE.framework)

        list(APPEND platform_libs
                ${google_angle_libs}
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libskia.a
                ${EXTERN_PATH}/skia/out/ios-arm64-angle/libwindow.a
        )
    endif ()

    set_property(
            SOURCE ${google_angle_libs}
            PROPERTY MACOSX_PACKAGE_LOCATION "Frameworks"
    )
endif ()

add_library(UIKit
        lib/Application.cpp
        ${platform_sources}
)

target_include_directories(UIKit PUBLIC
        ${EXTERN_PATH}/skia
        include
        lib
)

find_package(SDL2 REQUIRED)
target_link_libraries(UIKit PUBLIC
        ${platform_libs}
        SDL2::SDL2
)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_EMBED_FRAMEWORKS "${FRAMEWORKS}")
endif ()