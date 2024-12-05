message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Setup Sunmodules directory
set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")

# Include Submodules
function(check_libromfs_generator)
    if (NOT DEFINED LIBROMFS_PREBUILT_GENERATOR OR NOT EXISTS "${LIBROMFS_PREBUILT_GENERATOR}")
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libromfs-generator")
            set(LIBROMFS_PREBUILT_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/libromfs-generator" PARENT_SCOPE)
        else ()
            message(FATAL_ERROR "libromfs-generator has not been built, please refer to build_libromfs_generator.sh for more information")
        endif ()
    endif()
endfunction()

# LibRomFS Init functions
function(add_libromfs app res)
    message(STATUS "USE LIBROMFS")
    add_definitions(-DUSE_LIBROMFS)
    set(LIBROMFS_PROJECT_NAME ${app} PARENT_SCOPE)
    set(LIBROMFS_RESOURCE_LOCATION "${res}" PARENT_SCOPE)
endfunction()

# Platform specific configs
if (PLATFORM_DESKTOP)
    message("Building for Desktop")
    add_definitions( -DPLATFORM_DESKTOP )
    if (APPLE)
        add_definitions( -DPLATFORM_MAC )
    endif ()
elseif (PLATFORM_IOS)
    message("Building for iOS")
    add_definitions( -DPLATFORM_IOS )

    set(PLATFORM OS64)
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${EXTERN_PATH}/cmake/ios.toolchain.cmake CACHE PATH "ios toolchain file")
    set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2") # iphone, ipad
endif ()

# Setup LibRomFS
check_libromfs_generator()

function(setup_project)
    # Include Submodules
    add_subdirectory(${EXTERN_PATH})
    add_subdirectory(${EXTERN_PATH}/UIKit)

    add_libromfs(${PROJECT_NAME} ${PROJECT_RESOURCES})
    add_subdirectory(${EXTERN_PATH}/libromfs EXCLUDE_FROM_ALL)

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

        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Foundation" "-framework VideoToolbox" "-framework AVKit" "-framework MetalKit")
    endif ()
endfunction()