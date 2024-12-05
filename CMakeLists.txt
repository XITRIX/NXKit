cmake_minimum_required(VERSION 3.29)

# toolchain
set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Submodules)
include(${EXTERN_PATH}/cmake/toolchain.cmake)

project(ThorVGApp)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "2")
set(VERSION_ALTER "2")
set(VERSION_BUILD "1")
set(PACKAGE_NAME "ru.xitrix.Moonlight")
set(PSN_VERSION  "01.00")
set(PROJECT_AUTHOR "XITRIX")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/img/moonlight_icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        app/main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        UIKit
)

setup_project()