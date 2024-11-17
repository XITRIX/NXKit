project(UIKit)

find_package(SDL2 REQUIRED)

add_library(${PROJECT_NAME}
    lib/Application.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        lib
)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC
        SDL2::SDL2
        GLAD
        ThorVG
)