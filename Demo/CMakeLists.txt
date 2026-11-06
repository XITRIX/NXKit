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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_libromfs(${PROJECT_NAME} ${PROJECT_RESOURCES})

add_executable(${PROJECT_NAME}
        app/main.cpp
        app/SwitchShaderBake.cpp
        app/AppDelegate.cpp
        app/Screens/TestViewController/TestViewController.cpp
        app/Screens/YogaTestViewController/YogaTestViewController.cpp
        app/Screens/IBTestController/IBTestController.cpp
        app/Screens/TestScrollController/TestScrollController.cpp
        app/Screens/NavigationTestViewController/NavigationTestViewController.cpp
        app/Screens/GlassDemoViewController/GlassDemoViewController.cpp
        app/Screens/GlassDemoViewController/GlassDemoScene.cpp
        app/Screens/GlassDemoViewController/LensTabBarView.cpp
        app/Screens/GlassDemoViewController/GlassStressView.cpp
        app/Screens/GlassDemoViewController/GlassScreenView.cpp
        app/Screens/GlassDemoViewController/GlassScreenViewController.cpp
        app/Screens/GlassDemoViewController/LensDemoStage.cpp
        app/Screens/ControllerTestViewController/ControllerTestViewController.cpp
        app/Screens/TabBarDemoController/TabBarDemoController.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        app
)

setup_project()

target_link_libraries(${PROJECT_NAME}
        Xitrix::UIKit
        NXKit
)

if(PLATFORM_SWITCH)
    set(NXKIT_SWITCH_ADDRESS "" CACHE STRING "Switch IP for shader compilation and deployment")
    add_custom_target(${PROJECT_NAME}.switch-shaders
        COMMAND "${NXKIT_HOST_PYTHON}" "${UIKIT_PATH}/scripts/switch/bake_shaders.py"
                --build-dir "${CMAKE_BINARY_DIR}" --address "${NXKIT_SWITCH_ADDRESS}"
                --nxlink "$ENV{DEVKITPRO}/tools/bin/nxlink" --target "${PROJECT_NAME}"
        DEPENDS ${PROJECT_NAME}.nro
        USES_TERMINAL VERBATIM
        COMMENT "Compiling Switch Vulkan pipelines on the target and bundling the cache")
endif()

if(APPLE)
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_GENERATE_SCHEME TRUE)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_custom_target(${PROJECT_NAME}.performance-scheme
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/performance_scheme.py"
                "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj" "${PROJECT_NAME}"
            VERBATIM COMMENT "Generating optimized Xcode performance scheme")
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.performance-scheme)
    endif()
    option(NXKIT_BUILD_METAL_SHADERS "Build and embed standard UI Metal pipelines offline" ON)
    set(NXKIT_METAL_SHADER_HARVEST "${UIKIT_PATH}/shaders/apple/catalog" CACHE PATH
        "Source catalog exported by the Metal shader harness")
    if(NXKIT_BUILD_METAL_SHADERS)
        include(${UIKIT_PATH}/cmake/NXKitMetalCache.cmake)
        nxkit_target_metal_shaders(${PROJECT_NAME} "${NXKIT_METAL_SHADER_HARVEST}")
    endif()
endif()
