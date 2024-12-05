project(UIKit)

find_package(SDL2 REQUIRED)

add_definitions(
        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_library(${PROJECT_NAME}
        lib/Application.cpp
        lib/platforms/mac/SkiaInit.mm
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${EXTERN_PATH}/skia
        include
        lib
)


list(APPEND google_angle_libs
        ${EXTERN_PATH}/angle/mac/libEGL.dylib
        ${EXTERN_PATH}/angle/mac/libGLESv2.dylib
)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${google_angle_libs}
        ${EXTERN_PATH}/skia/out/mac-arm64-angle/libskia.a
        ${EXTERN_PATH}/skia/out/mac-arm64-angle/libwindow.a
        SDL2::SDL2
        GLAD
        ThorVG
)

set_property(
        SOURCE ${google_angle_libs}
        PROPERTY MACOSX_PACKAGE_LOCATION "Frameworks"
)