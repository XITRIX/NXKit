message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(EXTERN_PATH ${UIKIT_PATH}/Submodules)

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

function(ios_bundle iosStoryBoard assets plist)
    set(IOS_CODE_SIGN_IDENTITY "" CACHE STRING "The code sign identity to use when building the IPA.")
    set(IOS_GUI_IDENTIFIER "" CACHE STRING "The package name")
    if(IOS_CODE_SIGN_IDENTITY STREQUAL "")
        set(IOS_CODE_SIGNING_ENABLED NO)
    else()
        set(IOS_CODE_SIGNING_ENABLED YES)
    endif()
    if (IOS_GUI_IDENTIFIER STREQUAL "")
        set(IOS_GUI_IDENTIFIER "${PACKAGE_NAME}")
        message(WARNING "Using default package name: ${PACKAGE_NAME}")
    endif ()

    set(IOS_SPLASH_STORYBOARD "${iosStoryBoard}")

    set(IOS_ASSETS ${assets})
    set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${IOS_CODE_SIGN_IDENTITY}"
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${IOS_CODE_SIGNING_ENABLED}"
            BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${plist}
            XCODE_ATTRIBUTE_ENABLE_BITCODE NO
            XCODE_ATTRIBUTE_SKIP_INSTALL NO
    )
    set_property(
            SOURCE ${IOS_ASSETS}
            PROPERTY MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_property(
            SOURCE ${IOS_SPLASH_STORYBOARD}
            PROPERTY MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${PROJECT_NAME} PRIVATE
            ${IOS_ASSETS}
            ${IOS_SPLASH_STORYBOARD}
    )
endfunction()

# Platform specific configs
if (PLATFORM_DESKTOP)
    message("Building for Desktop")
    add_definitions( -DPLATFORM_DESKTOP )
    if (APPLE)
        set(PLATFORM_MAC ON)
        add_definitions( -DPLATFORM_MAC )
    endif ()

    set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")
elseif (PLATFORM_IOS)
    message(STATUS "building for iOS")
    add_definitions( -DPLATFORM_IOS )

    set(DEPLOYMENT_TARGET "15.0")

    set(USE_GLES ON)

    set(BUILD_SHARED_LIBS OFF)
    set(PLATFORM OS64)
    set(CMAKE_TOOLCHAIN_FILE ${EXTERN_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE PATH "vcpkg toolchain file")
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${EXTERN_PATH}/cmake/ios.toolchain.cmake CACHE PATH "ios toolchain file")
    set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2") # iphone, ipad
elseif (PLATFORM_SWITCH)
    message(STATUS "building for SWITCH")
    set(USE_GLES ON)
    if (NOT DEFINED ENV{DEVKITPRO})
        message(FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
    endif ()
    add_definitions( -DPLATFORM_SWITCH )
    set(DEVKITPRO $ENV{DEVKITPRO} CACHE BOOL "DEVKITPRO")
    set(__SWITCH__ ON)
    set(USE_SYSTEM_SDL ON)
    set(CMAKE_C_FLAGS "-I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include -D__SWITCH__")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
    include(${DEVKITPRO}/cmake/Switch.cmake REQUIRED)
    # message("${DEVKITPRO}/portlibs/switch/include")
endif ()

if (USE_GLES)
    add_definitions( -DUSE_GLES )
endif ()

# Setup LibRomFS
check_libromfs_generator()

function(setup_project)
    if (APPLE)
        add_subdirectory(${EXTERN_PATH}/SDL)

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
                XCODE_EMBED_FRAMEWORKS "${FRAMEWORKS}"
                INSTALL_RPATH "@executable_path/../Frameworks"
                XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
        )
    endif ()

    if (PLATFORM_MAC)
        list(APPEND google_angle_libs
                ${EXTERN_PATH}/angle/mac/libEGL.dylib
                ${EXTERN_PATH}/angle/mac/libGLESv2.dylib
        )

        list(APPEND platform_libs
                ${google_angle_libs}
        )

        set_property(
                SOURCE ${google_angle_libs}
                PROPERTY MACOSX_PACKAGE_LOCATION "Frameworks"
        )

        target_sources(${PROJECT_NAME} PRIVATE
                ${google_angle_libs}
        )
    elseif (PLATFORM_IOS)
        ios_bundle("${CMAKE_CURRENT_SOURCE_DIR}/app/platforms/ios/iphoneos/Splash.storyboard"
                "${CMAKE_CURRENT_SOURCE_DIR}/app/platforms/ios/Images.xcassets"
                "${CMAKE_CURRENT_SOURCE_DIR}/app/platforms/ios/iOSBundleInfo.plist.in")

        set_target_properties(${PROJECT_NAME} PROPERTIES
                INSTALL_RPATH @executable_path/Frameworks
                XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
                MACOSX_BUNDLE_COPYRIGHT "Copyright 2024 ${PROJECT_AUTHOR}"
                XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${VERSION_BUILD}"
                XCODE_ATTRIBUTE_INFOPLIST_KEY_GCSupportsGameMode YES
                XCODE_ATTRIBUTE_INFOPLIST_KEY_GCSupportsControllerUserInteraction YES
                XCODE_ATTRIBUTE_INFOPLIST_KEY_GCRequiresControllerUserInteraction YES
                XCODE_ATTRIBUTE_INFOPLIST_KEY_LSApplicationCategoryType "public.app-category.games"
                XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
                XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${EXTERN_PATH}/angle/ios/"
                XCODE_ATTRIBUTE_INSTALL_PATH $(LOCAL_APPS_DIR))

        set_target_properties(${PROJECT_NAME} PROPERTIES
                XCODE_EMBED_FRAMEWORKS ${EXTERN_PATH}/angle/ios/MetalANGLE.framework
        )
    elseif (PLATFORM_SWITCH)
        add_custom_target(${PROJECT_NAME}.nro DEPENDS ${PROJECT_NAME}
            COMMAND ${NX_NACPTOOL_EXE} --create "${PROJECT_NAME}" "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}" ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
#            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/font
            COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp
    )
    endif ()

    # Include Submodules
    add_subdirectory(${UIKIT_PATH})
    add_subdirectory(${EXTERN_PATH})
    add_subdirectory(${EXTERN_PATH}/yoga)
    add_subdirectory(${EXTERN_PATH}/tinyxml2)
    add_subdirectory(${EXTERN_PATH}/fmt)

    add_subdirectory(${EXTERN_PATH}/libromfs EXCLUDE_FROM_ALL)
    target_link_libraries(${PROJECT_NAME} ${LIBROMFS_LIBRARY})

#    target_link_libraries(${PROJECT_NAME} fmt tweeny yogacore ${PLATFORM_LIBS})
endfunction()