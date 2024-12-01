cmake_minimum_required(VERSION 3.29)

set(EXTERN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Submodules)
set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")

project(ThorVGApp)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${EXTERN_PATH})
add_subdirectory(${EXTERN_PATH}/UIKit)

add_executable(${PROJECT_NAME}
        app/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        UIKit
)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            # MACOSX_BUNDLE_ICON_FILE "borealis.icns"
            MACOSX_BUNDLE_COPYRIGHT "Copyright 2024 ${PROJECT_AUTHOR}"
            # RESOURCE "${CMAKE_SOURCE_DIR}/app/borealis.icns"
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
            MACOSX_BUNDLE_GUI_IDENTIFIER "${PACKAGE_NAME}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_BUILD}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}"
            XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.games" # Not work
            XCODE_ATTRIBUTE_INSTALL_PATH $(LOCAL_APPS_DIR)
            XCODE_EMBED_RESOURCES_CODE_SIGN_ON_COPY ON # Not work
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON # Not work
            XCODE_EMBED_FRAMEWORKS "${IOS_FRAMEWORKS}"
            INSTALL_RPATH "@executable_path/../Frameworks"
            XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
    )

#    list(APPEND APPLE_FRAMEWORKS ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/angle/mac/MetalANGLE.framework)
#
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            XCODE_EMBED_FRAMEWORKS "${APPLE_FRAMEWORKS}")

    list(APPEND google_angle_libs
            ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/angle/mac/libEGL.dylib
            ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/angle/mac/libGLESv2.dylib
    )

    set_property(
            SOURCE ${google_angle_libs}
            PROPERTY MACOSX_PACKAGE_LOCATION "Frameworks"
    )

    target_sources(${PROJECT_NAME} PRIVATE
            ${google_angle_libs}
    )
endif ()