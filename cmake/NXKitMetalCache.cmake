# Shared by normal Apple builds and the headless Metal regression/bake tool.
function(nxkit_configure_metal_cache target root)
    file(GLOB_RECURSE inputs CONFIGURE_DEPENDS
        "${root}/UIKit/lib/*.cpp" "${root}/UIKit/lib/*.hpp"
        "${root}/UIKitCore/lib/*Shader*" "${root}/UIKitCore/lib/*Corner*")
    list(APPEND inputs
        "${root}/UIKitCore/lib/BackdropEffect.cpp"
        "${root}/UIKitCore/include/BackdropEffect.h"
        "${root}/UIKitCore/lib/CALayer.cpp"
        "${root}/UIKitCore/include/CALayer.h"
        "${root}/UIKitCore/lib/LayerContentMask.hpp"
        "${root}/UIKitCore/lib/LayerUniformPrefix.hpp"
        "${root}/Submodules/skia/src/gpu/graphite/Device.h"
        "${root}/Submodules/skia/src/gpu/graphite/draw/DrawContext.h"
        "${root}/Submodules/skia/src/gpu/graphite/draw/DrawListBase.h"
        "${root}/Submodules/skia/src/gpu/graphite/mtl/MtlGraphiteUtils.mm"
        "${root}/Submodules/skia/src/gpu/graphite/mtl/MtlGraphicsPipeline.mm"
        "${root}/Submodules/skia/src/gpu/graphite/mtl/MtlCaps.mm"
        "${root}/Submodules/skia/src/gpu/graphite/mtl/MtlCommandBuffer.mm"
        "${root}/Submodules/skia/src/gpu/graphite/mtl/MtlCommandBuffer.h"
        "${root}/Submodules/skia/src/core/SkImageFilterTypes.cpp"
        "${root}/Submodules/skia/src/core/SkImageFilterTypes.h"
        "${root}/Submodules/skia/src/effects/imagefilters/SkRuntimeImageFilter.cpp"
        "${root}/Submodules/skia/include/effects/SkImageFilters.h"
        "${root}/Submodules/skia/include/gpu/graphite/mtl/MtlBackendContext.h")
    # CONFIGURE_DEPENDS on a glob detects additions/removals, not content edits.
    # Recompute the catalog ABI when an existing shader-generating input changes.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${inputs})
    set(signature "NXKit-Metal-v1-language2.3")
    foreach(input IN LISTS inputs)
        file(SHA256 "${input}" digest)
        file(RELATIVE_PATH name "${root}" "${input}")
        string(APPEND signature "${name}:${digest}\n")
    endforeach()
    execute_process(COMMAND git -C "${root}/Submodules/skia" rev-parse HEAD
        OUTPUT_VARIABLE revision OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    string(SHA256 abi "${revision}\n${signature}")
    set(directory "${CMAKE_CURRENT_BINARY_DIR}/${target}-metal-generated")
    file(MAKE_DIRECTORY "${directory}")
    file(CONFIGURE OUTPUT "${directory}/NXMetalShaderABI.hpp"
         CONTENT "#pragma once\n#define NXKIT_METAL_SHADER_ABI \"${abi}\"\n" @ONLY)
    target_include_directories(${target} PRIVATE "${directory}")
    target_sources(${target} PRIVATE "${root}/UIKitCore/lib/platforms/apple/MetalPipelineCache.mm")
    set_source_files_properties("${root}/UIKitCore/lib/platforms/apple/MetalPipelineCache.mm"
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc;-fno-rtti")
    set(NXKIT_METAL_SHADER_ABI "${abi}" PARENT_SCOPE)
    set_property(TARGET ${target} PROPERTY NXKIT_METAL_SHADER_ABI "${abi}")
    set_property(TARGET ${target} PROPERTY NXKIT_METAL_SHADER_ABI_HEADER "${directory}/NXMetalShaderABI.hpp")
endfunction()

# App opt-in: source catalog is portable, binary output is SDK/GPU-specific.
# The standard Demo enables this with the checked-in framework catalog.
function(nxkit_target_metal_shaders target catalog)
    if(NOT APPLE)
        return()
    endif()
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(NXKIT_METAL_GPU_FAMILY "apple7" CACHE STRING "Metal GPU family for offline pipeline compilation (Apple7 and later)")
    if(PLATFORM_IOS)
        if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
            set(sdk iphonesimulator)
            set(platform ios)
        else()
            set(sdk iphoneos)
            set(platform ios)
        endif()
        set(resources "$<TARGET_BUNDLE_DIR:${target}>/NXKitMetalShaders")
    else()
        set(sdk macosx)
        set(platform macos)
        set(resources "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/NXKitMetalShaders")
    endif()
    execute_process(COMMAND xcrun -sdk ${sdk} --show-sdk-version
        OUTPUT_VARIABLE sdkVersion OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    string(REGEX MATCH "^[0-9]+" sdkMajor "${sdkVersion}")
    set(NXKIT_METAL_ARCHIVE_OS_VERSION "${sdkMajor}.0" CACHE STRING
        "OS compiler generation for native Metal binaries; other generations use launch-time preparation")
    set(air "air64-apple-${platform}${NXKIT_METAL_ARCHIVE_OS_VERSION}")
    if(PLATFORM_IOS)
        set(libraryAir "air64-apple-ios15.0")
    else()
        set(libraryAir "air64-apple-macos12.0")
    endif()
    if(sdk STREQUAL "iphonesimulator")
        string(APPEND air "-simulator")
        string(APPEND libraryAir "-simulator")
    endif()
    file(GLOB sources CONFIGURE_DEPENDS "${catalog}/sources/*.metal")
    if(NOT sources OR NOT EXISTS "${catalog}/pipelines.mtlp-json")
        message(FATAL_ERROR "Metal shader catalog is missing: ${catalog}. Run MetalNavigationBenchmark with NXKIT_METAL_HARVEST_DIR, or explicitly disable NXKIT_BUILD_METAL_SHADERS for a runtime fallback build.")
    endif()
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${target}-metal-${sdk}")
    get_target_property(expectedABI UIKitCore NXKIT_METAL_SHADER_ABI)
    get_target_property(abiHeader UIKitCore NXKIT_METAL_SHADER_ABI_HEADER)
    get_filename_component(root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    execute_process(COMMAND xcrun -sdk ${sdk} --find metal
        OUTPUT_VARIABLE metalCompiler OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    file(CONFIGURE OUTPUT "${output}-configuration.txt"
        CONTENT "${sdk}\n${air}\n${libraryAir}\n${NXKIT_METAL_GPU_FAMILY}\n${expectedABI}\n${metalCompiler}\n" @ONLY)
    add_custom_command(OUTPUT "${output}/manifest.json"
        COMMAND "${Python3_EXECUTABLE}" "${root}/scripts/apple/bake_metal.py"
            --harvest "${catalog}" --output "${output}" --sdk "${sdk}"
            --target "${air}" --gpu-family "${NXKIT_METAL_GPU_FAMILY}"
            --library-target "${libraryAir}"
            --abi "${expectedABI}"
        DEPENDS ${sources} "${catalog}/manifest.json" "${catalog}/pipelines.mtlp-json"
            "${abiHeader}"
            "${output}-configuration.txt"
            "${root}/scripts/apple/bake_metal.py"
        COMMENT "Compiling ${target} Metal libraries and GPU pipelines offline"
        VERBATIM)
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        # Keep Xcode placeholders out of CMake's shell quoting. The helper
        # reads the generated destination verbatim and expands the SDK suffix.
        set(bundleDestination "${CMAKE_CURRENT_BINARY_DIR}/${target}-metal-bundle-$<CONFIG>.txt")
        file(GENERATE OUTPUT "${bundleDestination}" CONTENT "${resources}\n")
        add_custom_target(${target}.metal-shaders DEPENDS "${output}/manifest.json"
            COMMAND /bin/sh "${root}/scripts/apple/embed_metal.sh" "${output}" "${bundleDestination}"
            COMMENT "Embedding precompiled Metal shaders" VERBATIM)
    else()
        add_custom_target(${target}.metal-shaders DEPENDS "${output}/manifest.json"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory "${output}" "${resources}"
            COMMENT "Embedding precompiled Metal shaders" VERBATIM)
    endif()
    add_dependencies(${target} ${target}.metal-shaders)
endfunction()
