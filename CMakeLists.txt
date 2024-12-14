cmake_minimum_required(VERSION 3.29)

# toolchain
set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Submodules)
include(${EXTERN_PATH}/cmake/toolchain.cmake)

project(ThorVGApp)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_ALTER "1")
set(VERSION_BUILD "1")
set(PACKAGE_NAME "ru.xitrix.skiatest")
set(PSN_VERSION  "01.00")
set(PROJECT_AUTHOR "XITRIX")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/img/moonlight_icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        app/main.cpp
        app/AppDelegate.cpp
)

setup_project()

target_link_libraries(${PROJECT_NAME} PRIVATE
        UIKit
)


#list(APPEND google_angle_libs
#        ${EXTERN_PATH}/angle/mac/libEGL.dylib
#        ${EXTERN_PATH}/angle/mac/libGLESv2.dylib
#)
#
#set_property(
#        SOURCE ${google_angle_libs}
#        PROPERTY MACOSX_PACKAGE_LOCATION "Frameworks"
#)
#
#target_sources(${PROJECT_NAME} PRIVATE
#        ${google_angle_libs}
#)