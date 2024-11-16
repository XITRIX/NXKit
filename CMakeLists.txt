cmake_minimum_required(VERSION 3.29)

set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Submodules)
set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")

project(ThorVGApp)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${EXTERN_PATH})

add_executable(${PROJECT_NAME} main.cpp)


find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        ThorVG
        GLAD
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/angle/mac/libEGL.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/angle/mac/libGLESv2.dylib
)