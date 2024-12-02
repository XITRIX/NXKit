project(UIKit)

find_package(SDL2 REQUIRED)

add_definitions(
        -DSK_METAL
        -DSK_GANESH
#        -DSK_GRAPHITE
#        -DSK_TRIVIAL_ABI=[[clang::trivial_abi]]
)

add_library(${PROJECT_NAME}
        lib/Application.cpp
#        ${EXTERN_PATH}/skia/tools/sk_app/mac/main_mac.mm
#        ${EXTERN_PATH}/skia/tools/sk_app/mac/Window_mac.mm
#        ${EXTERN_PATH}/skia/tools/sk_app/CommandSet.cpp
#        ${EXTERN_PATH}/skia/tools/sk_app/Window.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${EXTERN_PATH}/skia
        include
        lib
)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${EXTERN_PATH}/skia/out/mac-arm64/libskia.a
        ${EXTERN_PATH}/skia/out/mac-arm64/libwindow.a
        SDL2::SDL2
        GLAD
        ThorVG
)