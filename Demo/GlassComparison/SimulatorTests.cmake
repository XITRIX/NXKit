# Real iOS simulator executables, not macOS versions of the tests.
get_target_property(sim_abi_header UIKitCore NXKIT_METAL_SHADER_ABI_HEADER)
get_filename_component(sim_abi_directory "${sim_abi_header}" DIRECTORY)
add_executable(MetalNavigationBenchmark NavigationComparison/MetalNavigationBenchmark.mm
    app/Screens/GlassDemoViewController/GlassScreenView.cpp
    app/Screens/GlassDemoViewController/GlassScreenViewController.cpp)
target_link_libraries(MetalNavigationBenchmark PRIVATE Xitrix::UIKit NXKit)
add_executable(MetalGlassStressPerformance GlassComparison/MetalGlassStressPerformance.mm
    app/Screens/GlassDemoViewController/GlassStressView.cpp)
target_link_libraries(MetalGlassStressPerformance PRIVATE Xitrix::UIKit)
add_executable(SimulatorGlassBlurTests GlassComparison/SimulatorGlassBlurTests.mm)
target_link_libraries(SimulatorGlassBlurTests PRIVATE Xitrix::UIKit)
add_executable(LensSettingsRenderer GlassComparison/LensRefractionRenderer.cpp)
target_link_libraries(LensSettingsRenderer PRIVATE Xitrix::UIKit)
set_target_properties(LensSettingsRenderer PROPERTIES MACOSX_BUNDLE FALSE
    XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS iphonesimulator
    XCODE_ATTRIBUTE_SDKROOT iphonesimulator XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO)
foreach(target MetalNavigationBenchmark MetalGlassStressPerformance SimulatorGlassBlurTests)
    target_include_directories(${target} PRIVATE "${UIKIT_PATH}/UIKitCore/lib" "${UIKIT_PATH}/UIKit/lib" "${sim_abi_directory}")
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-arc>")
    set_target_properties(${target} PROPERTIES MACOSX_BUNDLE FALSE
        XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS iphonesimulator
        XCODE_ATTRIBUTE_SDKROOT iphonesimulator XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO)
endforeach()
