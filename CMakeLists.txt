cmake_minimum_required(VERSION 3.29)

set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/extern)
set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")

project(ThorVG)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
)