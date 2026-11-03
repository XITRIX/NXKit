# Configure the native Vulkan dependency and an application-specific shader ABI.
# Baked native pipelines are driver/Skia/shader specific; never reuse blindly.
set(NXKIT_SWITCH_NVK_ROOT "${EXTERN_PATH}/nvk-switch" CACHE PATH "Horizon NVK SDK")
set(NXKIT_SWITCH_SKIA_OUT "${EXTERN_PATH}/skia/out/horizon-vulkan" CACHE PATH "Graphite Vulkan archives")
if(NOT EXISTS "${NXKIT_SWITCH_NVK_ROOT}/lib/libvulkan.a")
    message(FATAL_ERROR "Horizon NVK is missing. Run python3 ${UIKIT_PATH}/scripts/switch/fetch_nvk.py")
endif()
if(NOT EXISTS "${NXKIT_SWITCH_SKIA_OUT}/libskia.a")
    message(FATAL_ERROR "Graphite/Vulkan Skia is missing. Run sh ${UIKIT_PATH}/scripts/switch/build_skia.sh")
endif()

file(GLOB_RECURSE _nxkit_shader_inputs CONFIGURE_DEPENDS
        "${UIKIT_PATH}/UIKitCore/include/*.h" "${UIKIT_PATH}/UIKitCore/include/*.hpp"
        "${UIKIT_PATH}/UIKitCore/lib/*.cpp" "${UIKIT_PATH}/UIKitCore/lib/*.hpp"
        "${UIKIT_PATH}/UIKitCore/lib/*.h"
        "${UIKIT_PATH}/UIKit/include/*.h" "${UIKIT_PATH}/UIKit/include/*.hpp"
        "${UIKIT_PATH}/UIKit/lib/*.cpp" "${UIKIT_PATH}/UIKit/lib/*.hpp"
        "${UIKIT_PATH}/UIKit/lib/*.h"
        "${UIKIT_PATH}/NXKit/include/*.h" "${UIKIT_PATH}/NXKit/include/*.hpp"
        "${UIKIT_PATH}/NXKit/lib/*.cpp" "${UIKIT_PATH}/NXKit/lib/*.hpp"
        "${CMAKE_SOURCE_DIR}/app/*.cpp" "${CMAKE_SOURCE_DIR}/app/*.hpp"
        "${CMAKE_SOURCE_DIR}/app/*.h")
list(APPEND _nxkit_shader_inputs ${NXKIT_SWITCH_SHADER_SOURCES}
        "${NXKIT_SWITCH_NVK_ROOT}/lib/libvulkan.a"
        "${NXKIT_SWITCH_SKIA_OUT}/libskia.a")
list(SORT _nxkit_shader_inputs)
set(_nxkit_shader_identity "NXKit-Vulkan-cache-v1;${CMAKE_BUILD_TYPE};${CMAKE_CXX_FLAGS};${UIKIT_BUILD_UIKIT};${UIKIT_BUILD_NXKIT}")
foreach(_input IN LISTS _nxkit_shader_inputs)
    file(SHA256 "${_input}" _digest)
    # Content hashes, not absolute checkout paths, allow reproducible packaging.
    string(APPEND _nxkit_shader_identity ";${_digest}")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_nxkit_shader_inputs})
string(SHA256 NXKIT_SWITCH_SHADER_ABI "${_nxkit_shader_identity}")
set(NXKIT_SWITCH_SHADER_ABI "${NXKIT_SWITCH_SHADER_ABI}" CACHE INTERNAL "Current Switch shader ABI" FORCE)
set(NXKIT_SWITCH_SHADER_CACHE_DIR "${CMAKE_SOURCE_DIR}/build/switch-shaders/${NXKIT_SWITCH_SHADER_ABI}")
file(WRITE "${CMAKE_BINARY_DIR}/switch-shader-abi.txt" "${NXKIT_SWITCH_SHADER_ABI}\n")
file(WRITE "${CMAKE_BINARY_DIR}/switch-shader-cache-dir.txt" "${NXKIT_SWITCH_SHADER_CACHE_DIR}\n")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/SwitchShaderABI.hpp"
        CONTENT "#pragma once\n#define NXKIT_SWITCH_SHADER_ABI \"${NXKIT_SWITCH_SHADER_ABI}\"\n")
target_include_directories(UIKitCore PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
find_program(NXKIT_HOST_PYTHON NAMES python3 NO_CMAKE_FIND_ROOT_PATH REQUIRED)
set(_nxkit_seed_cpp "${CMAKE_CURRENT_BINARY_DIR}/generated/SwitchPipelineSeed.cpp")
file(MAKE_DIRECTORY "${NXKIT_SWITCH_SHADER_CACHE_DIR}")
# Optional seed files are tracked with CONFIGURE_DEPENDS so a completed bake is
# picked up by the next ordinary NRO build, without editing source resources.
file(GLOB _nxkit_seed_files CONFIGURE_DEPENDS
        "${NXKIT_SWITCH_SHADER_CACHE_DIR}/pipeline.cache"
        "${NXKIT_SWITCH_SHADER_CACHE_DIR}/pipeline.keys")
add_custom_command(OUTPUT "${_nxkit_seed_cpp}"
        COMMAND "${NXKIT_HOST_PYTHON}" "${UIKIT_PATH}/scripts/switch/embed_pipeline_cache.py"
                --directory "${NXKIT_SWITCH_SHADER_CACHE_DIR}" --output "${_nxkit_seed_cpp}"
        DEPENDS "${UIKIT_PATH}/scripts/switch/embed_pipeline_cache.py" ${_nxkit_seed_files}
        VERBATIM)
add_custom_target(NXKitSwitchPipelineSeed DEPENDS "${_nxkit_seed_cpp}")
add_dependencies(UIKitCore NXKitSwitchPipelineSeed)
target_sources(UIKitCore PRIVATE "${_nxkit_seed_cpp}")
