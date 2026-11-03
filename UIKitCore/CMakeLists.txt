set_source_files_properties(lib/platforms/apple/ios/IOSWindowGeometry.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")

add_library(UIKitCore
        lib/tools/Logger.cpp
        lib/tools/IBTools.cpp
        lib/tools/Fatal.cpp
        lib/tools/Tools.cpp
        lib/CABasicAnimation.cpp
        lib/CABasicAnimationPrototype.cpp
        lib/CABlurLayer.cpp
        lib/CASpringAnimation.cpp
        lib/CASpringAnimationPrototype.cpp
        lib/CAMediaTimingFunction.cpp
        lib/FontResolution.cpp
        lib/FontTracking.hpp
        lib/TextRendering.hpp
        lib/CATextLayer.cpp
        lib/CATransaction.cpp
        lib/ContentsGravityTransformation.cpp
        lib/platforms/FramePacing.cpp
        lib/platforms/SkiaCtx.cpp
        lib/Application.cpp
        lib/BackdropEffect.cpp
        lib/BackdropShader.cpp
        lib/BackdropShader.hpp
        lib/BackdropSampling.hpp
        lib/CABackdropLayer.cpp
        include/CABackdropLayer.h
        include/UIKitCore.h
        include/tools/DragSmoother.hpp
        include/UINavigationControllerInterface.h
        include/UITabBarControllerInterface.h
        lib/DispatchQueue.cpp
        lib/Timer.cpp
        lib/Geometry.cpp
        lib/IndexPath.cpp
        lib/CGImage.cpp
        lib/CADisplayLink.cpp
        lib/RenderPerformance.hpp
        lib/CALayer.cpp
        lib/LayerCornerGeometry.cpp
        lib/LayerCornerGeometry.hpp
        include/UICornerRadius.h
        include/UICornerConfiguration.h
        include/UIRectCorner.h
        include/CALayerCornerCurve.h
        lib/LayerAnimationTimeline.hpp
        lib/YGLayout.cpp
        include/UIBarButtonItem.h
        lib/UIBarButtonItem.cpp
        include/UINavigationItem.h
        lib/UINavigationItem.cpp
        lib/NavigationItemAccess.hpp
        lib/ViewAppearanceAccess.hpp
        lib/ViewCornerAccess.hpp
        lib/UIBarItem.cpp
        lib/UITabBarItem.cpp
        include/UIBarItem.h
        include/UITabBarItem.h
        include/UITabBarControllerMode.h
        lib/BarItemAccess.hpp
        lib/ViewControllerContainerAccess.hpp
        lib/UIAction.cpp
        lib/UIApplication.cpp
        lib/UIApplicationDelegateImpl.cpp
        lib/UIApplicationMain.cpp
        lib/UIBlurView.cpp
        lib/UIColor.cpp
        lib/UIControl.cpp
        lib/UIControlGestureRecognizer.cpp
        lib/UIEdgeInsets.cpp
        lib/UIEvent.cpp
        lib/UIFocus.cpp
        lib/UIFocusAnimationCoordinator.cpp
        lib/UIFocusSystem.cpp
        lib/UIGestureRecognizer.cpp
        lib/UIImage.cpp
        lib/UIImageView.cpp
        lib/UIKey.cpp
        lib/UILabel.cpp
        lib/UINib.cpp
        lib/UIPanGestureRecognizer.cpp
        lib/UIPress.cpp
        lib/UIPressesEvent.cpp
        lib/UIResponder.cpp
        lib/UIScrollView.cpp
        lib/UITapGestureRecognizer.cpp
        lib/UITextView.cpp
        lib/UITouch.cpp
        include/UIUserInterfaceSizeClass.h
        lib/UITraitCollection.cpp
        lib/UITraitEnvironment.cpp
        lib/UIView.cpp
        lib/UIViewCorners.cpp
        lib/UIViewAnimationGroup.cpp
        lib/UIViewAnimationOptions.cpp
        lib/UIViewController.cpp
        lib/UIPresentationController.cpp
        lib/UIWindow.cpp
        lib/NXAffineTransform.cpp
        lib/NXTransform3D.cpp
        lib/NXData.cpp
        lib/UIScrollViewExtensions/DecelerationTimingParameters.cpp
        lib/UIScrollViewExtensions/RubberBand.cpp
        lib/UIScrollViewExtensions/SpringTimingParameters.cpp
        lib/UIScrollViewExtensions/TimerAnimation.cpp
        lib/SkTools/EventTracingPriv.cpp
        lib/SkTools/SkDebugfTracer.cpp
)

add_library(Xitrix::UIKitCore ALIAS UIKitCore)

target_include_directories(UIKitCore PUBLIC include PRIVATE lib)


if (APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/NXKitMetalCache.cmake)
    nxkit_configure_metal_cache(UIKitCore "${CMAKE_CURRENT_LIST_DIR}/..")
    target_sources(UIKitCore PRIVATE
            lib/platforms/SkiaCtx_sdlBase.cpp
            lib/platforms/apple/AppleSystemFont.mm
            lib/platforms/apple/AppleSystemImage.mm
    )
    if (PLATFORM_DESKTOP)
        target_sources(UIKitCore PRIVATE
                lib/platforms/apple/macos/SkiaCtx_macos.mm
        )
    elseif (PLATFORM_IOS)
        target_sources(UIKitCore PRIVATE
                lib/platforms/apple/ios/SkiaCtx_ios.mm
                lib/platforms/apple/ios/IOSWindowGeometry.mm
                lib/platforms/apple/ios/IOSWindowGeometry.hpp
        )
    endif ()
elseif (PLATFORM_SWITCH)
    target_sources(UIKitCore PRIVATE
            lib/platforms/switch/SkiaCtx_switch.cpp
            lib/platforms/switch/SwitchPipelineCache.cpp
            lib/platforms/switch/SwitchPipelineCache.hpp
            lib/platforms/switch/SwitchCacheFormat.hpp
            lib/platforms/switch/SwitchCacheFile.hpp
            lib/platforms/switch/switch_wrapper.c
    )
endif ()

option(UIKITCORE_BUILD_TESTS "Build UIKitCore regression tests" OFF)
if (UIKITCORE_BUILD_TESTS)
    enable_testing()
    add_executable(NXKitCornerConfigurationTests tests/CornerConfigurationTests.cpp)
    target_link_libraries(NXKitCornerConfigurationTests PRIVATE UIKitCore)
    add_test(NAME NXKitCornerConfigurationTests COMMAND NXKitCornerConfigurationTests)
    add_executable(NXKitPressLifecycleTests tests/UIApplicationPressLifecycleTests.cpp)
    target_link_libraries(NXKitPressLifecycleTests PRIVATE UIKitCore)
    add_test(NAME NXKitPressLifecycleTests COMMAND NXKitPressLifecycleTests)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
    add_executable(NXKitSwitchCacheFormatTests tests/SwitchCacheFormatTests.cpp)
    add_test(NAME NXKitSwitchCacheFormatTests COMMAND NXKitSwitchCacheFormatTests)
    add_executable(NXKitSwitchCacheFileTests tests/SwitchCacheFileTests.cpp)
    add_test(NAME NXKitSwitchCacheFileTests COMMAND NXKitSwitchCacheFileTests)
    add_executable(UIKitCoreModuleBoundaryTests tests/ModuleBoundaryTests.cpp)
    target_link_libraries(UIKitCoreModuleBoundaryTests PRIVATE UIKitCore)
    add_test(NAME UIKitCoreModuleBoundaryTests COMMAND UIKitCoreModuleBoundaryTests)
    add_executable(NXKitLayoutPassTests tests/LayoutPassTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../UIKitCore/lib/UIApplicationMain.cpp)
    target_link_libraries(NXKitLayoutPassTests PRIVATE UIKitCore)
    add_test(NAME NXKitLayoutPassTests COMMAND NXKitLayoutPassTests)
    add_executable(NXKitRenderTraversalTests tests/RenderTraversalTests.cpp)
    target_link_libraries(NXKitRenderTraversalTests PRIVATE UIKitCore)
    add_test(NAME NXKitRenderTraversalTests COMMAND NXKitRenderTraversalTests)
    add_executable(NXKitLayerSamplingTests tests/LayerSamplingTests.cpp)
    target_link_libraries(NXKitLayerSamplingTests PRIVATE UIKitCore)
    add_test(NAME NXKitLayerSamplingTests COMMAND NXKitLayerSamplingTests)
    add_executable(NXKitScrollPhysicsTests tests/ScrollPhysicsTests.cpp)
    target_include_directories(NXKitScrollPhysicsTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../UIKitCore/lib)
    target_link_libraries(NXKitScrollPhysicsTests PRIVATE UIKitCore)
    add_test(NAME NXKitScrollPhysicsTests COMMAND NXKitScrollPhysicsTests)

    add_executable(NXKitScrollViewIndicatorTests
            tests/UIScrollViewIndicatorTests.cpp
    )
    target_link_libraries(NXKitScrollViewIndicatorTests PRIVATE UIKitCore)
    add_test(
            NAME NXKitScrollViewIndicatorTests
            COMMAND NXKitScrollViewIndicatorTests
    )

    add_executable(NXKitDispatchQueueTests tests/DispatchQueueTests.cpp)
    target_link_libraries(NXKitDispatchQueueTests PRIVATE UIKitCore)
    add_test(NAME NXKitDispatchQueueTests COMMAND NXKitDispatchQueueTests)
endif ()
