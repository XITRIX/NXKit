project(NXKit)

add_library(NXKit
        lib/NXControl.cpp
        lib/NXNavigationItem.cpp
        lib/NXNavigationController.cpp
        lib/NXResponderAction.cpp
        lib/NXSeparatorView.cpp
        lib/NXTabBarController.cpp
)

target_include_directories(NXKit PUBLIC
        include
        lib
)

target_link_libraries(NXKit PUBLIC XITRIXUIKit)

option(NXKIT_BUILD_TESTS "Build NXKit regression tests" OFF)
if (NXKIT_BUILD_TESTS)
    enable_testing()
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

    add_executable(NXKitPressRepeatTests
            tests/UIPressRepeatTests.cpp
    )
    target_link_libraries(NXKitPressRepeatTests PRIVATE NXKit)
    add_test(NAME NXKitPressRepeatTests COMMAND NXKitPressRepeatTests)
endif ()
