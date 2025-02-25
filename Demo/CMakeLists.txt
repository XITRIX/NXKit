cmake_minimum_required(VERSION 3.29)

# toolchain
set(UIKIT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/UIKit)
include(${UIKIT_PATH}/Submodules/cmake/toolchain.cmake)

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

add_libromfs(${PROJECT_NAME} ${PROJECT_RESOURCES})

add_executable(${PROJECT_NAME}
        app/main.cpp
        app/AppDelegate.cpp
        app/Screens/TestViewController/TestViewController.cpp
        app/Screens/YogaTestViewController/YogaTestViewController.cpp
        app/Screens/IBTestController/IBTestController.cpp
        app/Screens/NXTextScreen/NXNavigationController.cpp
        app/Screens/NXTextScreen/NXTabBarController.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        app
)

setup_project()

target_link_libraries(${PROJECT_NAME}
        UIKit
        NXKit
)