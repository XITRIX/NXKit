cmake_minimum_required(VERSION 3.20)
project(NXKitHeadlessRegression LANGUAGES C CXX)
if (APPLE)
    enable_language(OBJCXX)
endif ()
set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type matching the raster Skia archives" FORCE)
endif ()
get_filename_component(NXKIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(NXKIT_SKIA_RASTER_HEADERS "${NXKIT_ROOT}/Submodules/skia" CACHE PATH "Headers matching the raster archives")
set(NXKIT_SKIA_RASTER_ARCHIVE "${NXKIT_ROOT}/Submodules/skia/out/mac-arm64-angle" CACHE PATH "Raster Skia archive directory")
if (NOT EXISTS "${NXKIT_SKIA_RASTER_ARCHIVE}/libskia.a")
    message(FATAL_ERROR "Build raster Skia or set NXKIT_SKIA_RASTER_ARCHIVE to matching archives")
endif ()
enable_testing()
find_package(SDL3 CONFIG REQUIRED)
add_subdirectory(${NXKIT_ROOT}/Submodules/yoga/yoga yoga)
add_subdirectory(${NXKIT_ROOT}/Submodules/tinyxml2 tinyxml2)
add_subdirectory(${NXKIT_ROOT}/Submodules/fmt fmt)
add_subdirectory(${NXKIT_ROOT}/UIKitCore UIKitCore)
get_target_property(core_sources UIKitCore SOURCES)
list(FILTER core_sources EXCLUDE REGEX "(lib/platforms/.*|lib/Application.cpp|lib/UIApplicationMain.cpp)$")
set_property(TARGET UIKitCore PROPERTY SOURCES ${core_sources})
target_sources(UIKitCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/RasterPlatform.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/platforms/FramePacing.cpp)
option(NXKIT_SKIA_DEBUG "Match a raster Skia archive built with is_debug=true" OFF)
if (NXKIT_SKIA_DEBUG)
    target_compile_definitions(UIKitCore PUBLIC SK_DEBUG)
else ()
    target_compile_definitions(UIKitCore PUBLIC SK_RELEASE)
endif ()
target_compile_definitions(UIKitCore PUBLIC "SK_TRIVIAL_ABI=[[clang::trivial_abi]]")
target_include_directories(UIKitCore BEFORE PUBLIC ${NXKIT_SKIA_RASTER_HEADERS})
target_include_directories(UIKitCore PUBLIC ${NXKIT_ROOT}/Submodules ${NXKIT_ROOT}/Submodules/skia ${NXKIT_ROOT}/Submodules/yoga ${NXKIT_ROOT}/Submodules/tinyxml2 ${NXKIT_ROOT}/Submodules/libromfs/lib/include)
set(skia ${NXKIT_SKIA_RASTER_ARCHIVE})
target_link_libraries(UIKitCore PUBLIC yogacore tinyxml2 fmt SDL3::SDL3 ${skia}/libskia.a ${skia}/libskparagraph.a ${skia}/libskshaper.a ${skia}/libskunicode_core.a ${skia}/libskunicode_icu.a ${skia}/libicu.a "-framework Cocoa" "-framework Metal" "-framework CoreVideo" "-framework OpenGL" "-framework IOKit" "-framework QuartzCore" "-framework CoreText" z iconv expat)
if (APPLE)
    target_sources(UIKitCore PRIVATE ${NXKIT_ROOT}/UIKitCore/lib/platforms/apple/AppleSystemFont.mm)
    target_sources(UIKitCore PRIVATE ${NXKIT_ROOT}/UIKitCore/lib/platforms/apple/AppleSystemImage.mm)
    add_executable(AppleGlassPreferenceTests ${NXKIT_ROOT}/UIKitCore/tests/AppleGlassPreferenceTests.mm
        ${NXKIT_ROOT}/UIKitCore/lib/platforms/apple/AppleGlassPreference.mm)
    target_include_directories(AppleGlassPreferenceTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib/platforms/apple)
    target_link_libraries(AppleGlassPreferenceTests PRIVATE UIKitCore)
    add_test(NAME AppleGlassPreferenceTests COMMAND AppleGlassPreferenceTests)
    include(${NXKIT_ROOT}/cmake/NXKitMetalCache.cmake)
    nxkit_configure_metal_cache(UIKitCore "${NXKIT_ROOT}")
endif ()
add_executable(CoreModuleTests ${NXKIT_ROOT}/UIKitCore/tests/ModuleBoundaryTests.cpp)
target_link_libraries(CoreModuleTests PRIVATE UIKitCore)
add_test(NAME CoreModuleTests COMMAND CoreModuleTests)
add_executable(FramePacingTests ${NXKIT_ROOT}/UIKitCore/tests/FramePacingTests.cpp)
target_link_libraries(FramePacingTests PRIVATE UIKitCore)
add_test(NAME FramePacingTests COMMAND FramePacingTests)
set_tests_properties(FramePacingTests PROPERTIES ENVIRONMENT "NXKIT_UNCAPPED=0")
add_test(NAME UncappedFramePacingTests COMMAND FramePacingTests --uncapped)
set_tests_properties(UncappedFramePacingTests PROPERTIES ENVIRONMENT "NXKIT_UNCAPPED=1")
add_executable(ViewInteractionTests ${NXKIT_ROOT}/UIKitCore/tests/UIViewInteractionTests.cpp)
target_link_libraries(ViewInteractionTests PRIVATE UIKitCore)
add_test(NAME ViewInteractionTests COMMAND ViewInteractionTests)
add_executable(PressLifecycleTests ${NXKIT_ROOT}/UIKitCore/tests/UIApplicationPressLifecycleTests.cpp)
target_link_libraries(PressLifecycleTests PRIVATE UIKitCore)
add_test(NAME PressLifecycleTests COMMAND PressLifecycleTests)
add_executable(CornerConfigurationTests ${NXKIT_ROOT}/UIKitCore/tests/CornerConfigurationTests.cpp)
target_link_libraries(CornerConfigurationTests PRIVATE UIKitCore)
add_test(NAME CornerConfigurationTests COMMAND CornerConfigurationTests)
option(NXKIT_RASTER_CORE_ONLY "Build Core without either component kit" OFF)
if (NXKIT_RASTER_CORE_ONLY)
    return()
endif ()
add_subdirectory(${NXKIT_ROOT}/UIKit UIKit)
include_directories(${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib)
add_executable(UIKitNavigationTests ${NXKIT_ROOT}/UIKit/tests/UINavigationTests.cpp ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(UIKitNavigationTests PRIVATE Xitrix::UIKit)
add_test(NAME UIKitNavigationTests COMMAND UIKitNavigationTests)
add_executable(InteractiveNavigationTests ${NXKIT_ROOT}/UIKit/tests/InteractiveNavigationTests.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(InteractiveNavigationTests PRIVATE Xitrix::UIKit NXKit)
target_compile_definitions(InteractiveNavigationTests PRIVATE NXKIT_TEST_NX_NAVIGATION=1)
add_test(NAME InteractiveNavigationTests COMMAND InteractiveNavigationTests)

add_executable(UIKitTabTests ${NXKIT_ROOT}/UIKit/tests/UITabBarControllerTests.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(UIKitTabTests PRIVATE Xitrix::UIKit)
add_test(NAME UIKitTabTests COMMAND UIKitTabTests)
add_executable(LayerTests ${NXKIT_ROOT}/UIKit/tests/GlassAppearanceTests.cpp)
target_link_libraries(LayerTests PRIVATE XITRIXUIKit)
add_executable(CornerTests ${NXKIT_ROOT}/UIKit/tests/CornerRenderingTests.cpp)
target_link_libraries(CornerTests PRIVATE XITRIXUIKit)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME NativeGlassProfileData COMMAND ${Python3_EXECUTABLE}
    ${NXKIT_ROOT}/Demo/GlassComparison/NativeProfiles/generate.py --check)
add_executable(NativeGlassProfileTests ${NXKIT_ROOT}/UIKit/tests/NativeGlassProfileTests.cpp)
target_link_libraries(NativeGlassProfileTests PRIVATE XITRIXUIKit)
add_test(NAME NativeGlassProfileTests COMMAND NativeGlassProfileTests)
add_executable(NativeGlassAccentTests ${NXKIT_ROOT}/UIKit/tests/NativeGlassAccentTests.cpp)
target_link_libraries(NativeGlassAccentTests PRIVATE XITRIXUIKit)
add_test(NAME NativeGlassAccentTests COMMAND NativeGlassAccentTests)
add_test(NAME NativeGlassAccentData COMMAND ${Python3_EXECUTABLE}
    ${NXKIT_ROOT}/Demo/GlassComparison/NativePassProbe/generate-accent.py --check)
add_executable(NativeGlassBlurTests ${NXKIT_ROOT}/UIKit/tests/NativeGlassBlurTests.cpp)
target_link_libraries(NativeGlassBlurTests PRIVATE Xitrix::UIKit)
add_test(NAME NativeGlassBlurTests COMMAND NativeGlassBlurTests)
add_executable(NativeGlassLightingTests ${NXKIT_ROOT}/UIKit/tests/NativeGlassLightingTests.cpp)
target_link_libraries(NativeGlassLightingTests PRIVATE XITRIXUIKit)
add_test(NAME NativeGlassLightingTests COMMAND NativeGlassLightingTests)
add_executable(MaterialTests ${NXKIT_ROOT}/UIKit/tests/GlassMaterialTests.cpp)
target_link_libraries(MaterialTests PRIVATE XITRIXUIKit)

add_executable(ContainerTests ${NXKIT_ROOT}/UIKit/tests/GlassContainerTests.cpp)
target_sources(ContainerTests PRIVATE ${NXKIT_ROOT}/UIKit/tests/FilterRegionProbe.cpp)
set_source_files_properties(${NXKIT_ROOT}/UIKit/tests/FilterRegionProbe.cpp PROPERTIES COMPILE_OPTIONS -fno-rtti)
target_link_libraries(ContainerTests PRIVATE XITRIXUIKit)

add_executable(ShadowTests ${NXKIT_ROOT}/UIKit/tests/GlassShadowTests.cpp)
target_link_libraries(ShadowTests PRIVATE XITRIXUIKit)
add_executable(RimTests ${NXKIT_ROOT}/UIKit/tests/GlassRimTests.cpp)
target_link_libraries(RimTests PRIVATE XITRIXUIKit)

foreach(test LayerTests CornerTests MaterialTests ContainerTests ShadowTests RimTests)
    add_test(NAME ${test} COMMAND ${test})
endforeach()

add_executable(ShadowRenderer ../ShadowRenderer.cpp)
target_link_libraries(ShadowRenderer PRIVATE XITRIXUIKit)

add_executable(GalleryRenderer ../GalleryRenderer.cpp
    ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassDemoScene.cpp)
target_link_libraries(GalleryRenderer PRIVATE XITRIXUIKit)

add_executable(GalleryTests ../GallerySceneTests.cpp
    ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassDemoScene.cpp)
target_link_libraries(GalleryTests PRIVATE XITRIXUIKit)
add_test(NAME GalleryTests COMMAND GalleryTests)

add_executable(InteractionTests ${NXKIT_ROOT}/UIKit/tests/GlassInteractionTests.cpp)
target_link_libraries(InteractionTests PRIVATE XITRIXUIKit)
target_compile_definitions(InteractionTests PRIVATE NXKIT_GLASS_INTERACTION_REFERENCE="${NXKIT_ROOT}/docs/liquid-glass-interaction/uikit-reference.txt")
add_test(NAME InteractionTests COMMAND InteractionTests)

add_executable(TraitCollectionTests ${NXKIT_ROOT}/UIKit/tests/UITraitCollectionTests.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(TraitCollectionTests PRIVATE XITRIXUIKit)
add_test(NAME TraitCollectionTests COMMAND TraitCollectionTests)

add_executable(ApplicationScaleTests ${NXKIT_ROOT}/UIKitCore/tests/UIApplicationScaleTests.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
# Compile the real iOS startup path with a test context factory. UIKitCore's
# ordinary raster factory remains available to the other test targets.
target_compile_definitions(ApplicationScaleTests PRIVATE
    PLATFORM_IOS MakeSkiaCtx=MakeStartupScaleTestContext)
target_link_libraries(ApplicationScaleTests PRIVATE XITRIXUIKit)
add_test(NAME ApplicationScaleTests COMMAND ApplicationScaleTests)
set_tests_properties(ApplicationScaleTests PROPERTIES ENVIRONMENT "NXKIT_CONTINUOUS_RENDERING=0;NXKIT_UNCAPPED=0")
add_test(NAME ContinuousApplicationFrameTests COMMAND ApplicationScaleTests)
set_tests_properties(ContinuousApplicationFrameTests PROPERTIES ENVIRONMENT "NXKIT_CONTINUOUS_RENDERING=1;NXKIT_UNCAPPED=0")
add_test(NAME UncappedApplicationFrameTests COMMAND ApplicationScaleTests)
set_tests_properties(UncappedApplicationFrameTests PROPERTIES ENVIRONMENT "NXKIT_CONTINUOUS_RENDERING=0;NXKIT_UNCAPPED=1")
add_executable(LayoutPassTests ${NXKIT_ROOT}/UIKitCore/tests/LayoutPassTests.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(LayoutPassTests PRIVATE XITRIXUIKit)
add_test(NAME LayoutPassTests COMMAND LayoutPassTests)

option(NXKIT_RASTER_CONTROLLER_TESTS "Build adaptive tab navigation tests without a GPU window" OFF)
if (NXKIT_RASTER_CONTROLLER_TESTS)
    # Only the self-contained adaptive harness initializes an application/font
    # context here. Keep the general NXKit test targets under their own option.
    function(add_adaptive_tab_tests)
        set(NXKIT_BUILD_TESTS OFF)
        add_subdirectory(${NXKIT_ROOT}/NXKit NXKit)
        add_executable(ModuleCompositionTests
            ${NXKIT_ROOT}/UIKit/tests/ModuleCompositionTests.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(ModuleCompositionTests PRIVATE Xitrix::UIKit NXKit)
        add_test(NAME ModuleCompositionTests COMMAND ModuleCompositionTests)
        add_executable(AdaptiveTabTests
            ${NXKIT_ROOT}/NXKit/tests/NXAdaptiveTabBarControllerTests.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(AdaptiveTabTests PRIVATE NXKit)
        add_test(NAME AdaptiveTabTests COMMAND AdaptiveTabTests)
        add_executable(NavigationTests ${NXKIT_ROOT}/NXKit/tests/NXNavigationControllerTests.cpp)
        target_link_libraries(NavigationTests PRIVATE NXKit)
        add_test(NAME NavigationTests COMMAND NavigationTests)
        add_executable(TabControllerTests ${NXKIT_ROOT}/NXKit/tests/NXTabBarControllerTests.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(TabControllerTests PRIVATE NXKit)
        add_test(NAME TabControllerTests COMMAND TabControllerTests)
        add_executable(GlassScreenTests
            ${NXKIT_ROOT}/Demo/tests/GlassScreenTests.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenView.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenViewController.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/SystemControlsDemoViewController.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(GlassScreenTests PRIVATE NXKit Xitrix::UIKit)
        add_test(NAME GlassScreenTests COMMAND GlassScreenTests)
        add_executable(GlassPerformance ../GlassPerformance.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenView.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassDemoScene.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensDemoStage.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(GlassPerformance PRIVATE NXKit Xitrix::UIKit)
    endfunction()
    add_adaptive_tab_tests()
endif ()

add_executable(LensTabTests ${NXKIT_ROOT}/Demo/tests/LensTabBarTests.cpp
    ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp)
target_link_libraries(LensTabTests PRIVATE XITRIXUIKit)
add_test(NAME LensTabTests COMMAND LensTabTests)

add_executable(LensTests ${NXKIT_ROOT}/UIKit/tests/LensViewTests.cpp)
target_link_libraries(LensTests PRIVATE XITRIXUIKit)
add_test(NAME LensTests COMMAND LensTests)

add_executable(LensRefractionTests ${NXKIT_ROOT}/UIKit/tests/LensRefractionTests.cpp)
target_link_libraries(LensRefractionTests PRIVATE XITRIXUIKit)
add_test(NAME LensRefractionTests COMMAND LensRefractionTests)

add_executable(LensRefractionRenderer ../LensRefractionRenderer.cpp)
target_link_libraries(LensRefractionRenderer PRIVATE XITRIXUIKit)

add_executable(LensMotionTests ${NXKIT_ROOT}/UIKit/tests/LensMotionTests.cpp)
target_link_libraries(LensMotionTests PRIVATE XITRIXUIKit)
target_compile_definitions(LensMotionTests PRIVATE NXKIT_LENS_MOTION_REFERENCE="${NXKIT_ROOT}/docs/liquid-glass-lens/interaction/native-reference.csv")
add_test(NAME LensMotionTests COMMAND LensMotionTests)

add_executable(LayerSamplingTests ${NXKIT_ROOT}/UIKitCore/tests/LayerSamplingTests.cpp)
target_link_libraries(LayerSamplingTests PRIVATE XITRIXUIKit)
add_test(NAME LayerSamplingTests COMMAND LayerSamplingTests)

if (APPLE)
    add_executable(TextRendererTests ${NXKIT_ROOT}/UIKitCore/tests/TextRenderingTests.cpp
        ${NXKIT_ROOT}/UIKitCore/tests/RobotoTestFontManager.cpp)
    set_source_files_properties(${NXKIT_ROOT}/UIKitCore/tests/RobotoTestFontManager.cpp PROPERTIES COMPILE_OPTIONS -fno-rtti)
    target_link_libraries(TextRendererTests PRIVATE XITRIXUIKit)
    target_compile_definitions(TextRendererTests PRIVATE NXKIT_TEXT_TEST_FONT="${NXKIT_ROOT}/Submodules/skia/resources/fonts/Roboto-Regular.ttf")
    add_test(NAME TextRendererTests COMMAND TextRendererTests)
    add_executable(TabPressRenderer ../TabPressRenderer.cpp
        ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
    target_link_libraries(TabPressRenderer PRIVATE XITRIXUIKit)
    add_executable(TabTransferRenderer ../TabTransferRenderer.cpp
        ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
    target_link_libraries(TabTransferRenderer PRIVATE XITRIXUIKit)
endif ()

add_executable(GlassColorRenderer ../GlassColorRenderer.cpp)
target_link_libraries(GlassColorRenderer PRIVATE XITRIXUIKit)

add_executable(ScrollReplay ../../ScrollComparison/Replay.cpp)
target_link_libraries(ScrollReplay PRIVATE XITRIXUIKit)

add_executable(ScrollPhysicsTests ${NXKIT_ROOT}/UIKitCore/tests/ScrollPhysicsTests.cpp)
add_executable(ContentScrollViewTests ${NXKIT_ROOT}/UIKitCore/tests/ContentScrollViewTests.cpp)
target_include_directories(ContentScrollViewTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib)
target_link_libraries(ContentScrollViewTests PRIVATE UIKitCore)
add_test(NAME ContentScrollViewTests COMMAND ContentScrollViewTests)
add_executable(LayerContentMaskTests ${NXKIT_ROOT}/UIKitCore/tests/LayerContentMaskTests.cpp)
target_include_directories(LayerContentMaskTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib)
target_link_libraries(LayerContentMaskTests PRIVATE UIKitCore)
add_test(NAME LayerContentMaskTests COMMAND LayerContentMaskTests)
add_executable(PortableScrollEdgeReference ${NXKIT_ROOT}/Demo/ScrollEdgeComparison/PortableReference.cpp)
target_include_directories(PortableScrollEdgeReference PRIVATE ${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib)
target_link_libraries(PortableScrollEdgeReference PRIVATE Xitrix::UIKit)
add_executable(ScrollEdgeEffectsTests ${NXKIT_ROOT}/UIKit/tests/ScrollEdgeEffectsTests.cpp)
target_include_directories(ScrollEdgeEffectsTests PRIVATE ${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib)
target_link_libraries(ScrollEdgeEffectsTests PRIVATE Xitrix::UIKit)
add_test(NAME ScrollEdgeEffectsTests COMMAND ScrollEdgeEffectsTests)
target_include_directories(ScrollPhysicsTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib)
target_link_libraries(ScrollPhysicsTests PRIVATE XITRIXUIKit)
add_test(NAME ScrollPhysicsTests COMMAND ScrollPhysicsTests)
add_executable(GestureTests GestureMain.cpp)
if(APPLE)
    add_executable(MetalGlassStressPerformance ../MetalGlassStressPerformance.mm
        ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassStressView.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/platforms/SkiaCtx.cpp)
    get_target_property(stress_abi_header UIKitCore NXKIT_METAL_SHADER_ABI_HEADER)
    get_filename_component(stress_abi_directory "${stress_abi_header}" DIRECTORY)
    target_include_directories(MetalGlassStressPerformance PRIVATE
        ${NXKIT_ROOT}/UIKitCore/lib ${stress_abi_directory})
    target_link_libraries(MetalGlassStressPerformance PRIVATE Xitrix::UIKit)
    add_executable(MetalScrollEdgeBenchmark ../../ScrollEdgeComparison/MetalScrollEdgeBenchmark.mm
        ${NXKIT_ROOT}/UIKitCore/lib/platforms/SkiaCtx.cpp)
    get_target_property(scroll_edge_abi_header UIKitCore NXKIT_METAL_SHADER_ABI_HEADER)
    get_filename_component(scroll_edge_abi_directory "${scroll_edge_abi_header}" DIRECTORY)
    target_include_directories(MetalScrollEdgeBenchmark PRIVATE
        ${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib ${scroll_edge_abi_directory})
    target_link_libraries(MetalScrollEdgeBenchmark PRIVATE Xitrix::UIKit)
    add_executable(NavigationMotionRenderer ../../NavigationComparison/MotionRenderer.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
    target_include_directories(NavigationMotionRenderer PRIVATE ${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib)
    target_link_libraries(NavigationMotionRenderer PRIVATE Xitrix::UIKit)
    if(TARGET NXKit)
        add_executable(NavigationCycleRenderer ../../NavigationComparison/CycleRenderer.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenView.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenViewController.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/SystemControlsDemoViewController.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_include_directories(NavigationCycleRenderer PRIVATE ${NXKIT_ROOT}/UIKit/lib ${NXKIT_ROOT}/UIKitCore/lib)
        target_link_libraries(NavigationCycleRenderer PRIVATE Xitrix::UIKit NXKit)
        add_executable(InteractiveReplay ../../NavigationComparison/InteractiveReplay.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenView.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassScreenViewController.cpp
            ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/SystemControlsDemoViewController.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(InteractiveReplay PRIVATE Xitrix::UIKit NXKit)
        add_executable(MetalNavigationBenchmark ../../NavigationComparison/MetalNavigationBenchmark.mm
            ${NXKIT_ROOT}/UIKitCore/lib/platforms/SkiaCtx.cpp
            ../../app/Screens/GlassDemoViewController/GlassScreenView.cpp
            ../../app/Screens/GlassDemoViewController/GlassScreenViewController.cpp
            ../../app/Screens/GlassDemoViewController/SystemControlsDemoViewController.cpp
            ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
        target_link_libraries(MetalNavigationBenchmark PRIVATE Xitrix::UIKit NXKit)
    endif()
endif()
target_include_directories(GestureTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib ${NXKIT_ROOT}/UIKit/lib)
target_link_libraries(GestureTests PRIVATE XITRIXUIKit)
add_test(NAME GestureTests COMMAND GestureTests)
add_executable(ScrollIndicatorTests ${NXKIT_ROOT}/UIKitCore/tests/UIScrollViewIndicatorTests.cpp)
target_link_libraries(ScrollIndicatorTests PRIVATE XITRIXUIKit)
add_test(NAME ScrollIndicatorTests COMMAND ScrollIndicatorTests)

add_executable(GlassStressPerformance ../GlassStressPerformance.cpp
    ${NXKIT_ROOT}/Demo/app/Screens/GlassDemoViewController/GlassStressView.cpp
    ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
target_link_libraries(GlassStressPerformance PRIVATE XITRIXUIKit)
add_test(NAME GlassStressReplay COMMAND GlassStressPerformance
    ${CMAKE_CURRENT_BINARY_DIR}/glass-stress-replay --record-only)
set_tests_properties(GlassStressReplay PROPERTIES ENVIRONMENT "NXKIT_CONTINUOUS_RENDERING=1")
add_executable(RenderTraversalTests ${NXKIT_ROOT}/UIKitCore/tests/RenderTraversalTests.cpp)
target_link_libraries(RenderTraversalTests PRIVATE XITRIXUIKit)
add_test(NAME RenderTraversalTests COMMAND RenderTraversalTests)

if (APPLE)
    add_executable(DesktopResizeTests ${NXKIT_ROOT}/UIKitCore/tests/DesktopResizeTests.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/platforms/SkiaCtx_sdlBase.cpp
        ${NXKIT_ROOT}/UIKitCore/lib/UIApplicationMain.cpp)
    target_compile_definitions(DesktopResizeTests PRIVATE PLATFORM_MAC)
    target_link_libraries(DesktopResizeTests PRIVATE XITRIXUIKit)
    # Native Cocoa window test: run explicitly where window-server access exists.
endif ()

# Independent iOS 27 preference/style/color reference scenes.
add_executable(GlassTintMatrix ../TintPreference/PortableRenderer.cpp)
target_link_libraries(GlassTintMatrix PRIVATE Xitrix::UIKit)

if(APPLE)
    add_executable(MetalPipelineDescriptionTests ${NXKIT_ROOT}/UIKitCore/tests/MetalPipelineDescriptionTests.mm)
    target_include_directories(MetalPipelineDescriptionTests PRIVATE ${NXKIT_ROOT}/UIKitCore/lib)
    target_compile_options(MetalPipelineDescriptionTests PRIVATE -fobjc-arc)
    target_link_libraries(MetalPipelineDescriptionTests PRIVATE "-framework Foundation" "-framework Metal")
    add_test(NAME MetalPipelineDescriptionTests COMMAND MetalPipelineDescriptionTests)
endif()

add_executable(NativeGlassRefractionRenderer ../NativePassProbe/RefractionRenderer.cpp)
target_link_libraries(NativeGlassRefractionRenderer PRIVATE Xitrix::UIKit)
add_test(NAME NativeGlassRefractionCoordinates COMMAND /usr/bin/python3
    ${NXKIT_ROOT}/Demo/GlassComparison/NativePassProbe/compare-refraction.py
    --renderer $<TARGET_FILE:NativeGlassRefractionRenderer>
    --reference ${NXKIT_ROOT}/docs/liquid-glass-native-settings/pass-semantics/refraction)
