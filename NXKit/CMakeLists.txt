project(NXKit)

add_library(NXKit
        lib/NXControl.cpp
        lib/NXControllerIconResolver.cpp
        lib/NXNavigationItem.cpp
        lib/NXNavigationController.cpp
        lib/NXResponderAction.cpp
        lib/NXSeparatorView.cpp
        lib/NXTabBarController.cpp
)

if (APPLE)
    target_sources(NXKit PRIVATE
            lib/platforms/apple/NXControllerIconProvider_apple.mm
    )
    target_link_libraries(NXKit PRIVATE "-framework GameController")
    if (PLATFORM_IOS)
        target_link_libraries(NXKit PRIVATE "-framework UIKit")
    else ()
        target_link_libraries(NXKit PRIVATE "-framework AppKit")
    endif ()
elseif (PLATFORM_SWITCH)
    target_sources(NXKit PRIVATE
            lib/platforms/switch/NXControllerIconProvider_switch.cpp
    )
else ()
    target_sources(NXKit PRIVATE
            lib/platforms/NXControllerIconProvider_default.cpp
    )
endif ()

target_include_directories(NXKit PUBLIC
        include
        lib
)

target_link_libraries(NXKit PUBLIC XITRIXUIKit)

option(NXKIT_BUILD_TESTS "Build NXKit regression tests" OFF)
if (NXKIT_BUILD_TESTS)
    enable_testing()
    add_executable(NXKitControllerIconResolverTests
            tests/NXControllerIconResolverTests.cpp
    )
    target_link_libraries(NXKitControllerIconResolverTests PRIVATE NXKit)
    if (PLATFORM_IOS)
        set_target_properties(NXKitControllerIconResolverTests PROPERTIES
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                    "ru.xitrix.nxkit.tests.controller-icons"
                XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO
                XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
        )
    endif ()
    add_test(
            NAME NXKitControllerIconResolverTests
            COMMAND NXKitControllerIconResolverTests
    )

    add_executable(NXKitTabBarControllerTests
            tests/NXTabBarControllerTests.cpp
    )
    target_link_libraries(NXKitTabBarControllerTests PRIVATE NXKit)
    add_test(NAME NXKitTabBarControllerTests COMMAND NXKitTabBarControllerTests)

    add_executable(NXKitNavigationControllerTests
            tests/NXNavigationControllerTests.cpp
    )
    target_link_libraries(NXKitNavigationControllerTests PRIVATE NXKit)
    add_test(NAME NXKitNavigationControllerTests COMMAND NXKitNavigationControllerTests)

    add_executable(NXKitGestureRecognizerTests
            tests/UIGestureRecognizerTests.cpp
    )
    target_link_libraries(NXKitGestureRecognizerTests PRIVATE NXKit)
    add_test(NAME NXKitGestureRecognizerTests COMMAND NXKitGestureRecognizerTests)

    add_executable(NXKitScrollViewIndicatorTests
            tests/UIScrollViewIndicatorTests.cpp
    )
    target_link_libraries(NXKitScrollViewIndicatorTests PRIVATE NXKit)
    add_test(
            NAME NXKitScrollViewIndicatorTests
            COMMAND NXKitScrollViewIndicatorTests
    )

    add_executable(NXKitPressRepeatTests
            tests/UIPressRepeatTests.cpp
    )
    target_link_libraries(NXKitPressRepeatTests PRIVATE NXKit)
    if (PLATFORM_IOS)
        set_target_properties(NXKitPressRepeatTests PROPERTIES
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                    "ru.xitrix.nxkit.tests.press-repeat"
                XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO
                XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
        )
    endif ()
    add_test(NAME NXKitPressRepeatTests COMMAND NXKitPressRepeatTests)

    add_executable(NXKitLayerRenderingTests
            tests/CALayerRenderingTests.cpp
    )
    # The checked-in Skia archives are built without RTTI. Match that mode for
    # the test canvas subclass so its vtable does not require Skia typeinfo.
    target_compile_options(NXKitLayerRenderingTests PRIVATE -fno-rtti)
    target_link_libraries(NXKitLayerRenderingTests PRIVATE NXKit)
    add_test(NAME NXKitLayerRenderingTests COMMAND NXKitLayerRenderingTests)
endif ()
