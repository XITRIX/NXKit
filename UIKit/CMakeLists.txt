add_library(XITRIXUIKit
        include/UIKit.h
        include/UIKitRegistration.h
        include/UIButton.h
        include/UIGlassEffect.h
        include/UIGlassContainerEffect.h
        include/BackdropEffectView.h
        include/GlassContainerView.h
        include/LensView.h
        lib/BackdropEffectView.cpp
        lib/CABackdropEffectLayer.cpp
        lib/CAGlassContainerLayer.cpp
        lib/GlassContainerView.cpp
        lib/GlassShader.cpp
        lib/LensMaterial.cpp
        lib/LensView.cpp
        include/UITabBarController.h
        lib/UITabBarController.cpp
        include/UITabBarView.h
        include/UITabBar.h
        lib/UITabBarView.cpp
        lib/UITabBarViewInternal.hpp
        lib/TabSelectionMaterial.hpp
        lib/UIButton.cpp
        lib/UIGlassEffect.cpp
        lib/UIKitRegistration.cpp
)
add_library(Xitrix::UIKit ALIAS XITRIXUIKit)
target_include_directories(XITRIXUIKit PUBLIC include PRIVATE
    lib ${CMAKE_CURRENT_SOURCE_DIR}/../UIKitCore/lib)
target_link_libraries(XITRIXUIKit PUBLIC UIKitCore)

option(UIKIT_BUILD_TESTS "Build UIKit regression tests" OFF)
if (UIKIT_BUILD_TESTS)
    enable_testing()
    add_executable(UIKitTabBarControllerTests tests/UITabBarControllerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../UIKitCore/lib/UIApplicationMain.cpp)
    target_include_directories(UIKitTabBarControllerTests PRIVATE lib)
    target_link_libraries(UIKitTabBarControllerTests PRIVATE Xitrix::UIKit)
    add_test(NAME UIKitTabBarControllerTests COMMAND UIKitTabBarControllerTests)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/../UIKitCore/lib)
    add_executable(NXKitGestureRecognizerTests
            tests/UIGestureRecognizerTests.cpp
    )
    target_link_libraries(NXKitGestureRecognizerTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGestureRecognizerTests COMMAND NXKitGestureRecognizerTests)

    add_executable(NXKitLayerRenderingTests
            tests/CALayerRenderingTests.cpp
    )
    # The checked-in Skia archives are built without RTTI. Match that mode for
    # the test canvas subclass so its vtable does not require Skia typeinfo.
    target_compile_options(NXKitLayerRenderingTests PRIVATE -fno-rtti)
    target_link_libraries(NXKitLayerRenderingTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitLayerRenderingTests COMMAND NXKitLayerRenderingTests)

    add_executable(NXKitGlassMaterialTests tests/GlassMaterialTests.cpp)
    target_link_libraries(NXKitGlassMaterialTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGlassMaterialTests COMMAND NXKitGlassMaterialTests)

    add_executable(NXKitGlassInteractionTests tests/GlassInteractionTests.cpp)
    set(glass_interaction_reference "${CMAKE_CURRENT_SOURCE_DIR}/../docs/liquid-glass-interaction/uikit-reference.txt")
    target_sources(NXKitGlassInteractionTests PRIVATE "${glass_interaction_reference}")
    set_source_files_properties("${glass_interaction_reference}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_link_libraries(NXKitGlassInteractionTests PRIVATE Xitrix::UIKit)
    target_compile_definitions(NXKitGlassInteractionTests PRIVATE NXKIT_GLASS_INTERACTION_REFERENCE="${CMAKE_CURRENT_SOURCE_DIR}/../docs/liquid-glass-interaction/uikit-reference.txt")
    add_test(NAME NXKitGlassInteractionTests COMMAND NXKitGlassInteractionTests)

    add_executable(NXKitGlassAppearanceTests tests/GlassAppearanceTests.cpp)
    target_link_libraries(NXKitGlassAppearanceTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGlassAppearanceTests COMMAND NXKitGlassAppearanceTests)

    add_executable(NXKitLensViewTests tests/LensViewTests.cpp)
    target_link_libraries(NXKitLensViewTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitLensViewTests COMMAND NXKitLensViewTests)

    add_executable(NXKitLensMotionTests tests/LensMotionTests.cpp)
    set(lens_motion_reference "${CMAKE_CURRENT_SOURCE_DIR}/../docs/liquid-glass-lens/interaction/native-reference.csv")
    target_sources(NXKitLensMotionTests PRIVATE "${lens_motion_reference}")
    set_source_files_properties("${lens_motion_reference}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_link_libraries(NXKitLensMotionTests PRIVATE Xitrix::UIKit)
    target_compile_definitions(NXKitLensMotionTests PRIVATE NXKIT_LENS_MOTION_REFERENCE="${lens_motion_reference}")
    add_test(NAME NXKitLensMotionTests COMMAND NXKitLensMotionTests)

    add_executable(NXKitLensRefractionTests tests/LensRefractionTests.cpp)
    target_link_libraries(NXKitLensRefractionTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitLensRefractionTests COMMAND NXKitLensRefractionTests)

    add_executable(NXKitGlassContainerTests tests/GlassContainerTests.cpp tests/FilterRegionProbe.cpp)
    set_source_files_properties(tests/FilterRegionProbe.cpp PROPERTIES COMPILE_OPTIONS -fno-rtti)
    target_link_libraries(NXKitGlassContainerTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGlassContainerTests COMMAND NXKitGlassContainerTests)

    add_executable(NXKitGlassShadowTests tests/GlassShadowTests.cpp)
    target_link_libraries(NXKitGlassShadowTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGlassShadowTests COMMAND NXKitGlassShadowTests)
    add_executable(NXKitGlassRimTests tests/GlassRimTests.cpp)
    target_link_libraries(NXKitGlassRimTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitGlassRimTests COMMAND NXKitGlassRimTests)

    add_executable(NXKitCornerRenderingTests tests/CornerRenderingTests.cpp)
    target_link_libraries(NXKitCornerRenderingTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitCornerRenderingTests COMMAND NXKitCornerRenderingTests)

    add_executable(NXKitTraitCollectionTests tests/UITraitCollectionTests.cpp)
    target_link_libraries(NXKitTraitCollectionTests PRIVATE Xitrix::UIKit)
    add_test(NAME NXKitTraitCollectionTests COMMAND NXKitTraitCollectionTests)

endif ()
